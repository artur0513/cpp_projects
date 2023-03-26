#include "Texture.h"
#include <filesystem>

uint32_t TGAImage::getWidth(int mipMapLevel) { return mipMaps[0].width; }
uint32_t TGAImage::getHeight(int mipMapLevel) { return mipMaps[0].height; }
uint32_t TGAImage::getSize(int mipMapLevel) { return mipMaps[0].size; }
bool TGAImage::isRGBA() { return bytesPerPixel == 4; }
GLuint TGAImage::getCompressionType() { return 0; }
uint32_t TGAImage::getMipMapCount() { return 0; }
const uint8_t* TGAImage::getData(int mipMapLevel) { return mipMaps[0].pixels.data(); }

bool TGAImage::loadFromFile(std::string filename) {
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
    file.close();
    return true;
}

uint32_t DDSImage::getWidth(int mipMapLevel) { return mipMaps[mipMapLevel].width; }
uint32_t DDSImage::getHeight(int mipMapLevel) { return mipMaps[mipMapLevel].height; }
uint32_t DDSImage::getSize(int mipMapLevel) { return mipMaps[mipMapLevel].size; }
uint32_t DDSImage::getMipMapCount() { return mipMapCount; }
GLuint DDSImage::getCompressionType() { return compressionType; }
const uint8_t* DDSImage::getData(int mipMapLevel) { return mipMaps[mipMapLevel].pixels.data(); }
bool DDSImage::isRGBA() { return true; }

bool DDSImage::loadFromFile(std::string filename) {
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
    if (type_str == "DXT1") // DXT1 check
        compressionType = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    else if (type_str == "DXT3") // DXT3 check
        compressionType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    else if (type_str == "DXT5") // DXT5 check
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
    file.close();
    return true;
}

std::map<std::string, GLuint> Texture::allTextures;

GLuint Texture::loadFromFile(std::string filename) {
    filename = std::filesystem::relative(filename).string(); // Приводм название файла к единому виду
    auto tex = allTextures.find(filename);

    if (tex != allTextures.end()) { // Если нужная текстура уже загружена, то берем ее id
        id = tex->second;
        return id;
    }
    
    // Теперь, если текстуру еще не загрузили
    std::string ext = filename.substr(filename.rfind(".")); // Берем расширение файла

    Image* img;
    if (ext == ".dds") {
        img = new DDSImage();
        if (!img->loadFromFile(filename)) {
            std::cerr << "Unable to load texture: " << filename << "\n";
            return 0;
        }

    }
    else if (ext == ".tga") {
        img = new TGAImage();
        if (!img->loadFromFile(filename)) {
            std::cerr << "Unable to load texture: " << filename << "\n";
            return 0;
        }
    }
    else {
        std::cerr << "Unknown texture format. Supported are .dds and .tga: " << filename << "\n";
        return 0;
    }

    // Если дошли до сюда, то значит в img загружена текстура
    
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    // Установка параметров наложения текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложения текстуры GL_REPEAT (стандартный метод наложения)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Установка параметров фильтрации текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Если тип сжатия = 0, т.е. без сжатия то используем одну функцию, иначе другую + грузим все мипмапы
    if (img->getCompressionType() != 0)
        for (uint32_t i = 0; i < img->getMipMapCount(); i++)
            glCompressedTexImage2D(GL_TEXTURE_2D, i, img->getCompressionType(), img->getWidth(i), img->getHeight(i), 0, img->getSize(i), img->getData(i));
    else 
        for (uint32_t i = 0; i < img->getMipMapCount(); i++)
            glTexImage2D(GL_TEXTURE_2D, 0, img->isRGBA() ? GL_RGBA : GL_RGB, img->getWidth(), img->getHeight(), 0, img->isRGBA() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img->getData());

    allTextures.insert(std::pair<std::string, GLuint>(filename, id));
    return id;
}