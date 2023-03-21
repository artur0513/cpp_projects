#pragma once
#include <iostream>
#include <cmath>
#include "vec4.h"
#include "quat.h"

struct PersProjInfo { // информация о матрице перспективной проекции (float or double?)
	float fov_y; // Угол обзора
	float aspect_ratio; // Соотношение сторон
	float z_far; // Дальняя плоскость отсечения
	float z_near; // Ближняя плоскость отсечения
};

template <class T>
struct mat4 {
private:
	T data[16]; // column-major format

	friend struct mat4;

	template <class T>
	struct mat4line {
	private:
		friend struct mat4;
		T* m[4];
	public:
		mat4line(T& m0, T& m1, T& m2, T& m3) {
			m[0] = &m0;
			m[1] = &m1;
			m[2] = &m2;
			m[3] = &m3;
		}

		inline mat4line<T>& operator=(const vec4<T>& v) {
			*m[0] = v.x;
			*m[1] = v.y;
			*m[2] = v.z;
			*m[3] = v.w;
			return *this;
		}

		inline T& operator[](int n) {
			return *m[n];
		}
	};

public:
	// Default copy constructor works OK!

	mat4() {};

	mat4(T a0, T a4, T a8, T a12,
		T a1, T a5, T a9, T a13,
		T a2, T a6, T a10, T a14,
		T a3, T a7, T a11, T a15) {
		data[0] = a0; data[4] = a4; data[8] = a8; data[12] = a12;
		data[1] = a1; data[5] = a5; data[9] = a9; data[13] = a13;
		data[2] = a2; data[6] = a6; data[10] = a10; data[14] = a14;
		data[3] = a3; data[7] = a7; data[11] = a11; data[15] = a15;
	}

	template<class U>
	mat4(const mat4<U>& v) {
		for (int i = 0; i < 16; i++)
			data[i] = v.data[i];
	}

	template<class U>
	inline mat4<T>& operator=(const mat4<U>& v) {
		for (int i = 0; i < 16; i++)
			data[i] = v.data[i];
		return *this;
	}

	inline mat4line<T> operator[](int n) {
		n *= 4;
		return mat4line<T>(data[n], data[n + 1], data[n + 2], data[n + 3]);
	}

	inline const T* operator()() const {
		return &data[0];
	}

	template <class T>
	friend std::ostream& operator<< (std::ostream& stream, const mat4<T>& matrix);

	template <class T>
	friend mat4<T> operator*(const mat4<T>& l, const mat4<T>& r);

	template <class T>
	friend vec4<T> operator*(const vec4<T>& l, const mat4<T>& r);

	mat4<T>& init_identity() {
		data[0] = 1; data[4] = 0; data[8] = 0; data[12] = 0;
		data[1] = 0; data[5] = 1; data[9] = 0; data[13] = 0;
		data[2] = 0; data[6] = 0; data[10] = 1; data[14] = 0;
		data[3] = 0; data[7] = 0; data[11] = 0; data[15] = 1;
		return *this;
	}

	mat4<T>& init_perspective(const PersProjInfo& p) {
		data[0] = 1.0 / tan(p.fov_y / 2) / p.aspect_ratio; data[4] = 0.0; data[8] = 0.0; data[12] = 0.0;
		data[1] = 0.0; data[5] = 1.0 / tan(p.fov_y / 2.0); data[9] = 0.0; data[13] = 0.0;
		data[2] = 0.0; data[6] = 0.0; data[10] = (p.z_far + p.z_near) / (p.z_far - p.z_near); data[14] = 1.0;
		data[3] = 0.0; data[7] = 0.0; data[11] = - 2.0 * p.z_far * p.z_near / (p.z_far - p.z_near); data[15] = 0.0;
		return *this;
	}

