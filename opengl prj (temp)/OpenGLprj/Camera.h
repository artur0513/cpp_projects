#pragma once
#include "3dMath/3dMath.h"
#include <chrono>

namespace ogl {

	class Camera{
	private:
		m3d::PersProjInfo* pInfo;
		m3d::vec3d pos, dir, upVec = UP_VECTOR;
		
		std::chrono::steady_clock::time_point prevTimePoint;
		m3d::vec3d speed;
		bool forward = false, backward = false, left = false, right = false; // speed
		bool up = false, down = false;
		bool shift = false;

		double defSpeed = 1.f;
	public:
		Camera(m3d::PersProjInfo& _pInfo);
		Camera(m3d::PersProjInfo& _pInfo, const m3d::vec3f& pos, const m3d::vec3f& dir);
		
		void keyboardMove(int key, int action);
		void mouseMove(double xpos, double ypos);
		void update();

		const m3d::vec3f getPosition();
		const m3d::vec3f getDirection();
		const m3d::vec3f getUpVector();

		// Tranform without perspective matrix
		const m3d::mat4f getCameraTransform();
	};

}