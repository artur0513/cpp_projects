#pragma once

template<class T>
struct vec3 {
	T x, y, z;

	vec3() {};

	vec3(T _x, T _y, T _z) {
		x = _x;
		y = _y;
		z = _z;
	}
};
