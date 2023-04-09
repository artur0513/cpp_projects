#include "Camera.h"
#include <glfw3.h>

ogl::Camera::Camera(m3d::PersProjInfo& _pInfo) {
	pInfo = &_pInfo;
	pos = m3d::vec3f(0.0, 0.0, 0.0);
	dir = m3d::vec3f(1.0, 0.0, 0.0);
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
	}

}

void ogl::Camera::update() {
	speed = m3d::vec3f(0.f, 0.f, 0.f);
	speed += (float(forward) - float(backward)) * m3d::normalize(dir);
	speed += (float(right) - float(left)) * m3d::normalize(m3d::cross(dir, up));

	
	pos += speed * float(double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - prevTimePoint).count())/1000000.0);
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
	return up;
}
