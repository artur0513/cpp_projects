#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include "Mesh.h"
#include "Shader.h"
#include <chrono>

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    OBJ::Mesh m;
    m.loadFromFile("forTests/stol.obj");

    //m.meshParts.size();

    ogl::Shader* shader = ogl::ShaderManager::getInstance()->getShader("forTests/vertex.txt", "forTests/fragment.txt");
    ogl::Texture texture;
    texture.loadFromFile("forTests\\bricks.tga");
    texture.generateMipMap();

    ogl::Cubemap st;
    //std::string st_names[6] = { "forTests\\cubemap\\_#1.dds", "forTests\\cubemap\\_#2.dds",
    //"forTests\\cubemap\\_#3.dds" , "forTests\\cubemap\\_#4.dds" , "forTests\\cubemap\\_#5.dds" , "forTests\\cubemap\\_#6.dds" };

    std::string st_names[6] = { "forTests\\cubemap\\_#1.dds", "forTests\\cubemap\\_#2.dds",
    "forTests\\cubemap\\_#3.dds" , "forTests\\cubemap\\_#4.dds" , "forTests\\cubemap\\_#5.dds" , "forTests\\cubemap\\_#6.dds" };
    st.loadFromFile(st_names);
    //st.generateMipMap();

    m3d::PersProjInfo info(3.141f/2.f, 4.f/3.f, 0.1, 30.0);
    
    shader->use();
    shader->setUniform("texture1", texture);
    shader->setUniform("skybox", st);
    //shader.setUniform("texture2", trollface);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //auto x = createSkyboxVAO();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    checkGLError();
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = float(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count()) / 1000.f;

        m3d::mat4f matrix = m3d::mat4f().init_perspective(info) * m3d::mat4f().init_transfer(0, 0, 1.7) * m3d::mat4f().init_rotation_Y(0.5) * m3d::mat4f().init_rotation_X(-1.2);

        shader->use();
        shader->setUniform("matrix", matrix);

        glBindVertexArray(m.vdh.VAO);
        //glDrawElements(GL_TRIANGLES, 3 , GL_UNSIGNED_INT, (void*)3); Все верно, это рисует только один треугольник
        for (auto& mpart : m.meshParts) {
            //shader->setUniform(*mpart.material);
            shader->setUniform("map_Kd", texture);
            st.setSmooth(std::sin(time) > 0);
            shader->bindTextures();
            glDrawElements(GL_TRIANGLES, mpart.numOfIndices, GL_UNSIGNED_INT, (void*)mpart.firstIndex);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    shader->printInfo();
}

