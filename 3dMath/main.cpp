#include <iostream>
#include "3dMath.h"

int main()
{
    vec3f a = {1.0f, 2.0f, 0.3f}; // WTF как это работает епта? Работает все таки с классами и со структурами, прикол
    vec3f b = a;

    vec2f c = { 1.24, 2.25 };
    vec2f d = { 0.543, 3.345 };
    vec2d e = c;

    std::cout << e.y << std::endl;

    std::cout << (c+d).y << std::endl;
    std::cout << length(c) << std::endl;

    mat4f x;
    mat4d z(x);

    x[0][0] = 1.f; x[1][0] = 2.f; x[2][0] = 1.f; x[3][0] = 2.f;
    x[0][1] = 0.f; x[1][1] = 3.f; x[2][1] = 4.f; x[3][1] = 2.f;
    x[0][2] = 0.f; x[1][2] = 0.f; x[2][2] = 5.f; x[3][2] = 6.f;
    x[0][3] = 0.f; x[1][3] = 0.f; x[2][3] = 0.f; x[3][3] = 7.f;

    mat4 y(1.f, 0.f, 0.f, 0.f,
        0.f, 3.f, 0.f, 0.f,
        0.f, 0.f, 5.f, 0.f,
        0.f, 0.f, 0.f, 7.f);

    std::cout << x * y; // OK!

    std::cout << x * mat4f().init_identity(); // anonymous OK!

    quat<float> q(5, 1, 2, 3);
    std::cout << q;
}
