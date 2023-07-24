#define SOGL_MAJOR_VERSION 4
#define SOGL_MINOR_VERSION 5
#define SOGL_ARB_texture_barrier

#ifdef __linux__ 
#define SOGL_IMPLEMENTATION_X11
#elif _WIN32
#define SOGL_IMPLEMENTATION_WIN32
#endif

#include "simple-opengl-loader.h"

#include "Graphics.h"
#include <exception>
#include <typeinfo>

Console* con = Console::get_instance();

// json load functions
void gr::json_to_coord(sf::Vector2d& pos, sf::Vector2d& size, json& _json) {
	try {
		pos = sf::Vector2d(_json[0].get<double>(), _json[1].get<double>());
		size = sf::Vector2d(_json[2].get<double>(), _json[3].get<double>());
	} catch (nlohmann::detail::type_error err) {
		throw err;
	}
}

std::pair<sf::Sprite, sf::Time> gr::json_to_frame(json& _json) {
	sf::Sprite spr;
	sf::IntRect rect;
	float time = 0.f;
	try {
		rect.left = _json[0].get<double>();
		rect.top = _json[1].get<double>();

		rect.width = _json[2].get<double>();
		rect.height = _json[3].get<double>();

		time = _json[4].get<double>();
	}
	catch (nlohmann::detail::type_error err) {
		throw err;
	}
	spr.setTextureRect(rect);
	return std::pair<sf::Sprite, sf::Time>(spr, sf::milliseconds(time));
}

std::pair<sf::Glsl::Vec3, sf::Time> gr::json_to_lightframe(json& _json) {
	sf::Glsl::Vec3 color;
	float time = 0.f;
	try {
		color.x = _json[0].get<double>();
		color.y = _json[1].get<double>();
		color.z = _json[2].get<double>();

		time = _json[3].get<double>();
	}
	catch (nlohmann::detail::type_error err) {
		throw err;
	}
	return std::pair<sf::Glsl::Vec3, sf::Time>(color, sf::milliseconds(time));
}


// Rect class

gr::Rect::Rect() {
	pos = sf::Vector2d(0.0, 0.0);
	size = sf::Vector2d(0.0, 0.0);
};

gr::Rect::Rect(double pos_x, double pos_y, double size_x, double size_y) {
	pos = sf::Vector2d(pos_x, pos_y);
	size = sf::Vector2d(size_x, size_y);
};

gr::Rect::Rect(sf::Vector2d _pos, sf::Vector2d _size) {
	pos = _pos;
	size = _size;
};

bool gr::Rect::intersects(const gr::Rect* rect) const {
	Rect temp(rect->pos, rect->size);
	temp.size += size;
	temp.pos.x -= size.x;
	temp.pos.y += size.y;

	return (pos.x < temp.size.x + temp.pos.x) && (pos.x > temp.pos.x) && (pos.y > temp.pos.y - temp.size.y) && (pos.y < temp.pos.y);
}

// Camera class

sf::Vector2f gr::Camera::world_to_pixel_pos(sf::Vector2d world_pos) const {
	return sf::Vector2f((world_pos.x - pos.x) * render_res.x / size.x, (pos.y - world_pos.y) * render_res.y / size.y);
}

sf::Vector2f gr::Camera::world_to_pixel_size(sf::Vector2d world_size) const {
	return sf::Vector2f(world_size.x * render_res.x / size.x, world_size.y * render_res.y / size.y);
}

sf::Vector2d gr::Camera::pixel_to_world_pos(sf::Vector2f camera_pos) const {
	return sf::Vector2d(pos.x + size.x * double(camera_pos.x / render_res.x), pos.y - size.y * double(camera_pos.y / render_res.y));
}

sf::Vector2f gr::Camera::world_to_glsl_pos(sf::Vector2d world_pos) const {
	return sf::Vector2f((world_pos.x - pos.x)/ size.x, (world_pos.y - pos.y + size.y) / size.y);
}

sf::Vector2f gr::Camera::world_to_glsl_size(sf::Vector2d world_size) const {
	return sf::Vector2f(world_size.x / size.x, world_size.y / size.y);
}

// Animation class

void gr::Animation::add_frame(sf::Sprite spr, sf::Time t) {
	full_anim_time += t;
	frames.push_back(std::pair<sf::Sprite, sf::Time>(spr, full_anim_time));
}

void gr::Animation::add_frame(std::pair<sf::Sprite, sf::Time> frame) {
	full_anim_time += frame.second;
	frame.second = full_anim_time;
	frames.push_back(frame);
}

