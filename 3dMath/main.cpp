#include <iostream>
#include "3dMath.h"

int main()
{
    vec3f a = {1.0f, 2.0f, 0.3f}; // WTF как это работает епта? Работает все таки с классами и со структурами, прикол
    vec3f b = a;
    std::cout << b.normalize().x << " " << b.normalize().y << " " << b.normalize().z << std::endl;


    vec2d c = { 1, 2 };
    vec2d d = { 0, 3 };

    std::cout << (c+d).y << std::endl;
    std::cout << length(c) << std::endl;

    mat4f x, y;
    mat4d z;

    x[0][0] = 1.f; x[1][0] = 2.f; x[2][0] = 1.f; x[3][0] = 2.f;
    x[0][1] = 0.f; x[1][1] = 3.f; x[2][1] = 4.f; x[3][1] = 2.f;
    x[0][2] = 0.f; x[1][2] = 0.f; x[2][2] = 5.f; x[3][2] = 6.f;
    x[0][3] = 0.f; x[1][3] = 0.f; x[2][3] = 0.f; x[3][3] = 7.f;

    y[0][0] = 1.f; y[1][0] = 0.f; y[2][0] = 0.f; y[3][0] = 0.f;
    y[0][1] = 0.f; y[1][1] = 3.f; y[2][1] = 0.f; y[3][1] = 0.f;
    y[0][2] = 0.f; y[1][2] = 0.f; y[2][2] = 5.f; y[3][2] = 0.f;
    y[0][3] = 0.f; y[1][3] = 0.f; y[2][3] = 0.f; y[3][3] = 7.f;

    std::cout << x * y; // OK!
}
