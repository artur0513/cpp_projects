#include "Framebuffer.h"
#include "Syslog.h"

ogl::Framebuffer *ogl::Framebuffer::bindedForRead = nullptr;
ogl::Framebuffer *ogl::Framebuffer::bindedForDraw = nullptr;
ogl::Framebuffer *ogl::Framebuffer::savedForRead = nullptr;
ogl::Framebuffer *ogl::Framebuffer::savedForDraw = nullptr;

ogl::Framebuffer::Framebuffer(GLuint _sizeX, GLuint _sizeY, GLenum _type) : colorAttachments(maxColorAttachments, nullptr) {
    glGenFramebuffers(1, &fbo);
    sizeX = _sizeX;
    sizeY = _sizeY;
    type = _type;
}

void ogl::Framebuffer::bind() noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, sizeX, sizeY);
    bindedForDraw = this;
    bindedForRead = this;
}

void ogl::Framebuffer::bindForDraw() noexcept {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, sizeX, sizeY);
    bindedForDraw = this;
}

void ogl::Framebuffer::bindForRead() noexcept {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    bindedForRead = this;
}

void ogl::Framebuffer::clear() noexcept { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

bool ogl::Framebuffer::checkErrors() noexcept {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Syslog::log("Framebuffer error: " + std::to_string(status), Syslog::ERROR);
        return false;
    }
    return true;
}

ogl::Sampler* ogl::Framebuffer::createSampler(GLenum type){
    if (type == GL_TEXTURE_2D)
        return new ogl::Texture();
    else if (type == GL_TEXTURE_CUBE_MAP)
        return new ogl::Cubemap();
    return nullptr;
}

ogl::Sampler* ogl::Framebuffer::createColorAttachment(size_t attachment, GLenum format) noexcept {
    if (attachment >= maxColorAttachments)
        return nullptr;

    if (colorAttachments[attachment] != nullptr)
        return colorAttachments[attachment];

    colorAttachments[attachment] = createSampler(type);
    colorAttachments[attachment]->createColorAttachment(sizeX, sizeY, format);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, type, colorAttachments[attachment]->getId(), 0);
    checkErrors();
    return colorAttachments[attachment];
}

ogl::Sampler* ogl::Framebuffer::createDepthAttachment() noexcept {
    if (depthAttachment != nullptr)
        return depthAttachment;

    depthAttachment = createSampler(type);
    depthAttachment->createDepthAttachment(sizeX, sizeY);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, depthAttachment->getId(), 0);
    checkErrors();
    return depthAttachment;
}

ogl::Sampler* ogl::Framebuffer::getColorAttachment(size_t attachment) noexcept { return colorAttachments[attachment]; }
ogl::Sampler* ogl::Framebuffer::getDepthAttachment() noexcept { return depthAttachment; }
const m3d::vec2<uint32_t> ogl::Framebuffer::getSize() const noexcept { return m3d::vec2<uint32_t>(sizeX, sizeY); }
const GLenum ogl::Framebuffer::getType() const noexcept { return type; }

void ogl::Framebuffer::blitTo(Framebuffer& buffer) { // add coordinates support
    ogl::Framebuffer* read = bindedForRead;
    ogl::Framebuffer* draw = bindedForDraw;

    buffer.bindForDraw();
    bindForRead();
    glBlitFramebuffer( // transfer from binded read framebuffer to binded draw framebuffer
        0, 0, sizeX, sizeY,
        //0, 0, buffer.sizeX, buffer.sizeY,
        0, 0, sizeX, sizeY,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    if (read != nullptr)
        read->bindForRead();
    if (draw != nullptr)
        draw->bindForDraw(); // Вернули все как было
}

ogl::Framebuffer* ogl::Framebuffer::getBindedForRead() {
    return bindedForRead;
}

ogl::Framebuffer* ogl::Framebuffer::getBindedForDraw() {
    return bindedForDraw;
}

void ogl::Framebuffer::saveBindings() {
    savedForDraw = bindedForRead;
    savedForRead = bindedForRead;
}

void ogl::Framebuffer::restoreBindings() {
    if (savedForDraw != nullptr)
        savedForDraw->bindForDraw();
     if (savedForRead != nullptr)
        savedForRead->bindForRead();
}

ogl::Framebuffer::~Framebuffer(){
    //vector will automatically call destructors for color attachments
    delete depthAttachment;
    delete stencilAttachment;
    glDeleteFramebuffers(1, &fbo);
}