sf::Sprite* gr::Animation::get_frame(sf::Time time) {
	if (frames.size() == 0)
		throw std::logic_error("Animation size was 0 frames");
	if (full_anim_time == sf::milliseconds(0))
		throw std::logic_error("Full animation time was 0");

	time %= full_anim_time;
	for (std::pair<sf::Sprite, sf::Time>& frame : frames)
		if (time < frame.second)
			return &frame.first;

	return &frames[0].first;
}

// Drawable class

double gr::Drawable::get_layer() {
	return layer;
}

// SpriteObject class

bool gr::SpriteObject::var_update(const gr::Camera& cam) {
	if (!cam.intersects(this))
		return false;

	animation_update();

	current_sprite->setPosition(cam.world_to_pixel_pos(pos));
	sf::Vector2f new_pixel_size = cam.world_to_pixel_size(size);
	current_sprite->setScale(sf::Vector2f(new_pixel_size.x / current_sprite->getLocalBounds().width, new_pixel_size.y / current_sprite->getLocalBounds().height));

	layer = pos.y - size.y + layer_shift;
	return true;
}

bool gr::SpriteObject::set_anim(size_t anim_num, float speed_factor) {
	if (anim_num < anims.size()) {
		anim_speed_factor = speed_factor;
		current_anim = anim_num;
		clock.restart();
		anim_time = sf::milliseconds(0);
		return true;
	}
	return false;
}

void gr::SpriteObject::animation_update() {
	anim_time += clock.getElapsedTime() * anim_speed_factor;
	clock.restart();
	
	try {
		current_sprite = anims[current_anim].first.get_frame(anim_time);
	} catch (std::logic_error& e) {
		con->log(get_message_prefix(this) + " Animation error (currnet_anim =" + std::to_string(current_anim) + "):" + e.what(), ConsoleMessageType::ERR);
	}
	return;
}

void gr::SpriteObject::draw(RTextures& rtex) {
	if (type == SPRITE_TYPE::DIFFUSE)
		rtex.diffuse.draw(*current_sprite);
	else if (type == SPRITE_TYPE::HEIGHT) {
		rtex.height.draw(*current_sprite, &rtex.height_shader);
		((void (*)(void)) rtex.glTextureBarrier)();
	}
}

void gr::SpriteObject::load_from_file(json& _json) {
	std::string texture_name, sprite_type;
	try {
		texture_name = _json["texture"].get<std::string>();
	} catch(nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"texture\" parameter type != string, or it does not exist", ConsoleMessageType::ERR);
		return;
	}

	try {
		sprite_type = _json["sprite_type"].get<std::string>();
		if (sprite_type == "diffuse")
			type = SPRITE_TYPE::DIFFUSE;
		else if (sprite_type == "height")
			type = SPRITE_TYPE::HEIGHT;
	}
	catch (nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"sprite_type\" parameter type != string, or it does not exist, set to diffuse as defalt", ConsoleMessageType::ERR);
		type = SPRITE_TYPE::DIFFUSE;
	}

	texture = Resource_manager<sf::Texture>::get_instance()->get_object(texture_name);
	if (texture == nullptr) {
		con->log(get_message_prefix(this) + " Error loading texture from file " + _json["texture"].get<std::string>());
		return;
	}

	try {
		layer_shift = _json["layer_shift"].get<double>();
	} catch (nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"layer_shift\" parameter type != double, or it does not exist. It is automaticly set to 0.0", ConsoleMessageType::ERR);
		layer_shift = 0.0;
	}

	try {
		json_to_coord(pos, size, _json["coordinates"]);
	} catch (nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"coordiantes\" parameter type error. Should be 4 float numbers", ConsoleMessageType::ERR);
		return;
	}

	for (auto& anim : _json["anims"].items()) {
		Animation new_anim;
		for (auto& frame : anim.value()) {
			try {
				auto spr = json_to_frame(frame);
				spr.first.setTexture(*texture);
				new_anim.add_frame(spr);
			} catch (nlohmann::detail::type_error err) {
				con->log(get_message_prefix(this) + " Json error while loading animations", ConsoleMessageType::ERR);
				return;
			}
		}
		anims.push_back(std::pair<Animation, std::string>(new_anim, anim.key()));
	}
	is_valid = true;
	return;
}

// Effect class

// Particle class

// LightAnimation

void gr::LightAnimation::add_frame(sf::Glsl::Vec3 color, sf::Time t) {
	full_anim_time += t;
	colors.push_back(std::pair<sf::Glsl::Vec3, sf::Time>(color, full_anim_time));
}

