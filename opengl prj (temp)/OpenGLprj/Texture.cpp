﻿#include "Texture.h"
#include <filesystem>

// =============== TGAImage ===============

uint32_t ogl::TGAImage::getWidth(int mipMapLevel) { return mipMaps[0].width; }
uint32_t ogl::TGAImage::getHeight(int mipMapLevel) { return mipMaps[0].height; }
uint32_t ogl::TGAImage::getSize(int mipMapLevel) { return mipMaps[0].size; }
bool ogl::TGAImage::isRGBA() { return bytesPerPixel == 4; }
GLuint ogl::TGAImage::getCompressionType() { return 0; }
uint32_t ogl::TGAImage::getMipMapCount() { return 1; }
const uint8_t* ogl::TGAImage::getData(int mipMapLevel) { return mipMaps[0].pixels.data(); }

bool ogl::TGAImage::loadFromFile(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Can`t open file and load image: " << filename << "\n";
        file.close();
        return false;
    }

    mipMaps.resize(1);
    mipMapCount = 1;

    uint8_t header[18] = { 0 };
    file.read(reinterpret_cast<char*>(&header), 18);
    mipMaps[0].width = header[13] * 256 + header[12];
    mipMaps[0].height = header[15] * 256 + header[14];
    bitsPerPixel = header[16];
    bytesPerPixel = bitsPerPixel / 8;
    mipMaps[0].size = mipMaps[0].width * mipMaps[0].height * bytesPerPixel;

    if (bitsPerPixel != 24 && bitsPerPixel != 32) {
        std::cerr << "Unknown file format, required 24 or 32 bits per pixel: " << filename << "\n";
        file.close();
        return false;
    }

    TGA_Pixel pixel;
    auto save_pixel = [&](uint32_t& pixel_count) { // lambda for automaticly saving pixel to vector, and swapping BGRA to RGBA 
        uint32_t id = pixel_count * bytesPerPixel;
        mipMaps[0].pixels[id] = pixel.R;
        mipMaps[0].pixels[id + 1] = pixel.G;
        mipMaps[0].pixels[id + 2] = pixel.B;
        if (bytesPerPixel == 4)
            mipMaps[0].pixels[id + 3] = pixel.A;
    };

    if (header[2] == 2) { //uncomressed image
        mipMaps[0].pixels.resize(mipMaps[0].size);
        for (uint32_t i = 0; i < mipMaps[0].width * mipMaps[0].height; i++) {
            file.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);
            save_pixel(i);
        }

    }
    else if (header[2] == 10) { //encoded image
        mipMaps[0].pixels.resize(mipMaps[0].size);
        uint32_t current_pixel = 0;
        uint8_t block_header;

        while (current_pixel < mipMaps[0].width * mipMaps[0].height) {
            file.read(reinterpret_cast<char*>(&block_header), 1);
            if (block_header < 128) {
                // Next (block_header + 1) pixels are uncompressed, just read them as usual
                for (uint32_t i = current_pixel; i < uint32_t(block_header + 1) + current_pixel; i++) {
                    file.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);
                    save_pixel(i);
                }

                current_pixel += block_header + 1;
            }
            else {
                // We read next pixel, and this pixel will be repeated for (block_header - 127) times
                file.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);
                for (uint8_t i = 0; i < block_header - 127; i++) {
                    save_pixel(current_pixel);
                    current_pixel++;
                }
            }
        }
    }
    else {
        std::cerr << "This TGA type is unsupported. Supported types are uncompressed or encoded true-color image: " << filename << "\n";
        file.close();
        return false;
    }

    if (file.eof() || file.fail() || file.bad()) {
        std::cerr << "Fstream error while loading texture: " << filename << "\n";
        file.close();
        return false;
    }

    file.close();
    return true;
}

// =============== DDSImage ===============

uint32_t ogl::DDSImage::getWidth(int mipMapLevel) { return mipMaps[mipMapLevel].width; }
uint32_t ogl::DDSImage::getHeight(int mipMapLevel) { return mipMaps[mipMapLevel].height; }
uint32_t ogl::DDSImage::getSize(int mipMapLevel) { return mipMaps[mipMapLevel].size; }
uint32_t ogl::DDSImage::getMipMapCount() { return mipMapCount; }
GLuint ogl::DDSImage::getCompressionType() { return compressionType; }
const uint8_t* ogl::DDSImage::getData(int mipMapLevel) { return mipMaps[mipMapLevel].pixels.data(); }
bool ogl::DDSImage::isRGBA() { return true; }

bool ogl::DDSImage::loadFromFile(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Can`t open file and load image: " << filename << "\n";
        file.close();
        return false;
    }

    uint32_t format;
    file.read(reinterpret_cast<char*>(&format), sizeof(format));
    if (format != 0x20534444) {
        std::cerr << "File format is not DDS: " << filename << "\n";
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    mipMapCount = header.dwMipMapCount;
    mipMaps.resize(mipMapCount);

    std::string type_str(header.ddspf.dwFourCC);
    if (type_str == "DXT1")
        compressionType = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    else if (type_str == "DXT3")
        compressionType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    else if (type_str == "DXT5")
        compressionType = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    else {
        std::cerr << "Unsupported DDS compression type. Supported are DXT1, DXT3, DXT5: " << filename << "\n";
        file.close();
        return false;
    }

    uint32_t blockSize = (compressionType == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    uint32_t iHeight = header.dwHeight, iWidth = header.dwWidth, iSize, sumsize = 0;
    for (uint32_t i = 0; i < mipMapCount; i++) {
        iSize = ((iWidth + 3) / 4) * ((iHeight + 3) / 4) * blockSize;
        sumsize += iSize;
        mipMaps[i].pixels.resize(iSize);
        mipMaps[i].width = iWidth;
        mipMaps[i].height = iHeight;
        mipMaps[i].size = iSize;
        file.read(reinterpret_cast<char*>(mipMaps[i].pixels.data()), iSize);

        iWidth /= 2;
        iHeight /= 2;
    }

    if (file.eof() || file.fail() || file.bad()) {
        std::cerr << "Fstream error while loading texture: " << filename << "\n";
        file.close();
        return false;
    }

    file.close();
    return true;
}

// =============== loadImage function ===============

ogl::Image* ogl::loadImage(std::string filename, GLenum textureType) {
    filename = std::filesystem::relative(filename).string(); // Приводм название файла к единому виду
    std::string ext = filename.substr(filename.rfind(".")); // Берем расширение файла

    Image* img;
    if (ext == ".dds") {
        img = new DDSImage();
        if (!img->loadFromFile(filename)) {
            std::cerr << "Unable to load texture: " << filename << "\n";
            return nullptr;
        }

    }
    else if (ext == ".tga") {
        img = new TGAImage();
        if (!img->loadFromFile(filename)) {
            std::cerr << "Unable to load texture: " << filename << "\n";
            return nullptr;
        }
    }
    else {
        std::cerr << "Unknown texture format. Supported are .dds and .tga: " << filename << "\n";
        return nullptr;
    }

    // Если тип сжатия = 0, т.е. без сжатия то используем одну функцию, иначе другую + грузим все мипмапы
    if (img->getCompressionType() != 0)
        for (uint32_t i = 0; i < img->getMipMapCount(); i++)
            glCompressedTexImage2D(textureType, i, img->getCompressionType(), img->getWidth(i), img->getHeight(i), 0, img->getSize(i), img->getData(i));
    else
        for (uint32_t i = 0; i < img->getMipMapCount(); i++)
            glTexImage2D(textureType, i, img->isRGBA() ? GL_RGBA : GL_RGB, img->getWidth(i), img->getHeight(i), 0, img->isRGBA() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img->getData(i));

    return img;
}

// =============== Texture ===============

ogl::Texture::Texture() {};

ogl::Texture::Texture(Texture&& t) noexcept {
    id = t.id;
    t.id = 0;

    size = t.size;
    name = t.name;
    t_hasMipMap = t.t_hasMipMap;
    t_isSmooth = t.t_isSmooth;

    t.size = { 0, 0 };
    t.name = "";
    t.t_hasMipMap = false;
}

ogl::Texture& ogl::Texture::operator=(ogl::Texture&& t) noexcept {
    id = t.id;
    t.id = 0;

    size = t.size;
    name = t.name;
    t_hasMipMap = t.t_hasMipMap;
    t_isSmooth = t.t_isSmooth;

    t.size = { 0, 0 };
    t.name = "";
    t.t_hasMipMap = false;

    return *this;
}

GLuint ogl::Texture::loadFromFile(std::string filename) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    Image* img = loadImage(filename, GL_TEXTURE_2D);
    if (img == nullptr)
        return id;

    // Установка параметров наложения текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Установка параметров фильтрации текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // assume t_isSmooth = true by deafault
    if (img->getMipMapCount() == 1)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    setFilteringLevel(8.f);

    t_hasMipMap = (img->getMipMapCount() > 0); // Сохраняем необходимые данные
    name = filename;
    size.x = img->getWidth();
    size.y = img->getHeight();

    delete img;
    return id;
}

