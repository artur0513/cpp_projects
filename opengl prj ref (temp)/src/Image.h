#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <GL/glew.h>
#include "3dMath/3dMath.h"

namespace ogl {

    class Image {
    protected:
        uint32_t bitsPerPixel = 32, bytesPerPixel = 4;
        uint32_t mipMapCount = 0;
        GLuint compressionType = 0;

        struct mipMap {
            uint32_t width, height, size;
            std::vector<uint8_t> pixels;
        };

        std::vector<mipMap> mipMaps;
    public:
        uint32_t getWidth(int mipMapLevel = 0);
        uint32_t getHeight(int mipMapLevel = 0);
        uint32_t getSize(int mipMapLevel = 0);
        uint32_t getMipMapCount();
        GLuint getCompressionType();
        bool isRGBA();
        const uint8_t* getData(int mipMapLevel = 0);

        [[nodiscard]] virtual bool loadFromFile(std::string filename) = 0;
        virtual ~Image() = default;
    };

    class TGAImage : public Image {
    public:
        [[nodiscard]] bool loadFromFile(std::string filename) override;
        ~TGAImage() override = default;
    };

    // Do not forget that DDS texuters will be Y-inverted
    class DDSImage : public Image {
    public:
        [[nodiscard]] bool loadFromFile(std::string filename) override;
        ~DDSImage() override = default;
    };

    Image* loadImage(std::string filename, GLenum textureType);
}
