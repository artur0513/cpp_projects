#include "3dMath.h"

// �������� ��� ��������� ���, ����� ������� dll? ��� ������������ ��� ��� ���� �����

template struct vec2<int>;
template struct vec2<float>;
template struct vec2<double>;

template struct vec3<int>;
template struct vec3<float>;
template struct vec3<double>;

template struct vec4<int>;
template struct vec4<float>;
template struct vec4<double>;

template struct mat4<float>;
template struct mat4<double>;

template struct quat<float>;
template struct quat<double>;

//template vec2<int> operator+<int>(const vec2<int>& l, const vec2<int>& r);