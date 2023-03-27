#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include "Mesh.h"

void error_callback(int error, const char* description)
{
    std::cout << "error from glfw: ";
    fprintf(stderr, "Error: %s\n", description);
}

int main()
{
    if (!glfwInit()) {
        std::cout << "error glfw init \n";
    }
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window) {
        std::cout << "error creating window \n";
    }

    //glfwMakeContextCurrent(window); //Код чтобы убрать ограничение на 60 фпс
    //glfwSwapInterval(0);

    glfwMakeContextCurrent(window);

    OBJMesh m;
    m.loadFromFile("1.obj");
    m.printInfo();

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        std::cout << "error from glew: ";
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
}
