#pragma once
#include "Framebuffer.h"
#include "Texture.h"
#include "Shader.h"

namespace ogl {

    class Font {
    private:
        Texture texture;
        std::string imageName, iniName;
        int height = 0;

        struct Symbol {
            bool nonZeroSize = false;
            m3d::vec2i sizeInt;
            m3d::vec2f posFloat, sizeFloat;
        } symbols[256];

    public:
        Font(const std::string _imageName, const std::string _iniName);

        m3d::vec2<uint32_t> getTextureSize() const noexcept;
        m3d::vec2f getSymbolSizeFloat(char c) noexcept;
        m3d::vec2i getSymbolSizeInt(char c) const noexcept;
        m3d::vec2f getSymbolPosition(char c) const noexcept;
        int getSymbolHeight() const noexcept;
        ogl::Texture* getGlyphsTexture() noexcept;
    };

    class Text {
    private:
        Font* font = nullptr;
        bool fontChanged = false;

        Framebuffer* buff = nullptr;

        std::string text;
        m3d::vec2f pos, scale = m3d::vec2f(1.f, 1.f);
        m3d::vec4f color;
        bool isSmooth = true;

        bool settingsChanged = false;

        m3d::vec2f size;
        char sizeDefinedBy = 0; // 0 - scale, 1 - height, 2 - height and width
        bool scaleChanged = false;
    public:
        Text();

        void render();
        void setFont(ogl::Font& _font) noexcept;
        void setColor(m3d::vec4f _color) noexcept;
        void setPosition(m3d::vec2f _pos) noexcept;
        void setScale(float _scale) noexcept;
        void setScale(m3d::vec2f _scale) noexcept;
        void setHeight(float height) noexcept;
        void setSize(m3d::vec2f _size) noexcept;
        void setText(std::string _text) noexcept;
        void setSmooth(bool _isSmooth) noexcept;

        ogl::Font* getFont() const noexcept;
        m3d::vec4f getColor() const noexcept;
        m3d::vec2f getPosition() const noexcept;
        m3d::vec2f getScale() const noexcept;
        std::string getText() const noexcept;
        bool getSmooth() const noexcept;

        void copyToBinded() noexcept;
        ogl::Framebuffer* getFramebuffer() noexcept;

        ~Text();
    };

}
