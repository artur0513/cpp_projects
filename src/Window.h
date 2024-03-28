#pragma once
#include "Framebuffer.h"
#include "Font.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace ogl::Window {

    bool init() noexcept;

    void refresh() noexcept;

    bool isOpen() noexcept;

    void close() noexcept;

    void setVsync(bool _vsync) noexcept;

    bool getVsync() noexcept;

    ogl::Framebuffer* getFramebuffer() noexcept;
}
