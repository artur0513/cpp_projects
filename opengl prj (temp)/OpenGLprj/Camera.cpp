#include "Camera.h"
#include <glfw3.h>

ogl::Camera::Camera(m3d::PersProjInfo& _pInfo) {
	pInfo = &_pInfo;
	pos = m3d::vec3f(0.0, 0.0, 0.0);
	dir = m3d::vec3f(0.0, 0.0, 1.0);
}

ogl::Camera::Camera(m3d::PersProjInfo& _pInfo, const m3d::vec3f& _pos, const m3d::vec3f& _dir) {
	pInfo = &_pInfo;
	pos = _pos;
	dir = _dir;
}

void ogl::Camera::keyboardMove(int key, int action) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W)
			forward = true;

		if (key == GLFW_KEY_S)
			backward = true;

		if (key == GLFW_KEY_D)
			right = true;

		if (key == GLFW_KEY_A)
			left = true;

		if (key == GLFW_KEY_SPACE)
			up = true;

		if (key == GLFW_KEY_LEFT_CONTROL)
			down = true;

		if (key == GLFW_KEY_LEFT_SHIFT)
			shift = true;
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W)
			forward = false;

		if (key == GLFW_KEY_S)
			backward = false;

		if (key == GLFW_KEY_D)
			right = false;

		if (key == GLFW_KEY_A)
			left = false;

		if (key == GLFW_KEY_SPACE)
			up = false;

		if (key == GLFW_KEY_LEFT_CONTROL)
			down = false;

		if (key == GLFW_KEY_LEFT_SHIFT)
			shift = false;
	}

}

void ogl::Camera::update() {
	speed = m3d::vec3f(0.f, 0.f, 0.f);
	speed += (double(left) - double(right)) * m3d::normalize(m3d::cross(dir, upVec));
	speed += (double(forward) - double(backward)) * m3d::normalize(dir);
	speed += (double(up) - double(down)) * upVec;
	
	pos += speed * defSpeed * (1.0 + double(shift)) * double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - prevTimePoint).count())/1000000.0;
	prevTimePoint = std::chrono::steady_clock::now();

}

const m3d::vec3f ogl::Camera::getPosition() {
	update();
	return pos;
}

const m3d::vec3f ogl::Camera::getDirection() { 
	update();
	return dir;
}

const m3d::vec3f ogl::Camera::getUpVector() {
	update();
	return upVec;
}

const m3d::mat4f ogl::Camera::getCameraTransform() {
	update();
	return m3d::mat4f().init_camera_transform(dir)*m3d::mat4f().init_transfer(-1.0 * pos);
}

void ogl::Camera::mouseMove(double deltaX, double deltaY) {
	auto newQuat = m3d::quatd(deltaX, upVec) * dir * m3d::quatd(deltaX, -1.0 * upVec);
	dir = m3d::vec3d(newQuat.i, newQuat.j, newQuat.k);

	auto axis = m3d::normalize(cross(dir, upVec));
	newQuat = m3d::quatd(deltaY, -1.0 * axis) * dir * m3d::quatd(deltaY, axis);
	dir = m3d::vec3d(newQuat.i, newQuat.j, newQuat.k);
}