	mat4<T>& init_rotation_X(T angle) {
		data[0] = 1.0; data[4] = 0.0;        data[8] = 0.0;         data[12] = 0.0;
		data[1] = 0.0; data[5] = cos(angle); data[9] = -sin(angle); data[13] = 0.0;
		data[2] = 0.0; data[6] = sin(angle); data[10] = cos(angle); data[14] = 0.0;
		data[3] = 0.0; data[7] = 0.0;        data[11] = 0.0;        data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_rotation_Y(T angle) {
		data[0] = cos(angle); data[4] = 0.0; data[8] = -sin(angle); data[12] = 0.0;
		data[1] = 0.0;        data[5] = 1.0; data[9] = 0.0;         data[13] = 0.0;
		data[2] = sin(angle); data[6] = 0.0; data[10] = cos(angle); data[14] = 0.0;
		data[3] = 0.0;        data[7] = 0.0; data[11] = 0.0;        data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_rotation_Z(T angle) {
		data[0] = cos(angle); data[4] = -sin(angle); data[8] = 0.0;  data[12] = 0.0;
		data[1] = sin(angle); data[5] = cos(angle);  data[9] = 0.0;  data[13] = 0.0;
		data[2] = 0.0;        data[6] = 0.0;         data[10] = 1.0; data[14] = 0.0;
		data[3] = 0.0;        data[7] = 0.0;         data[11] = 0.0; data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_scale(T scale_x, T scale_y, T scale_z) {
		data[0] = scale_x; data[4] = 0.0;     data[8] = 0.0;      data[12] = 0.0;
		data[1] = 0.0;     data[5] = scale_y; data[9] = 0.0;      data[13] = 0.0;
		data[2] = 0.0;     data[6] = 0.0;     data[10] = scale_z; data[14] = 0.0;
		data[3] = 0.0;     data[7] = 0.0;     data[11] = 0.0;     data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_transfer(T move_x, T move_y, T move_z) {
		data[0] = 1.0; data[4] = 0.0; data[8] = 0.0; data[12] = move_x;
		data[1] = 0.0; data[5] = 1.0; data[9] = 0.0; data[13] = move_y;
		data[2] = 0.0; data[6] = 0.0; data[10] = 1.0; data[14] = move_z;
		data[3] = 0.0; data[7] = 0.0; data[11] = 0.0; data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_rotation(const quat<T>& q) {
		T y2 = 2.0 * q.y * q.y, x2 = 2.0 * q.x * q.x, z2 = 2.0 * q.z * q.z,
			xy = 2.0 * q.x * q.y, xz = 2.0 * q.z * q.x, yz = 2.0 * q.y * q.z,
			wx = 2.0 * q.w * q.x, wy = 2.0 * q.w * q.y, wz = 2.0 * q.w * q.z;

		data[0] = 1.0 - y2 - z2; data[4] = xy - wz;       data[8] = xz + wy;        data[12] = 0.0;
		data[1] = xy + wz;       data[5] = 1.0 - x2 - z2; data[9] = yz - wx;        data[13] = 0.0;
		data[2] = xz - wy;       data[6] = yz + wx;       data[10] = 1.0 - x2 - y2; data[14] = 0.0;
		data[3] = 0.0;           data[7] = 0.0;           data[11] = 0.0;           data[15] = 1.0;
		return *this;
	}

	mat4<T>& init_transfer(const vec3<T>& move) {
		init_transfer(move.x, move.y, move.z);
		return *this;
	}

	//void inverse() to be added

	//T determinant() to be added

	//void init_rotation(const quaternion& quat) - to be added
};

template <class T>
std::ostream& operator<< (std::ostream& stream, const mat4<T>& matrix) {
	stream << matrix.data[0] << " " << matrix.data[4] << " " << matrix.data[8] << " " << matrix.data[12] << "\n";
	stream << matrix.data[1] << " " << matrix.data[5] << " " << matrix.data[9] << " " << matrix.data[13] << "\n";
	stream << matrix.data[2] << " " << matrix.data[6] << " " << matrix.data[10] << " " << matrix.data[14] << "\n";
	stream << matrix.data[3] << " " << matrix.data[7] << " " << matrix.data[11] << " " << matrix.data[15] << "\n";
	return stream;
}

template <class T>
inline mat4<T> operator*(const mat4<T>& l, const mat4<T>& r) {
	mat4<T> ret;

	for (unsigned i = 0; i < 4; i++) {
		for (unsigned j = 0; j < 4; j++) { // i - строка, j - столбец
			unsigned qj = j * 4; // quatro - j

			ret.data[j * 4 + i] = l.data[i] * r.data[qj] +
				l.data[i + 4] * r.data[qj + 1] +
				l.data[i + 8] * r.data[qj + 2] +
				l.data[i + 12] * r.data[qj + 3];
		}
	}

	return ret;
}

template <class T>
inline vec4<T> operator*(const vec4<T>& l, const mat4<T>& r) {
	vec4<T> ret;
	
	ret.x = l.x * r.data[0] + l.y * r.data[1] + l.z * r.data[2] + l.w * r.data[3];
	ret.y = l.x * r.data[4] + l.y * r.data[5] + l.z * r.data[6] + l.w * r.data[7];
	ret.z = l.x * r.data[8] + l.y * r.data[9] + l.z * r.data[10] + l.w * r.data[11];
	ret.w = l.x * r.data[12] + l.y * r.data[13] + l.z * r.data[14] + l.w * r.data[15];
	
	return ret;
}