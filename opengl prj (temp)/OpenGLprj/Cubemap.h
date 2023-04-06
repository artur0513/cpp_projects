#pragma once
#include "Texture.h"

class Cubemap {
protected:
    GLuint id = 0;

    bool t_hasMipMap = false, t_isSmooth = true;
    std::string name[6];
    m3d::vec2<uint32_t> size[6];

    Cubemap(const Cubemap& t) = delete;
    Cubemap operator=(const Cubemap& t) = delete;
public:
    Cubemap();
    Cubemap(Cubemap&& c) noexcept;
    Cubemap& operator=(Cubemap&& t) noexcept;

    [[nodiscard]] GLuint loadFromFile(std::string* filenames);
    void bind(GLenum texture = GL_TEXTURE0);
    void generateMipMap();
    void setSmooth(bool smooth);

    const GLuint getId();
    const m3d::vec2<uint32_t> getSize();
    const std::string& getName();
    const bool hasMipMap();
    const bool isSmooth();

    ~Cubemap();
};

GLuint createCubemapVAO();