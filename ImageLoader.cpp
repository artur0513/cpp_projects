#include <iostream>
#include <string>
#include <fstream>
#include <vector>

enum GLuint { // Delete this, and replace with actual OpenGL code
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};

class TGAImage {
private:
    uint32_t width, height, bitsPerPixel, size;
    std::vector<uint8_t> pixels;
    uint32_t bytesPerPixel;

    union TGA_Pixel {
        std::uint32_t color;
        struct
        {
            std::uint8_t B, G, R, A;
        };
    };
public:
    uint32_t getWidth() { return width; }
    uint32_t getHeight() { return height; }
    uint32_t getSize() { return size; }
    bool isRGBA() { return bytesPerPixel == 4; }
    auto getData() { return pixels.data(); }

    bool loadFromFile(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Can`t open file and load image: " << filename << "\n";
            file.close();
            return false;
        }

        uint8_t header[18] = { 0 };
        file.read(reinterpret_cast<char*>(&header), 18);
        width = header[13] * 256 + header[12];
        height = header[15] * 256 + header[14];
        bitsPerPixel = header[16];
        bytesPerPixel = bitsPerPixel / 8;
        size = width * height * bytesPerPixel;

        if (bitsPerPixel != 24 && bitsPerPixel != 32) {
            std::cerr << "Unknown file format, required 24 or 32 bits per pixel: " << filename << "\n";
            file.close();
            return false;
        }

        TGA_Pixel pixel;
        auto save_pixel = [&](uint32_t& pixel_count) { // lambda for automaticly saving pixel to vector, and swapping BGRA to RGBA 
            uint32_t id = pixel_count * bytesPerPixel;
            pixels[id] = pixel.R;
            pixels[id + 1] = pixel.G;
            pixels[id + 2] = pixel.B;
            if (bytesPerPixel == 4)
                pixels[id + 3] = pixel.A;
        };

        if (header[2] == 2) { //uncomressed image
            pixels.resize(size);
            for (uint32_t i = 0; i < width * height; i++) {
                file.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);
                save_pixel(i);
            }

        }
        else if (header[2] == 10) { //encoded image
            pixels.resize(size);
            uint32_t current_pixel = 0;
            uint8_t block_header;

            while (current_pixel < width * height) {
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
        file.close();
        return true;
    }
};

// Do not forget that DDS texuters will be Y-inverted
class DDSImage {
private:
    uint32_t mipMapCount = 0;
    GLuint compressionType;

    struct mipMap {
        uint32_t width, height, size;
        std::vector<uint8_t> pixels;
    };

    std::vector<mipMap> mipMaps;

    struct DDS_PIXELFORMAT {
        uint32_t dwSize; // DDS_PIXELFORMAT size
        uint32_t dwFlags;
        uint8_t dwFourCC[4]; // Compression format. There will be something like DXT1 (4 bytes, each byte is char)
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
    uint32_t getWidth(int mipMapLevel = 0) { return mipMaps[mipMapLevel].width; }
    uint32_t getHeight(int mipMapLevel = 0) { return mipMaps[mipMapLevel].height; }
    uint32_t getSize(int mipMapLevel = 0) { return mipMaps[mipMapLevel].size; }
    auto getData(int mipMapLevel = 0) { return mipMaps[mipMapLevel].pixels.data(); }

    bool loadFromFile(std::string filename) {
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

        mipMaps[0].width = header.dwWidth;
        mipMaps[0].height = header.dwHeight;
        mipMaps[0].size = header.dwPitchOrLinearSize;


        if (header.ddspf.dwFourCC[0] == 'D' && header.ddspf.dwFourCC[1] == 'X' && header.ddspf.dwFourCC[2] == 'T' && header.ddspf.dwFourCC[3] == '1') // DXT1 check
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        else if (header.ddspf.dwFourCC[0] == 'D' && header.ddspf.dwFourCC[1] == 'X' && header.ddspf.dwFourCC[2] == 'T' && header.ddspf.dwFourCC[3] == '3') // DXT3 check
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        else if (header.ddspf.dwFourCC[0] == 'D' && header.ddspf.dwFourCC[1] == 'X' && header.ddspf.dwFourCC[2] == 'T' && header.ddspf.dwFourCC[3] == '5') // DXT5 check
            compressionType = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        else {
            std::cerr << "Unsupported DDS compression type. Supported are DXT1, DXT3, DXT5: " << filename << "\n";
            file.close();
            return false;
        }

        uint32_t blockSize = (compressionType == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        uint32_t iHeight = mipMaps[0].height, iWidth = mipMaps[0].width, iSize, sumsize = 0;
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
        file.close();
        return true;
    }
};

int main()
{
    TGAImage img;
    img.loadFromFile("test.tga");

    /* TGA
    Протестированы в OpenGL такие варианты:
    24-bit сжатое изображение (без альфа канала)
    24-bit НЕ сжатое изображение (без альфа канала)
    32-bit сжатое изображение
    32-bit НЕ сжатое изображение

    Со всеми этими случаями работает нормально! Другие варианты .tga изображений не поддерживаются, всякие ЧБ изображения, с палитрой, и т.д.
    Нужно разобраться с проверкой, что перед нами действтельно .tga файл, и посмотреть что будет если подсовывть коду битые файлы (+ случай, когда размер файла < 18 байт)

    Можно использовать такую функцю загрузки:
    glTexImage2D(GL_TEXTURE_2D, 0, img.isRGBA()? GL_RGBA : GL_RGB, img.getWidth(), img.getHeight(), 0, img.isRGBA() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img.getData());
    */

    DDSImage img2;
    img2.loadFromFile("test.dds");

    /* DDS
    Протестирована в OpenGL текстуры с mipMap-ами и без, на DXT1, DXT3, DXT5, и оно работает. 
    Возможно есть другие разновидности dds-ок, хз как проверять, например какое бывает число бит на пиксель и т.д. В dds сохраненных в paint.net соответсвующие 
    поля uint32_t dwRGBBitCount; uint32_t dwRBitMask; uint32_t dwGBitMask; uint32_t dwBBitMask; uint32_t dwABitMask; ??? вообще равны 0;

    Т.к. dds формат был сделан для direct3D, а там система координат перевернута по Y относительно openGL, то и текстуры получатся перевернутыми. Это можно
    исправить либо в шейдере, либо при загрузке 3д модели (самый плохой вариант как по мне), либо просто в графическом редакторе

    Можно использовать такую функцю загрузки:
    i = Номер Мипмапы
    glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, img.getWidth(i), img.getHeight(i), 0, img.getSize(i), img.getData(i));
    Вместо GL_COMPRESSED_RGBA_S3TC_DXT1_EXT указать тип сжатия из класса GLuint compressionType;
    */
}