void ogl::Texture::bind(GLenum texture) {
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_2D, id);
}

float maxAnisotropy;
void ogl::Texture::setFilteringLevel(float _AFlevel) {
    static bool checkMaxAnisotropy = []() { // call only once to check max anisotropy level
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        return true;
    } ();
    
    glBindTexture(GL_TEXTURE_2D, id);
    AFlevel = (_AFlevel > maxAnisotropy) ? maxAnisotropy : _AFlevel;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, AFlevel);
}

void ogl::Texture::generateMipMap() { // works fine
    if (!t_hasMipMap) {
        glBindTexture(GL_TEXTURE_2D, id);
        glGenerateTextureMipmap(id);
        if (t_isSmooth)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        else 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        t_hasMipMap = true;
    }
}

void ogl::Texture::setSmooth(bool smooth) { // works fine
    if (smooth == t_isSmooth)
        return;

    t_isSmooth = smooth;
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);

    if (t_hasMipMap)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);
}

const float ogl::Texture::getFilteringLevel() const { return AFlevel; }
const GLuint ogl::Texture::getId() const { return id; }
const bool ogl::Texture::hasMipMap() const { return t_hasMipMap; }
const bool ogl::Texture::isSmooth() const { return t_isSmooth; }
const m3d::vec2<uint32_t> ogl::Texture::getSize() const { return size; }
const std::string& ogl::Texture::getName() const { return name; }
ogl::Texture::~Texture() { glDeleteTextures(1, &id); }