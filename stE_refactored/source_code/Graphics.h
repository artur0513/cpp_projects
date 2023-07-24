#pragma once
#include "Basics.h"
#include "ResourceManager.h"
#include "Console.h"
#include "json.hpp"

using json = nlohmann::json;

namespace gr {

	void json_to_coord(sf::Vector2d& pos, sf::Vector2d& size, json & _json);

	std::pair<sf::Sprite, sf::Time> json_to_frame(json& _json);

	std::pair<sf::Glsl::Vec3, sf::Time> json_to_lightframe(json& _json);

	enum class SPRITE_TYPE : bool {
		DIFFUSE,
		HEIGHT
	};

	struct RTextures {
		sf::RenderTexture diffuse;
		sf::RenderTexture height;
		sf::RenderTexture lightresult;
		sf::RenderTexture effectbuffer;
		sf::RenderTexture lightmap;

		int light_counter = 0; // Счетчик уже отрендеренных источников света

		sf::Shader height_shader;
		sf::Shader lightmap_shader;

		void(*glTextureBarrier); //краеугольный камень для карты высот

		sf::Sprite lightmap_sprite, lightresult_sprite;
	};

	// Класс похожий на sf::Rect<double>, но чуть более мне удобный
	class Rect {
	public:
		sf::Vector2d pos, size; // pos = левый верхний угол

		Rect();

		Rect(double pos_x, double pos_y, double size_x, double size_y);

		Rect(sf::Vector2d _pos, sf::Vector2d _size);

		bool intersects(const Rect* rect) const; // Проверка пересечения в другим прямоугольником 
	};

	class Camera : public Rect {
	public:
		double aspect_ratio;
		sf::Vector2f render_res;

		sf::Vector2f world_to_pixel_pos(sf::Vector2d world_pos) const;

		sf::Vector2f world_to_pixel_size(sf::Vector2d world_size) const;

		sf::Vector2d pixel_to_world_pos(sf::Vector2f camera_pos) const;

		sf::Vector2f world_to_glsl_pos(sf::Vector2d world_pos) const;

		sf::Vector2f world_to_glsl_size(sf::Vector2d world_size) const;
	};

	class Animation {
	private:
		std::deque<std::pair<sf::Sprite, sf::Time>> frames;
		sf::Time full_anim_time = sf::milliseconds(0);

	public:
		void add_frame(sf::Sprite spr, sf::Time t);

		void add_frame(std::pair<sf::Sprite, sf::Time> frame);

		sf::Sprite* get_frame(sf::Time time);
	};

	class Drawable : public Rect {
	protected:
		double layer = pos.y - size.y, layer_shift = 0.0;

	public:
		std::string name;

		virtual bool var_update(const Camera& cam) = 0;

		virtual void animation_update() = 0;

		virtual void load_from_file(json& _json) = 0;

		virtual void draw(RTextures& rtex) = 0;

		double get_layer();
	};

	class SpriteObject final : public Drawable {
	private:
		sf::Sprite* current_sprite;
		std::deque<std::pair<Animation, std::string>> anims; // Все анимации спрайта. Теперь нету default_sprite, он как-бы просто лежит в anims[0]

		size_t current_anim = 0;
		float anim_speed_factor = 1.0f;

		sf::Clock clock;
		sf::Time anim_time;

		bool is_valid = false; // Корректно ли загружен обьект и загружен ли вообще, т.е. можно ли его использовать
		sf::Texture* texture;
	public:
		SPRITE_TYPE type;

		bool set_anim(size_t anim_num, float speed_factor); // Пробуем врубить анимации под заданным номером. true если анимация существует

		bool var_update(const Camera& cam) override final;

		void animation_update() override final;

		void draw(RTextures& rtex) override final;

		void load_from_file(json& _json) override final;
	};

	class Effect : public Drawable {

	};

	class Particle : public Drawable {

	};

	// Очень похоже на Animation, но решил не делать темплейт
	class LightAnimation {
	private:
		std::deque<std::pair<sf::Glsl::Vec3, sf::Time>> colors;
		sf::Time full_anim_time = sf::milliseconds(0);

	public:
		bool smooth = true; // Плавная анимация
		float light_factor = 1.f;

		void add_frame(sf::Glsl::Vec3 color, sf::Time t);

		void add_frame(std::pair<sf::Glsl::Vec3, sf::Time> color_frame);

		sf::Glsl::Vec3 get_color(sf::Time time);
	};

	class LightSource : public Drawable {// Для единообразия здесь pos будет также левым верхним углом, в отличии от прошлой версии где координаты ист. света задавались через центр
	private:
		sf::Glsl::Vec3 current_color;
		std::deque<std::pair<LightAnimation, std::string>> anims;

		sf::Glsl::Vec2 glsl_pos, glsl_size;
		sf::Glsl::Vec2 screen_pos, screen_size;
		sf::Vector2f direction = sf::Vector2f(0.0, 7.0);

		size_t current_anim = 0;
		float anim_speed_factor = 1.0f;

		sf::Clock clock;
		sf::Time anim_time;

		bool is_valid = false;
	public:
		bool set_anim(size_t anim_num, float speed_factor);

		bool var_update(const Camera& cam) override final;

		void animation_update() override final;

		void draw(RTextures& rtex) override final;

		void load_from_file(json& _json) override final;
	};

	// Разметка классов очень примерная пока
	class GraphicsEngine {
	private:
		const sf::Color clear_color = sf::Color(0, 0, 0, 0);
		const int MAX_LIGHT_COUNT = 32;
		const int SHADER_UPD_FREQ = 20;

		sf::Vector2u render_res = sf::Vector2u(1920, 1080), output_res;
		sf::Vector2f f_render_res = sf::Vector2f(render_res);

		RTextures rtex;
		sf::Clock render_clock;

		sf::Sprite render_result;

		Camera cam;

		std::deque<Drawable*> objects; // Все обьекты на карте
		std::deque<Drawable*> render_queue; // Очередь отрисовки

		//sf::Glsl::Mat3 color_correction; // !!! Вынести все постэффекты в отдельный класс !!!!

		sf::Shader lightmapShader, combineShader, posteffectShader, heightShader;

		void createRenderQueue(); // Создать очередь отрисовки = найти все обьекты в кадре

		void sortRenderQueue(); // Отсортировать все обьекты в кадре = diffuse, height, effect по layer, свет в конец
		// Всего обьектов в кадре даже если 1000 будет, сортировка будет ультра быстрой, и можно все типы обьектов в кучу пихать

		void renderLightSource(LightSource* src); // Для каждого источника вызываем шейдер и рисуем одну строчку в lightmap

		void setUniform(); // Отправить часть параметров (погодных, эффектов) в шейдер (ту часть, что можно обновлять не каждый кадр)

	public:
		GraphicsEngine();

		void render();

		void load_from_file(json& _json);

		sf::Sprite& get_sprite();

		~GraphicsEngine();
	};

}