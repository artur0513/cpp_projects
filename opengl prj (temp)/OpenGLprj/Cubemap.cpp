#include "Cubemap.h"

// =============== Cubemap ===============

Cubemap::Cubemap() {}

GLuint Cubemap::loadFromFile(std::string* filenames) {
    static GLenum types[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X ,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X ,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    Image* img;
    std::string filename, ext;
    for (int i = 0; i < 6; i++) {
        filename = std::filesystem::relative(filenames[i]).string(); // Приводм название файла к единому виду
        ext = filename.substr(filename.rfind(".")); // Берем расширение файла

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
        
        // Если тип сжатия = 0, т.е. без сжатия то используем одну функцию, иначе другую + грузим все мипмапы
        if (img->getCompressionType() != 0)
            for (uint32_t j = 0; j < img->getMipMapCount(); j++)
                glCompressedTexImage2D(types[i], j, img->getCompressionType(), img->getWidth(j), img->getHeight(j), 0, img->getSize(j), img->getData(j));
        else
            for (uint32_t j = 0; j < img->getMipMapCount(); j++)
                glTexImage2D(types[i], j, img->isRGBA() ? GL_RGBA : GL_RGB, img->getWidth(j), img->getHeight(j), 0, img->isRGBA() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img->getData(j));

        name[i] = filename;
        size[i].x = img->getWidth();
        size[i].y = img->getHeight();
        delete img;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //uint8_t* pixels = new uint8_t[512 * 512 * 4];
    //glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    //std::cout << int(pixels[0]) << " " << int(pixels[1]) << " " << int(pixels[2]) << " " << int(pixels[3]) << "\n";

    return id;
}

void Cubemap::bind(GLenum texture) {
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

const GLuint Cubemap::getId() { return id; }
Cubemap::~Cubemap() { glDeleteTextures(1, &id); }

GLuint createSkyboxVAO() {
    GLuint VAO, VBO;

    static float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    // Координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glEnableVertexAttribArray(0);

    return VAO;
}