void gr::LightAnimation::add_frame(std::pair<sf::Glsl::Vec3, sf::Time> color_frame) {
	full_anim_time += color_frame.second;
	color_frame.second = full_anim_time;
	colors.push_back(color_frame);
}

sf::Glsl::Vec3 gr::LightAnimation::get_color(sf::Time time) {
	if (colors.size() == 0)
		throw std::logic_error("Animation size was 0 frames");
	if (full_anim_time == sf::milliseconds(0))
		throw std::logic_error("Full animation time was 0");

	std::pair<sf::Glsl::Vec3, sf::Time> prev_color((colors.end() - 1)->first, sf::milliseconds(0));
	time %= full_anim_time;
	for (auto& color : colors) {
		if (time < color.second) {
			if (!smooth)
				return color.first;
			else
				return (color.first * (time - prev_color.second).asSeconds() + prev_color.first * (color.second - time).asSeconds()) / (color.second - prev_color.second).asSeconds();
		}
		prev_color = color;
	}

	return colors[0].first;
}

// LightSource class

bool gr::LightSource::var_update(const Camera& cam) {
	if (!cam.intersects(this))
		return false;

	animation_update();

	glsl_pos = cam.world_to_glsl_pos(pos);
	glsl_size = cam.world_to_glsl_size(size);

	screen_pos = cam.world_to_pixel_pos(pos);
	screen_size = cam.world_to_pixel_size(size);

	layer = pos.y - size.y + layer_shift;
	return true;
}

void gr::LightSource::animation_update() {
	anim_time += clock.getElapsedTime() * anim_speed_factor;
	clock.restart();

	try {
		current_color = anims[current_anim].first.get_color(anim_time);
	}
	catch (std::logic_error& e) {
		con->log(get_message_prefix(this) + " Animation error (currnet_anim =" + std::to_string(current_anim) + "):" + e.what(), ConsoleMessageType::ERR);
	}
	return;
}


bool gr::LightSource::set_anim(size_t anim_num, float speed_factor) {
	if (anim_num < anims.size()) {
		anim_speed_factor = speed_factor;
		current_anim = anim_num;
		clock.restart();
		anim_time = sf::milliseconds(0);
		return true;
	}
	return false;
}

void gr::LightSource::draw(RTextures& rtex) {
	rtex.lightmap_shader.setUniform("light_position", glsl_pos);
	rtex.lightmap_shader.setUniform("light_size", glsl_size);
	rtex.lightmap_shader.setUniform("light_direction", direction);

	rtex.lightmap_sprite.setPosition(sf::Vector2f(0, float(rtex.light_counter)));
	rtex.lightmap.draw(rtex.lightmap_sprite, &rtex.lightmap_shader);
	rtex.light_counter++;

	//rtex.lightresult_sprite.setPosition(screen_pos);
	//rtex.lightresult_sprite.setScale(sf::Vector2f(screen_size.x / rtex.lightresult_sprite.getLocalBounds().width, screen_size.y / rtex.lightresult_sprite.getLocalBounds().height));
	//rtex.lightresult.draw(rtex.lightresult_sprite)
}

void gr::LightSource::load_from_file(json& _json) {
	try {
		json_to_coord(pos, size, _json["coordinates"]);
	}
	catch (nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"coordiantes\" parameter type error. Should be 4 float numbers", ConsoleMessageType::ERR);
		return;
	}

	try {
		direction = sf::Vector2f(_json["direction"][0].get<float>(), _json["direction"][1].get<float>());
	}
	catch (nlohmann::detail::type_error err) {
		con->log(get_message_prefix(this) + " Json \"direction\" parameter type error. Should be 2 float numbers", ConsoleMessageType::WARN);
	}

	for (auto& anim : _json["anims"].items()) {
		LightAnimation new_anim;
		for (auto& frame : anim.value()) {
			try {
				new_anim.add_frame(json_to_lightframe(frame));
			}
			catch (nlohmann::detail::type_error err) {
				con->log(get_message_prefix(this) + " Json error while loading animations", ConsoleMessageType::ERR);
				return;
			}
		}
		anims.push_back(std::pair<LightAnimation, std::string>(new_anim, anim.key()));
	}
	is_valid = true;


}

// GraphicsEngine class

