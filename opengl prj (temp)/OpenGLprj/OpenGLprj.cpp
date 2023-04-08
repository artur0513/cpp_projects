#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include "Mesh.h"
#include "Shader.h"
#include <chrono>
#include "Skybox.h"

// Z AXIS UP !

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

    glfwMakeContextCurrent(window); //Код чтобы убрать ограничение на 60 фпс
    glfwSwapInterval(0);

    glfwMakeContextCurrent(window);

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        std::cout << "error from glew: ";
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    
    // Оно на удивление работает, пишем дальше загрузчик моделей
    OBJ::Mesh m, cube;
    m.loadFromFile("forTests/stol.obj");
    cube.loadFromFile("forTests/cube.obj");

    //m.meshParts.size();

    
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

    ogl::Skybox::initSkybox();
    ogl::Skybox::setSkyboxCubemap(st);
    
    m3d::mat4f persMat;
    persMat.init_perspective(info);
    ogl::Skybox::setCameraMatrix(persMat);

    ogl::Shader shader;
    shader.loadFromFile("forTests/vertex.txt", "forTests/fragment.txt");
    shader.use();
    shader.setUniform("texture1", texture);
    shader.setUniform("skybox", st);
    //shader.setUniform("texture2", trollface);


    ogl::Shader skyboxShader;
    skyboxShader.loadFromFile("forTests/skyboxVertShader.txt", "forTests/skyboxFragShader.txt");
    skyboxShader.use(); 
    skyboxShader.setUniform("skybox", st);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //auto x = createSkyboxVAO();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point fpsClockStart = std::chrono::steady_clock::now();
    unsigned frameCounter = 0;
    
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = float(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count()) / 1000.f;

        m3d::mat4f matrix = persMat * m3d::mat4f().init_transfer(0, 0, 1.7) * m3d::mat4f().init_rotation(m3d::quatf(time, m3d::vec3f(0.f, 0.f, 1.f)));
        matrix = persMat * m3d::mat4f().init_transfer(0, 0, 1.7);

        //shader.use();
        //shader.setUniform("matrix", matrix);
        //glBindVertexArray(m.vdh.VAO);
        //glDrawElements(GL_TRIANGLES, 3 , GL_UNSIGNED_INT, (void*)3); Все верно, это рисует только один треугольник
        //for (auto& mpart : m.meshParts) {
            //shader->setUniform(*mpart.material);
           // shader.setUniform("map_Kd", texture);
           // st.setSmooth(std::sin(time) > 0);
            //shader.bindTextures();
            //glDrawElements(GL_TRIANGLES, mpart.numOfIndices, GL_UNSIGNED_INT, (void*)mpart.firstIndex);
        //}

        glBindVertexArray(cube.vdh.VAO);
        skyboxShader.use();
        skyboxShader.setUniform("matrix", persMat);
        for (auto& mpart : cube.meshParts) {
            skyboxShader.bindTextures();
            glDrawElements(GL_TRIANGLES, mpart.numOfIndices, GL_UNSIGNED_INT, (void*)mpart.firstIndex);
        }

        //ogl::Skybox::renderSkybox();
        //checkGLError();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (frameCounter == 1000) {
            float fps = 1000.f * 1000.f / float(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - fpsClockStart).count());
            glfwSetWindowTitle(window, std::to_string(fps).c_str());
            fpsClockStart = std::chrono::steady_clock::now();
            frameCounter = 0;
        }
        frameCounter++;
    }
    shader.printInfo();
    skyboxShader.printInfo();
}

