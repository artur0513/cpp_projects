#pragma once
#include "3dMath/3dMath.h"

struct Camera {
	m3d::PersProjInfo pInfo;

	m3d::vec3d pos;
	double phi, theta;

	m3d::vec3d getDirection();

};

