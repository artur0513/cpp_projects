#pragma once
#include "Texture.h"

namespace ogl {

    class Cubemap {
    protected:
        GLuint id = 0;

        bool t_hasMipMap = false, t_isSmooth = true;
        std::string name[6];
        m3d::vec2<uint32_t> size;

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

        const GLuint getId() const;
        const m3d::vec2<uint32_t> getSize() const;
        const std::string& getName(unsigned i = 0) const;
        const bool hasMipMap() const;
        const bool isSmooth() const;

        ~Cubemap();
    };

}