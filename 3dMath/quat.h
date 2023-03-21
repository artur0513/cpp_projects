#pragma once
#include <cmath>
#include <type_traits>

template<class T>
struct quat {
	static_assert(std::is_floating_point<T>::value, "Quaternions should be used only with floating point types");

	union { T w, r; };
	union { T x, i; };
	union { T y, j; };
	union { T z, k; };

	quat() {}

	quat(T _w, T _x, T _y, T _z) {
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	}

	template<class U>
	quat(const quat<U>& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}

	quat(T angle, const vec3<T>& dir) {
		T sinhalf = sin(dir / 2.0);
		w = cos(dir / 2.0);
		x = dir.x * sinhalf;
		y = dir.y * sinhalf;
		z = dir.z * sinhalf;
	}

	template<class U>
	inline quat<T>& operator=(const quat<U>& q) {
		w = q.w; x = q.x; y = q.y; z = q.z;
		return *this;
	}

	inline quat<T>& operator*(const T& v) {
		x *= v;
		y *= v;
		z *= v;
		w *= v;
		return *this;
	}

	inline quat<T>& operator/(const T& v) {
		x /= v;
		y /= v;
		z /= v;
		w /= v;
		return *this;
	}
};

template<class T>
inline T length(quat<T> q) {
	return sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

template<class T>
[[nodiscard]] inline quat<T> normalize(const quat<T>& q) {
	return q / length(q);
}

template<class T>
inline quat<T> operator*(const quat<T>& l, const quat<T>& r) {
	quat<T> ret;

	ret.w = l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z;
	ret.x = l.x * r.w + l.w * r.x + l.y * r.z - l.z * r.y;
	ret.y = l.y * r.w + l.w * r.y + l.z * r.x - l.x * r.z;
	ret.z = l.z * r.w + l.w * r.z + l.x * r.y - l.y * r.x;

	return ret;
}