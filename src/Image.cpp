#include "Image.h"
#include "Syslog.h"

namespace ogl {

    uint32_t Image::getWidth(int mipMapLevel) {
        if (mipMaps.size() <= mipMapLevel)
            return 0;
        return mipMaps[mipMapLevel].width;
    }

    uint32_t Image::getHeight(int mipMapLevel) {
        if (mipMaps.size() <= mipMapLevel)
            return 0;
        return mipMaps[mipMapLevel].height;
    }

    uint32_t Image::getSize(int mipMapLevel) {
        if (mipMaps.size() <= mipMapLevel)
            return 0;
        return mipMaps[mipMapLevel].size;
    }

    const uint8_t* Image::getData(int mipMapLevel) {
        if (mipMaps.size() <= mipMapLevel)
            return nullptr;
        return mipMaps[mipMapLevel].pixels.data();
    }

    uint32_t Image::getMipMapCount() { return mipMapCount; }
    GLuint Image::getCompressionType() { return compressionType; }
    bool Image::isRGBA() { return bytesPerPixel == 4; }

    // =============== TGAImage loading function ===============

    union TGA_Pixel {
        std::uint32_t color;
        struct {
            std::uint8_t B, G, R, A;
        };
    };

    bool TGAImage::loadFromFile(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            Syslog::log("Can`t open file and load image: " + filename, Syslog::ERROR);
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
            Syslog::log("Unknown file format, required 24 or 32 bits per pixel: " + filename, Syslog::ERROR);
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
            Syslog::log("This TGA type is unsupported. Supported types are uncompressed or encoded true-color image: " + filename, Syslog::ERROR);
            file.close();
            return false;
        }

        if (file.eof() || file.fail() || file.bad()) {
            Syslog::log("Fstream error while loading texture: " + filename, Syslog::ERROR);
            file.close();
            return false;
        }

        file.close();
        return true;
    }

    // =============== DDSImage ===============

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
    }; // DDS_HEADER + "DDS " signature in the beginning of file = 128 bytes (looks like this is always true)

    bool DDSImage::loadFromFile(std::string filename) {
        DDS_HEADER header;
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            Syslog::log("Can`t open file and load image: " + filename, Syslog::ERROR);
            file.close();
            return false;
        }

        uint32_t format;
        file.read(reinterpret_cast<char*>(&format), sizeof(format));
        if (format != 0x20534444) {
            Syslog::log("File format is not DDS: " + filename, Syslog::ERROR);
            file.close();
            return false;
        }

        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        mipMapCount = (header.dwMipMapCount == 0) ? 1 : header.dwMipMapCount;
        mipMaps.resize(mipMapCount);

        std::string type_str(header.ddspf.dwFourCC);
        if (type_str == "DXT1")
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        else if (type_str == "DXT3")
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        else if (type_str == "DXT5")
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        else {
            Syslog::log("Unsupported DDS compression type. Supported are DXT1, DXT3, DXT5: " + filename, Syslog::ERROR);
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
            Syslog::log("Fstream error while loading texture: " + filename, Syslog::ERROR);
            file.close();
            return false;
        }

        file.close();
        return true;
    }

    // =============== loadImage function ===============

    ogl::Image* loadImage(std::string filename, GLenum textureType) {
        if (!std::filesystem::exists(filename)) {
            Syslog::log("Trying to load a texture from a non-existent file: " + filename, Syslog::ERROR);
            return nullptr;
        }
        filename = std::filesystem::relative(filename).string(); // Приводм название файла к единому виду
        std::string ext = filename.substr(filename.rfind('.')); // Берем расширение файла

        Image* img;
        if (ext == ".dds") {
            img = new DDSImage();
            if (!img->loadFromFile(filename)) {
                Syslog::log("Unable to load texture: " + filename, Syslog::ERROR);
                return nullptr;
            }

        }
        else if (ext == ".tga") {
            img = new TGAImage();
            if (!img->loadFromFile(filename)) {
                Syslog::log("Unable to load texture: " + filename, Syslog::ERROR);
                return nullptr;
            }
        }
        else {
            Syslog::log("Unknown texture format. Supported are .dds and .tga: " + filename, Syslog::ERROR);
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

}
