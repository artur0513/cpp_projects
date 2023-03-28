#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include "Mesh.h"
#include "Shader.h"

void error_callback(int error, const char* description)
{
    std::cout << "error from glfw: ";
    fprintf(stderr, "Error: %s\n", description);
}

void checkGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << err;
    }
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

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        std::cout << "error from glew: ";
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    
    // Оно на удивление работает, пишем дальше загрузчик моделей
    OBJMesh m;
    m.loadFromFile("forTests/2.obj");
    m.printInfo();
    auto vao = m.passToGPU();

    Shader shader;
    shader.loadFromFile("forTests/vertex.txt", "forTests/fragment.txt");

    Texture texture;
    texture.loadFromFile("forTests/test.tga");

    texture.bind(GL_TEXTURE3);
    shader.use();
    shader.setUniform("texture1", 3); // SUKA, сюда надо ставить не идентификатор текстуры, а эту хрень 
    //The uniform value for a sampler refers to the texture unit, not the texture id.
    //shader.setUniform("texture1", i) ===> texture.bind(GL_TEXTUREi);
    // Но при этом если shader.setUniform("texture1", 0) 0 ставить, то биндить можно вообще как угодно?

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        texture.bind(GL_TEXTURE3);

        shader.use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

