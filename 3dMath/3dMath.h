#pragma once
#include <iostream>
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"
#include "quat.h"

template<class T>
std::ostream& operator<< (std::ostream& stream, const vec2<T>& v) {
	stream << "(" << v.x << ", " << v.y << ")";
	return stream;
}

template<class T>
std::ostream& operator<< (std::ostream& stream, const vec3<T>& v) {
	stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return stream;
}

template<class T>
std::ostream& operator<< (std::ostream& stream, const vec4<T>& v) {
	stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	return stream;
}

template <class T>
std::ostream& operator<< (std::ostream& stream, const mat4<T>& matrix) {
	stream << matrix.data[0] << " " << matrix.data[4] << " " << matrix.data[8] << " " << matrix.data[12] << "\n";
	stream << matrix.data[1] << " " << matrix.data[5] << " " << matrix.data[9] << " " << matrix.data[13] << "\n";
	stream << matrix.data[2] << " " << matrix.data[6] << " " << matrix.data[10] << " " << matrix.data[14] << "\n";
	stream << matrix.data[3] << " " << matrix.data[7] << " " << matrix.data[11] << " " << matrix.data[15] << "\n";
	return stream;
}

template<class T>
std::ostream& operator<< (std::ostream& stream, const quat<T>& v) {
	stream << v.w << " + " << v.i << "*i + " << v.j << "*j + " << v.k << "*k";
	return stream;
}

using vec2i = vec2<int>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;

using vec3i = vec3<int>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;

using vec4i = vec4<int>;
using vec4f = vec4<float>;
using vec4d = vec4<double>;

using mat4i = mat4<int>;
using mat4f = mat4<float>;
using mat4d = mat4<double>;

using quatf = quat<float>;
using quatd = quat<double>;

/*
Обернуть все в namespace
Все дублируем в double чтобы попробовать потом сделать двойную точность координат.

Добавить:
В матрицу 4x4
Инициализацию как матрицу масштаба, поворота, параллельного переноса, персепективной проекции, и потом еще как матрицу поворота из кватерниона, м.б. еще что то
Умножение на вектор тоже наверное
Сумму разность вроде особо не надо
!!! Важно (наверное). Все критичные к производительности функции с матрицой должны обращаться к ее элементам напрямую через mat.data[4*j + i],
а не через мой двойной [j][i] - такой вариант скорее всего сильно медленее (преждевременная оптимизация, хехе)

Возможно нужны матрицы 3х3, не уверен
Кватернионы конечно
И далее по необходимости, что понадобится. Возможно свои структуры для данных о матрицы проекции, каких нибудь Viewing frustum, и поиска всяких пересечений, хз
*/