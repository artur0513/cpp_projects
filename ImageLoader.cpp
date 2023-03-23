#include <iostream>
#include <string>
#include <fstream>
#include <vector>

class Image {
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

    void loadFromFile(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Can`t open file and load image: " << filename << "\n";
            file.close();
            return;
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
            return;
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
            return;
        }
        file.close();
    }
};



int main()
{
    Image img;
    img.loadFromFile("test.tga");

    /*
    Класс для загрузки .tga изображений

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
}
