#include <iostream>
#include "3dMath.h"

int main()
{
    vec3f a = {1.0f, 2.0f, 0.3f}; // WTF как это работает епта?  причем оно работает только если vec3 это struct, а не class
    vec3f b = a;
    std::cout << b.z << std::endl;

    vec2d c = { 1, 2 };
    vec2d d = { 0, 3 };
    std::cout << (c+d).y << std::endl;
    std::cout << length(c) << std::endl;
}
