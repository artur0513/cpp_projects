#pragma once
#include <cmath>

template<class T>
struct vec2 {
	T x, y;

	vec2() {};

	vec2(T _x, T _y) : x(_x), y(_y) {}

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

	inline vec2<T>& normalize() {
		*this /= length(*this);
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
inline vec2<T> operator*(const T& l, const vec2<T>& r) {
	return vec2<T>(l * r.x, l * r.y);
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
inline vec2<T> normalize(const vec2<T>& v) {
	return v/length(v);
}