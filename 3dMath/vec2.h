#pragma once
#include <cmath>

// I dont like these templates, what should i do?

template<class T>
struct vec2 {
	T x, y;

	vec2() {};

	vec2(T _x, T _y) {
		x = _x;
		y = _y;
	}

	inline vec2<T>& operator+=(const vec2<T>& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	inline vec2<T>& operator-=(const vec2<T>& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline vec2<T>& operator*=(const T& v) {
		x *= v;
		y *= v;
		return *this;
	}

	inline vec2<T>& operator/=(const T& v) {
		x /= v;
		y /= v;
		return *this;
	}
};

template<class T>
inline vec2<T> operator+(const vec2<T>& l, const vec2<T>& r) {
	return vec2<T>( l.x + r.x, l.y + r.y );
}

template<class T>
inline vec2<T> operator-(const vec2<T>& l, const vec2<T>& r) {
	return vec2<T>(l.x - r.x, l.y - r.y);
}

template<class T>
inline vec2<T> operator*(const vec2<T>& l, const T& r) {
	return vec2<T>(l.x * r, l.y * r);
}

template<class T>
inline vec2<T> operator/(const vec2<T>& l, const T& r) {
	return vec2<T>(l.x / r, l.y / r);
}

template<class T>
inline T dot(const vec2<T>& l, const vec2<T>& r) {
	return l.x * r.x + l.y * r.y;
}

template<class T>
inline T cos(const vec2<T>& l, const vec2<T>& r) {
	return dot(l, r) / length(l) / length(r);
}

template<class T>
inline T length(const vec2<T>& v) {
	return std::sqrt(dot(v, v));
}

template<class T>
inline T normalize(const vec2<T>& v) {
	return v/length(v);
}

// for vec3 also add cross product