gr::GraphicsEngine::GraphicsEngine() {
	rtex.diffuse.create(render_res); // ПОТОМ ПОМЕНЯТЬ
	rtex.height.create(render_res); // ПОТОМ ПОМЕНЯТЬ
	rtex.effectbuffer.create(render_res); // ПОТОМ ПОМЕНЯТЬ
	rtex.lightresult.create(render_res); // ПОТОМ ПОМЕНЯТЬ
	rtex.lightmap.create(sf::Vector2u(1536, 64)); // ПОТОМ ПОМЕНЯТЬ

	rtex.lightmap_sprite.setTexture(rtex.lightmap.getTexture());
	rtex.lightmap_sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(2048, 1)));
	rtex.lightresult_sprite.setTexture(rtex.diffuse.getTexture());
	rtex.lightresult_sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(1000, 1000)));

	rtex.lightmap_shader.loadFromFile("resources\\shaders\\lightmap_shader.txt", sf::Shader::Fragment);
	rtex.lightmap_shader.setUniform("obstacle_map", rtex.height.getTexture());

	rtex.height_shader.loadFromFile("resources\\shaders\\height_shader.txt", sf::Shader::Fragment);
	rtex.height_shader.setUniform("sprite", sf::Shader::CurrentTexture);
	rtex.height_shader.setUniform("height_resolution", f_render_res);
	rtex.height_shader.setUniform("height_result", rtex.height.getTexture());

	//combineShader.loadFromFile("resources\\shaders\\light_render_shader.txt", sf::Shader::Fragment);

	if (!sogl_loadOpenGL()) {
		const char** failures = sogl_getFailures();
		con->log("Error loading", ConsoleMessageType::ERR);
	}
	else {
		con->log("Sogl load succsessfull!", ConsoleMessageType::SUCC);
	}
	rtex.glTextureBarrier = sogl_loadOpenGLFunction("glTextureBarrier");
	sogl_cleanup();

	render_result.setTexture(rtex.lightmap.getTexture());

	cam.pos = sf::Vector2d(-8, 4.5);
	cam.size = sf::Vector2d(16, 9);
	cam.render_res = sf::Vector2f(1920, 1080);
}

void gr::GraphicsEngine::createRenderQueue() {
	render_queue.clear();
	for (auto obj : objects)
		if (obj->var_update(cam))
			render_queue.push_back(obj);
}

void gr::GraphicsEngine::sortRenderQueue() {
	sort(render_queue.begin(), render_queue.end(),
		[](Drawable* obj1, Drawable* obj2) {return (obj1->get_layer() > obj2->get_layer()); });
}

void gr::GraphicsEngine::setUniform() {
	rtex.lightmap_shader.setUniform("world_camera_size", sf::Vector2f(cam.size));
}

void gr::GraphicsEngine::render() {
	rtex.diffuse.clear(clear_color);
	rtex.height.clear(clear_color);
	rtex.light_counter = 0;

	createRenderQueue();
	sortRenderQueue();
	setUniform();

	for (auto obj : render_queue) {
		obj->draw(rtex);
	}

	

	rtex.diffuse.display();
	rtex.height.display();
}

sf::Sprite& gr::GraphicsEngine::get_sprite() {
	return render_result;
}

void gr::GraphicsEngine::load_from_file(json& _json) {
	sf::Clock clock;
	con->log("Loading " + std::to_string(_json.size()) + " graphics objects...", ConsoleMessageType::INFO);

	for (auto& json_obj : _json.items()) {
		std::string type;
		Drawable* obj = nullptr;

		try {
			type = json_obj.value()["type"].get<std::string>(); // maybe try catch ? 
		}
		catch (...) { // BRUH
			con->log("Unable to find \"type\" parameter (or some other error), while loading object: " + json_obj.key(), ConsoleMessageType::ERR);
			continue;
		}

		if (type == "LightSource") {
			obj = new LightSource;
			
		}
		else if (type == "Particle") {

		}
		else if (type == "Effect") {

		}
		else if (type == "SpriteObject") {
			obj = new SpriteObject;
		}

		// final part
		if (obj != nullptr) {
			obj->load_from_file(json_obj.value());
			obj->name = json_obj.key();
			objects.push_back(obj);
		}
	}

	con->log("Graphics objects loading done! Time in milliseconds (except parsing): " + std::to_string(clock.getElapsedTime().asMilliseconds()), ConsoleMessageType::SUCC);
	con->log("Loaded graphics objects: " + std::to_string(objects.size()), ConsoleMessageType::SUCC);
	Resource_manager<sf::Texture>::get_instance()->print();
}

gr::GraphicsEngine::~GraphicsEngine() {
	for (auto obj : objects)
		delete obj;
}