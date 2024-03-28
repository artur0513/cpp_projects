#include "Font.h"
#include "Framebuffer.h"
#include "Syslog.h"
#include <cctype>
#include <fstream>
#include <algorithm>

namespace {
    std::string vertexShaderText =
    "#version 330 core \n "
    "layout (location = 0) in vec2 unitSquare; \n "
    "uniform vec2 pos; \n "
    "uniform vec2 size; \n "
    "void main() { gl_Position = vec4((unitSquare + vec2(1.0)) * size + pos * 2.0 - vec2(1.0), 0.0, 1.0); } \n ";

    std::string copyFragmentShaderText =
    "#version 330 core \n "
    "out vec4 FragColor; \n "
    "uniform sampler2D textureForCopy; \n "
    "uniform vec2 framebufferSize; \n "
    "uniform vec2 pos; \n "
    "uniform vec2 size; \n "
    "uniform vec4 color; \n "
    "void main() { FragColor = texture(textureForCopy, (gl_FragCoord.xy/framebufferSize - pos)/size) * color; } \n ";

    std::string fontFragmentShaderText =
    "#version 330 core \n "
    "out vec4 FragColor; \n "
    "uniform sampler2D glyphs; \n "
    "uniform vec2 glyphPos; \n "
    "uniform vec2 glyphSize; \n "
    "uniform vec2 pos; \n "
    "uniform vec2 size; \n "
    "uniform vec2 framebufferSize; \n "
    "void main() { vec2 temp = (gl_FragCoord.xy/framebufferSize - pos)/size; \n "
    "FragColor = texture(glyphs, vec2(temp.x, 1.0 - temp.y) * glyphSize + glyphPos); } \n ";
}

ogl::Font::Font(const std::string _imageName, const std::string _iniName) {
    imageName = _imageName;
    iniName = _iniName;

    if (!texture.loadFromFile(imageName))
        Syslog::log("Failed to load font image: " + imageName, Syslog::ERROR);
    auto size = texture.getSize();
    float x(size.x), y(size.y);

    std::ifstream file(iniName);
    if (!file.is_open())
        Syslog::log("Failed to open font ini: " + iniName, Syslog::ERROR);

    std::string line;
    int x1, x2, y1, y2;
    while(getline(file, line)) {
        std::string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
        line.erase(end_pos, line.end());

        auto pos = line.find('=');
        if (pos == std::string::npos || !std::isdigit(line[0]))
            continue;

        size_t symbolId = std::stoi(line.substr(0, pos));
        line.erase(0, pos+1);

        x1 = std::stoi(line.substr(0, (pos = line.find(','))));
        symbols[symbolId].posFloat.x = float(x1)/x;
        line.erase(0, pos+1);

        y1 = std::stoi(line.substr(0, (pos = line.find(','))));
        symbols[symbolId].posFloat.y = float(y1)/y; // To glsl coord system ?
        line.erase(0, pos+1);

        x2 = std::stof(line.substr(0, (pos = line.find(','))));
        symbols[symbolId].sizeFloat.x = float(x2)/x - symbols[symbolId].posFloat.x;
        line.erase(0, pos+1);

        y2 = std::stoi(line);
        symbols[symbolId].sizeFloat.y = float(y2)/y - symbols[symbolId].posFloat.y;

        symbols[symbolId].sizeInt.x = x2 - x1;
        symbols[symbolId].sizeInt.y = y2 - y1;

        if (symbols[symbolId].sizeInt.x != 0 && symbols[symbolId].sizeInt.y != 0){
            symbols[symbolId].nonZeroSize = true;
            if (!height)
                height = symbols[symbolId].sizeInt.y;
        }
    }
    Syslog::log("Font loaded from: " + _imageName + " and " + _iniName, Syslog::SUCSSES);
}


m3d::vec2<uint32_t> ogl::Font::getTextureSize() const noexcept { return texture.getSize(); }
m3d::vec2f ogl::Font::getSymbolSizeFloat(char c) noexcept { return symbols[c].sizeFloat; }
m3d::vec2i ogl::Font::getSymbolSizeInt(char c) const noexcept { return symbols[c].sizeInt; }
m3d::vec2f ogl::Font::getSymbolPosition(char c) const noexcept { return symbols[c].posFloat; }
int ogl::Font::getSymbolHeight() const noexcept { return height; }
ogl::Texture* ogl::Font::getGlyphsTexture() noexcept { return &texture; }

// ================== Text ==================

GLuint VAO, VBO, EBO; // One VAO for all text rendering
m3d::vec2i screenSize;
ogl::Shader fontShader, copyShader;

ogl::Text::Text() {
    static bool createVAO = []() { // call only once to create VAO
        // some very shitty code, dont want to think
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        float vertices[] = {-1.0, 1.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0};
        GLushort indices[] = {0, 1, 2, 0, 2, 3};

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        GLint dims[4] = {0}; // BE CAREFUL IF RESIZING WINDOW
        glGetIntegerv(GL_VIEWPORT, dims);
        screenSize = m3d::vec2i(dims[2], dims[3]);

        fontShader.loadFromString(vertexShaderText, fontFragmentShaderText);
        copyShader.loadFromString(vertexShaderText, copyFragmentShaderText);

        return true;
    } ();

}

