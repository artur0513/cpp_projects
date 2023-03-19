#pragma once

template<class T>
struct vec4 {
	T x, y, z, w;

	vec4() {};

	vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

	inline vec4<T>& operator+=(const vec4<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	inline vec4<T>& operator-=(const vec4<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	inline vec4<T>& operator*=(const T& v) {
		x *= v;
		y *= v;
		z *= v;
		w *= v;
		return *this;
	}

	inline vec4<T>& operator/=(const T& v) {
		x /= v;
		y /= v;
		z /= v;
		w /= v;
		return *this;
	}
};

template<class T>
inline vec4<T> operator+(const vec4<T>& l, const vec4<T>& r) {
	return vec4<T>(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
}

template<class T>
inline vec4<T> operator-(const vec4<T>& l, const vec4<T>& r) {
	return vec4<T>(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
}

template<class T>
inline vec4<T> operator*(const vec4<T>& l, const T& r) {
	return vec4<T>(l.x * r, l.y * r, l.z * r, l.w * r);
}

template<class T>
inline vec4<T> operator*(const T& l, const vec4<T>& r) {
	return vec4<T>(l * r.x, l * r.y, l * r.z, l * r.w);
}

template<class T>
inline vec4<T> operator/(const vec4<T>& l, const T& r) {
	return vec4<T>(l.x / r, l.y / r, l.z / r, l.w / r);
}

template<class T>
inline T dot(const vec4<T>& l, const vec4<T>& r) {
	return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}

template<class T>
inline T cos(const vec4<T>& l, const vec4<T>& r) {
	return dot(l, r) / length(l) / length(r);
}

template<class T>
inline T length(const vec4<T>& v) {
	return std::sqrt(dot(v, v));
}

template<class T>
inline T normalize(const vec4<T>& v) {
	return v / length(v);
}
