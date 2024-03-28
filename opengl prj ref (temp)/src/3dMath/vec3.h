#pragma once
#define UP_VECTOR m3d::vec3<float>(0.0f, 1.0f, 0.0f)

namespace m3d {

	template<class T>
	struct vec3 {
		T x = 0.0, y = 0.0, z = 0.0;

		vec3() {};

		vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

		template<class U>
		vec3(const vec3<U>& v) : x(v.x), y(v.y), z(v.z) {}

		template<class U>
		inline vec3<T>& operator=(const vec3<U>& v) {
			x = v.x; y = v.y, z = v.z;
			return *this;
		}

		inline vec3<T>& operator+=(const vec3<T>& v) {
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		inline vec3<T>& operator-=(const vec3<T>& v) {
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		inline vec3<T>& operator*=(const T& v) {
			x *= v;
			y *= v;
			z *= v;
			return *this;
		}

		inline vec3<T>& operator/=(const T& v) {
			x /= v;
			y /= v;
			z /= v;
			return *this;
		}
	};

	template<class T>
	inline vec3<T> operator+(const vec3<T>& l, const vec3<T>& r) {
		return vec3<T>(l.x + r.x, l.y + r.y, l.z + r.z);
	}

	template<class T>
	inline vec3<T> operator-(const vec3<T>& l, const vec3<T>& r) {
		return vec3<T>(l.x - r.x, l.y - r.y, l.z - r.z);
	}

	template<class T>
	inline vec3<T> operator*(const vec3<T>& l, const T& r) {
		return vec3<T>(l.x * r, l.y * r, l.z * r);
	}

	template<class T>
	inline vec3<T> operator*(const T& l, const vec3<T>& r) {
		return vec3<T>(l * r.x, l * r.y, l * r.z);
	}

	template<class T>
	inline vec3<T> operator/(const vec3<T>& l, const T& r) {
		return vec3<T>(l.x / r, l.y / r, l.z / r);
	}

	template<class T>
	inline vec3<T> cross(const vec3<T>& l, const vec3<T>& r) {
		return vec3<T>(l.y * r.z - l.z * r.y, l.x * r.z - l.z * r.x, l.x * r.y - l.y * r.x);
	}

	template<class T>
	inline T dot(const vec3<T>& l, const vec3<T>& r) {
		return l.x * r.x + l.y * r.y + l.z * r.z;
	}

	template<class T>
	inline T cos(const vec3<T>& l, const vec3<T>& r) {
		static_assert(std::is_floating_point<T>::value, "This operation is supported only for floating point types");
		return dot(l, r) / length(l) / length(r);
	}

	template<class T>
	inline T length(const vec3<T>& v) {
		static_assert(std::is_floating_point<T>::value, "This operation is supported only for floating point types");
		return std::sqrt(dot(v, v));
	}

	template<class T>
	inline T norm(const vec3<T>& v) {
		static_assert(std::is_floating_point<T>::value, "This operation is supported only for floating point types");
		return dot(v, v);
	}

	template<class T>
	[[nodiscard]] inline vec3<T> normalize(const vec3<T>& v) {
		static_assert(std::is_floating_point<T>::value, "This operation is supported only for floating point types");
		return v / length(v);
	}

}