#pragma once
#include "Image.h"

namespace ogl {

    class Sampler {
    protected:
        GLuint id = 0;

        float AFlevel = 16.f;
        bool loadOnce = false;
        bool t_hasMipMap = false, t_isSmooth = false, t_isRenderSampler = false; // !НЕ! МЕНЯТЬ t_isSmooth на TRUE!
        std::vector<std::string> name;
        m3d::vec2<uint32_t> size;

        Sampler(const Sampler& t) = delete;
        Sampler operator=(const Sampler& t) = delete;
    public:
        Sampler();
        Sampler(Sampler&& t) noexcept;
        Sampler& operator=(Sampler&& t) noexcept;

        virtual GLenum getSamplerType() const noexcept; // add constexpr maybe?

        void bind(GLenum texture = GL_TEXTURE0) const;
        void generateMipMap();
        void setSmooth(bool smooth);
        void setFilteringLevel(float AFlevel);

        const float getFilteringLevel() const;
        const GLuint getId() const;
        const m3d::vec2<uint32_t> getSize() const; // Ок, потому что все стороны у кубмапы имеют одинаковый размер
        const std::string& getName(size_t i = 0) const;
        const bool hasMipMap() const;
        const bool isSmooth() const;
        const bool isRenderSampler() const;

        GLuint createColorAttachment(GLuint wx, GLuint wy, GLenum format = GL_SRGB8_ALPHA8);
        GLuint createDepthAttachment(GLuint wx, GLuint wy);
        // create stencil attachment

        ~Sampler();
    };

    class Texture : public Sampler {
    public:
        GLenum getSamplerType() const noexcept override;
        GLuint loadFromFile(std::string filename);
    };

    class Cubemap : public Sampler {
    public:
        GLenum getSamplerType() const noexcept override;
        GLuint loadFromFile(std::string filename[]);
    };

}
