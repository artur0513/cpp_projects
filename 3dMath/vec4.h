#pragma once

template<class T>
struct vec4 {
	T x, y, z, w;

	vec4() {};

	vec4(T _x, T _y, T _z, T _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};