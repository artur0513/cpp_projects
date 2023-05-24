﻿#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include "Mesh.h"
#include "Shader.h"
#include <chrono>
#include "Skybox.h"
#include "Camera.h"

#include "windows.h"
#include "psapi.h"

int wx = 1280, wy = 720;
m3d::PersProjInfo info(1.3f, float(wx) / float(wy), 0.1, 30.0);
ogl::Camera mainCamera(info);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    mainCamera.keyboardMove(key, action);

    // Print memory usage
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
        SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
        std::cout << "virtual memoty usage (mb): " << double(virtualMemUsedByMe) / (1024.0 * 1024.0) << "\n";
        std::cout << "phys memoty usage (mb): " << double(physMemUsedByMe) / (1024.0 * 1024.0) << "\n";
    }
}

double prevMouseX = 0.0, prevMouseY = 0.0, wxd = 0.0, wyd = 0.0;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mainCamera.mouseMove((xpos - prevMouseX)/800.0, (ypos - prevMouseY)/800.0);

    prevMouseX = xpos;
    prevMouseY = ypos;
}

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    
    GLFWwindow* window = glfwCreateWindow(wx, wy, "My Title", NULL, NULL);
    if (!window) {
        std::cout << "error creating window \n";
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // thats WAY better than GLFW_CURSOR_HIDDEN
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
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

    ogl::Texture texture;
    texture.loadFromFile("forTests\\bricks.tga");
    texture.generateMipMap();

    ogl::Cubemap st;
    std::string st_names[6] = { "forTests\\cubemap\\_#1.dds", "forTests\\cubemap\\_#2.dds",
    "forTests\\cubemap\\_#3.dds" , "forTests\\cubemap\\_#4.dds" , "forTests\\cubemap\\_#5.dds" , "forTests\\cubemap\\_#6.dds" };

    std::string skybox1472[6] = { "forTests\\skybox1472\\sky_l1escape1_bk.dds", "forTests\\skybox1472\\sky_l1escape1_fr.dds",
    "forTests\\skybox1472\\sky_l1escape1_up.dds" , "forTests\\skybox1472\\sky_l1escape1_down.dds" , "forTests\\skybox1472\\sky_l1escape1_lf.dds" , "forTests\\skybox1472\\sky_l1escape1_rt.dds" };
    st.loadFromFile(skybox1472);
    //st.generateMipMap();

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

    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDepthFunc(GL_LEQUAL);

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    GLuint color, depth, fbo;

    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, wx, wy);

    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, wx, wy);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "glCheckFramebufferStatus: %x\n", status);
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);

    std::cout << m.meshParts.size() << " mesh parts size\n";
    for (auto& mpart : m.meshParts) {
        std::cout << mpart.firstIndex << " - " << mpart.firstIndex + mpart.numOfIndices << "\n";
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point fpsClockStart = std::chrono::steady_clock::now();
    unsigned frameCounter = 0;

    glfwSwapInterval(1);//Код чтобы убрать/добавить ограничение на 60 фпс
    //glDisable(GL_CULL_FACE);
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); //  | GL_DEPTH_BUFFER_BIT

        glClearDepth(0.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        float time = float(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count()) / 1000.f;

        // * m3d::mat4f().init_rotation(m3d::quatf(3.14f, m3d::vec3f(0.f, 1.f, 0.f)))
        m3d::mat4f matrix = persMat * m3d::mat4f().init_transfer(mainCamera.getPosition());
        //matrix = persMat * m3d::mat4f().init_transfer(0, -0.7, 1.7);

        auto camMat = m3d::mat4f().init_perspective_reversed(info) * m3d::mat4f().init_camera_transform(mainCamera.getDirection());
        ogl::Skybox::setCameraMatrix(camMat);
        ogl::Skybox::renderSkybox();

        shader.use();
        shader.setUniform("matrix", m3d::mat4f().init_perspective_reversed(info) * mainCamera.getCameraTransform() * m3d::mat4f().init_transfer(0, -0.7, 1.7));
        glBindVertexArray(m.vdh.VAO);
        //glDrawElements(GL_TRIANGLES, 3 , GL_UNSIGNED_INT, (void*)3); Все верно, это рисует только один треугольник
        for (auto& mpart : m.meshParts) {
            shader.setUniform(mpart.material);
            //shader.setUniform("map_Kd", texture);
            //st.setSmooth(std::sin(time) > 0);
            shader.bindTextures();
            glDrawElements(GL_TRIANGLES, mpart.numOfIndices, GL_UNSIGNED_INT, (void*)(mpart.firstIndex*sizeof(unsigned)));
        }
        
        
        mainCamera.update();
        checkGLError();

        glfwSwapBuffers(window);
        glfwPollEvents();

        int frameMax = 200;
        if (frameCounter == frameMax) {
            float fps = 1000.f * float(frameMax) / float(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - fpsClockStart).count());
            glfwSetWindowTitle(window, std::to_string(fps).c_str());
            fpsClockStart = std::chrono::steady_clock::now();
            frameCounter = 0;

        }
        frameCounter++;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // default FBO
        glBlitFramebuffer(
            0, 0, wx, wy,
            0, 0, wx, wy,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    shader.printInfo();
}

