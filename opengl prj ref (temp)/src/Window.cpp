#include "Window.h"
#include <cstdlib>
#include "Syslog.h"
#include "Framebuffer.h"

using namespace std::chrono_literals;

namespace ogl::Window {
    /* private section */
    namespace {
        m3d::vec2i windowSize;
        GLFWwindow* window;
        bool windowOpen = false;
        bool vsync = true;

        void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                close();
            }
        }

        void errorCallback(int error, const char* description)
        {
            Syslog::log("Error from glfw: " + std::string(description), Syslog::ERROR);
        }

    }

    /* Create window */
    Framebuffer *buffer;
    bool init() noexcept {
        if (!glfwInit()){
            Syslog::log("glfwInit fail!", Syslog::FATAL);
            return 0;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window = glfwCreateWindow(mode->width, mode->height, "My Title", monitor, NULL);
        windowSize = m3d::vec2i(mode->width, mode->height);
        if (!window) {
            Syslog::log("Window creation fail!", Syslog::FATAL);
            return 0;
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetErrorCallback(errorCallback);
        glfwMakeContextCurrent(window);
        glfwSwapInterval((int) vsync);

        GLenum err = glewInit();
        if (err != GLEW_OK){
            Syslog::log("glewInit fail! Error: " + std::string((char*)glewGetErrorString(err)), Syslog::FATAL);
            return 0;
        }

        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
        buffer = new Framebuffer(mode->width, mode->height);
        buffer->createColorAttachment();//GL_RGB10
        buffer->createDepthAttachment();
        buffer->bind();

        windowOpen = true;
        std::atexit([](){glfwTerminate();});

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(0.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Syslog::log("Window creation sucssesful", Syslog::SUCSSES);
        return 1;
    }

    bool isOpen() noexcept {
        return windowOpen;
    }

    void close() noexcept {
        delete buffer;
        if (windowOpen)
            glfwSetWindowShouldClose(window, true);
        windowOpen = false;
        Syslog::log("WindowOpen set to false, window will be closed at program exit", Syslog::INFO);
    }

    void refresh() noexcept {
        buffer->bindForDraw();
        // End of performance part


        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // THIS SHOULD BE THE ONLY TIME WE BIND DEFAULT FRAMEBUFFER
        buffer->bindForRead();

        glBlitFramebuffer( // transfer from binded read framebuffer to binded draw framebuffer
            0, 0, buffer->getSize().x, buffer->getSize().y,
            0, 0, windowSize.x, windowSize.y,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);

        buffer->bind();

        glfwSwapBuffers(window); /* We dont check if window is open/closed here */
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void setVsync(bool _vsync) noexcept {
        vsync = _vsync;
        glfwSwapInterval((int) vsync);
    }

    bool getVsync() noexcept {
        return vsync;
    }

    ogl::Framebuffer* getFramebuffer() noexcept {
        return buffer;
    }

}
