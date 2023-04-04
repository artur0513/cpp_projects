#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <glew.h>

class Image {
protected:
    uint32_t bitsPerPixel = 32, bytesPerPixel = 4;
    uint32_t mipMapCount = 0;
    GLuint compressionType;

    struct mipMap {
        uint32_t width, height, size;
        std::vector<uint8_t> pixels;
    };

    std::vector<mipMap> mipMaps;
public:
    virtual uint32_t getWidth(int mipMapLevel = 0) = 0;
    virtual uint32_t getHeight(int mipMapLevel = 0) = 0;
    virtual uint32_t getSize(int mipMapLevel = 0) = 0;
    virtual uint32_t getMipMapCount() = 0;
    virtual GLuint getCompressionType() = 0;
    virtual bool isRGBA() = 0;
    virtual const uint8_t* getData(int mipMapLevel = 0) = 0;

    [[nodiscard]] virtual bool loadFromFile(std::string filename) = 0;
};

class TGAImage : public Image{
private:
    union TGA_Pixel {
        std::uint32_t color;
        struct
        {
            std::uint8_t B, G, R, A;
        };
    };
public:
    uint32_t getWidth(int mipMapLevel = 0) override;
    uint32_t getHeight(int mipMapLevel = 0) override;
    uint32_t getSize(int mipMapLevel = 0) override;
    uint32_t getMipMapCount() override;
    bool isRGBA() override;
    GLuint getCompressionType() override;
    const uint8_t* getData(int mipMapLevel = 0) override;

    [[nodiscard]] bool loadFromFile(std::string filename) override;
};

// Do not forget that DDS texuters will be Y-inverted
class DDSImage : public Image{
private:
    struct DDS_PIXELFORMAT {
        uint32_t dwSize; // DDS_PIXELFORMAT size
        uint32_t dwFlags;
        char dwFourCC[4]; // Compression format. There will be something like DXT1 (4 bytes, each byte is char)
        uint32_t dwRGBBitCount;
        uint32_t dwRBitMask;
        uint32_t dwGBitMask;
        uint32_t dwBBitMask;
        uint32_t dwABitMask;
    };
    struct DDS_HEADER {
        uint32_t           dwSize; // header size in bytes
        uint32_t           dwFlags;
        uint32_t           dwHeight; // just height
        uint32_t           dwWidth; // and width of the image
        uint32_t           dwPitchOrLinearSize; // image size
        uint32_t           dwDepth;
        uint32_t           dwMipMapCount;
        uint32_t           dwReserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t           dwCaps;
        uint32_t           dwCaps2;
        uint32_t           dwCaps3;
        uint32_t           dwCaps4;
        uint32_t           dwReserved2;
    } header; // DDS_HEADER + "DDS " signature in the beginning of file = 128 bytes (looks like this is always true)

public:
    uint32_t getWidth(int mipMapLevel = 0) override;
    uint32_t getHeight(int mipMapLevel = 0) override;
    uint32_t getSize(int mipMapLevel = 0) override;
    uint32_t getMipMapCount() override;
    GLuint getCompressionType() override;
    const uint8_t* getData(int mipMapLevel = 0) override;
    bool isRGBA() override;

    [[nodiscard]] bool loadFromFile(std::string filename) override;
};

class Texture {
protected:
    GLuint id = 0;

    bool hasMipMap = false;
    std::string name;
    uint32_t height = 0, width = 0;

    Texture(const Texture& t) = delete;
    Texture operator=(const Texture& t) = delete;
public:
    Texture();
    Texture(Texture&& t) noexcept;
    Texture& operator=(Texture&& t) noexcept;

    [[nodiscard]] GLuint loadFromFile(std::string filename); // Возвращает id текстуры в случае успеха, либо 0 в случае ошибки
    GLuint getId();
    void bind(GLenum texture = GL_TEXTURE0);
    void generateMipMap();
    const std::string& getName();
    ~Texture();
};

class TextureManager {
private:
    std::unordered_map<std::string, Texture*> textures;

    TextureManager() {};
    TextureManager(const TextureManager& r) {};
    TextureManager operator=(const TextureManager& r) {};
    TextureManager(TextureManager&& r) noexcept {};
    TextureManager& operator=(const TextureManager&& r) noexcept {};
public:
    static TextureManager* getInstance();

    Texture* getTexture(std::string filename);
    void printInfo();

    ~TextureManager();
};