void ogl::Text::setFont(ogl::Font& _font) noexcept {
    font = &_font;
    settingsChanged = true;
}

void ogl::Text::setSmooth(bool _isSmooth) noexcept {
    if (buff) // nullptr check
        buff->getColorAttachment()->setSmooth(_isSmooth);
    isSmooth = _isSmooth;
}

void ogl::Text::setColor(m3d::vec4f _color) noexcept {
    color = _color;
    settingsChanged = true;
}

void ogl::Text::setPosition(m3d::vec2f _pos) noexcept {
    pos = _pos;
    settingsChanged = true;
}

void ogl::Text::setPosition(float _posx, float _posy) noexcept {
    pos = m3d::vec2f(_posx, _posy);
    settingsChanged = true;
}

void ogl::Text::move(m3d::vec2f _delta) noexcept {
    pos += _delta;
    settingsChanged = true;
}

void ogl::Text::move(float _deltax, float _deltay) noexcept {
    pos += m3d::vec2f(_deltax, _deltay);
    settingsChanged = true;
}

void ogl::Text::setScale(float _scale) noexcept {
    scale.x = _scale;
    scale.y = _scale;
    scaleChanged = true;
    sizeDefinedBy = 0;
}

void ogl::Text::setScale(m3d::vec2f _scale) noexcept {
    scale = _scale;
    scaleChanged = true;
    sizeDefinedBy = 0;
}

void ogl::Text::setHeight(float height) noexcept {
    size.y = height;
    scaleChanged = true;
    sizeDefinedBy = 1;
}

void ogl::Text::setSize(m3d::vec2f _size) noexcept {
    size = _size;
    scaleChanged = true;
    sizeDefinedBy = 2;
}

void ogl::Text::setText(std::string _text) noexcept {
    text = _text;
    settingsChanged = true;
};

void ogl::Text::render() {
    if (!settingsChanged || !font || !text.size())
        return;

    int textSize = 0;
    for (char c : text)
        textSize += font->getSymbolSizeInt(c).x;

    bool bufferTooSmall = false;
    if (buff)
        bufferTooSmall = textSize > buff->getSize().x;

    if (bufferTooSmall || fontChanged || !buff) {
        delete buff;
        buff = new Framebuffer(std::min(textSize * 5 / 4, screenSize.x), font->getSymbolHeight());
        buff->createColorAttachment()->setSmooth(isSmooth);
    }

    // render text to framebuffer here
    ogl::Framebuffer::saveBindings();

    buff->bind();
    glClearColor(color.x, color.y, color.z, 0.0);
    buff->clear();
    glBindVertexArray(VAO);
    fontShader.use();
    fontShader.setUniform("glyphs", font->getGlyphsTexture());
    fontShader.setUniform("framebufferSize", m3d::vec2f(buff->getSize()));
    int pixelCounter = 0;
    for (char c : text) {
        fontShader.setUniform("size", m3d::vec2f(float(font->getSymbolSizeInt(c).x)/float(buff->getSize().x), 1.0));
        fontShader.setUniform("glyphPos", font->getSymbolPosition(c));
        fontShader.setUniform("glyphSize", font->getSymbolSizeFloat(c));
        fontShader.setUniform("pos", m3d::vec2f(float(pixelCounter)/float(buff->getSize().x), 0.0));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        pixelCounter += font->getSymbolSizeInt(c).x;
    }

    ogl::Framebuffer::restoreBindings();
    settingsChanged = false;
}

void ogl::Text::copyToBinded() noexcept {
    render();
    m3d::vec2f buffSize = Framebuffer::getBindedForDraw()->getSize();
    glBindVertexArray(VAO);

    if (scaleChanged) {
        if (sizeDefinedBy == 2)
            scale = m3d::vec2f(size.x / buffSize.x, size.y / buffSize.y);
        if (sizeDefinedBy == 1)
            scale = m3d::vec2f(size.y / buffSize.y, size.y / buffSize.y);
        scaleChanged = false;
    }

    copyShader.use();
    copyShader.setUniform("textureForCopy", buff->getColorAttachment());
    copyShader.setUniform("framebufferSize", buffSize);
    copyShader.setUniform("pos", pos);
    copyShader.setUniform("size", m3d::vec2f(float(buff->getSize().x)/buffSize.x * scale.x, float(buff->getSize().y)/buffSize.y * scale.y));
    copyShader.setUniform("color", color);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

ogl::Framebuffer* ogl::Text::getFramebuffer() noexcept {
    render();
    return buff;
}

ogl::Font* ogl::Text::getFont() const noexcept { return font; };
m3d::vec4f ogl::Text::getColor() const noexcept { return color; };
m3d::vec2f ogl::Text::getPosition() const noexcept { return pos; };
m3d::vec2f ogl::Text::getScale() const noexcept { return scale; };
std::string ogl::Text::getText() const noexcept { return text; };
bool ogl::Text::getSmooth() const noexcept { return isSmooth; };

ogl::Text::~Text() {
    delete buff;
}
