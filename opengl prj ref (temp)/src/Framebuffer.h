#pragma once
#include "Texture.h"

namespace ogl {

    // Also remember about GL_DEPTH_STENCIL_ATTACHMENT

    class Framebuffer {
    private:
        GLuint fbo;
        GLuint sizeX = 0, sizeY = 0;

        GLenum type = GL_TEXTURE_2D;

        const size_t maxColorAttachments = 8;
        std::vector<ogl::Sampler*> colorAttachments;
        ogl::Sampler *depthAttachment = nullptr,
                     *stencilAttachment = nullptr; // depth&stencil attachment?


        static ogl::Framebuffer* bindedForRead;
        static ogl::Framebuffer* bindedForDraw;

        static ogl::Framebuffer* savedForRead;
        static ogl::Framebuffer* savedForDraw;

        ogl::Sampler *createSampler(GLenum type);
    public:
        Framebuffer(GLuint _sizeX, GLuint _sizeY, GLenum _type = GL_TEXTURE_2D);
        bool checkErrors() noexcept;

        void bind() noexcept;
        void bindForDraw() noexcept;
        void bindForRead() noexcept;
        void clear() noexcept;
        void blitTo(Framebuffer& buffer);

        ogl::Sampler* createColorAttachment(size_t attachment = 0, GLenum format = GL_SRGB8_ALPHA8) noexcept;
        ogl::Sampler* createDepthAttachment() noexcept;

        ogl::Sampler* getColorAttachment(size_t attachment = 0) noexcept;
        ogl::Sampler* getDepthAttachment() noexcept;
        const m3d::vec2<uint32_t> getSize() const noexcept;
        const GLenum getType() const noexcept;

        static ogl::Framebuffer* getBindedForRead();
        static ogl::Framebuffer* getBindedForDraw();
        static void saveBindings();
        static void restoreBindings();

        ~Framebuffer();
    };

}
