#pragma once
#include <iostream>
#include "vec4.h"

template <class T>
struct mat4 {
private:
	T data[16]; // column-major format

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

			ret.data[j * 4 + i] = l.data[i] * r.data[j * 4] +
				l.data[i + 4] * r.data[j * 4 + 1] +
				l.data[i + 8] * r.data[j * 4 + 2] +
				l.data[i + 12] * r.data[j * 4 + 3];
		}
	}

	return ret;
}