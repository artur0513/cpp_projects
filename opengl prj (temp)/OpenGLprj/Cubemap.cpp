#include "Cubemap.h"
#include <iostream>

// =============== Cubemap ===============

ogl::Cubemap::Cubemap() {}

GLuint ogl::Cubemap::loadFromFile(std::string* filenames) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    t_hasMipMap = true;
    GLuint compressionType;
    ogl::Image* img;
    for (int i = 0; i < 6; i++) {
        img = ogl::loadImage(filenames[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        if (img == nullptr)
            return 0;

        name[i] = filenames[i];

        t_hasMipMap &= bool(img->getMipMapCount() - 1);
        if (i == 0) {
            compressionType = img->getCompressionType();
            size.x = img->getWidth();
            size.y = img->getHeight();
            if (size.x != size.y) {
                std::cerr << "Error while loading cubemap: " << filenames[i] << " - each side must be a square \n";
                return 0;
            }
        }
        else {
            if (compressionType != img->getCompressionType()) {
                std::cerr << "Error while loading cubemap: " << filenames[i] << " - textures compression types must be same \n";
                return 0;
            }
            if (size.x != img->getWidth() || size.y != img->getHeight()) {
                std::cerr << "Error while loading cubemap: " << filenames[i] << " - each side must have same resolution \n";
                return 0;
            }
        }
        delete img;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if (!t_hasMipMap)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return id;
}

void ogl::Cubemap::generateMipMap() {
    if (!t_hasMipMap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        glGenerateTextureMipmap(id);
        if (t_isSmooth)
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        else
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        t_hasMipMap = true;
    }
}

void ogl::Cubemap::setSmooth(bool smooth) { // works fine
    if (smooth == t_isSmooth)
        return;

    t_isSmooth = smooth;
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);

    if (t_hasMipMap)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);
}

void ogl::Cubemap::bind(GLenum texture) {
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

const GLuint ogl::Cubemap::getId() { return id; }
const bool ogl::Cubemap::hasMipMap() { return t_hasMipMap; }
const bool ogl::Cubemap::isSmooth() { return t_isSmooth; }
const m3d::vec2<uint32_t> ogl::Cubemap::getSize() { return size; }
const std::string& ogl::Cubemap::getName(unsigned i) { return name[i < 6 ? i : 0]; }
ogl::Cubemap::~Cubemap() { glDeleteTextures(1, &id); }