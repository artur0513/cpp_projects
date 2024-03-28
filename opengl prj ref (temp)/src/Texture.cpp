#include "Texture.h"
#include "Syslog.h"

namespace ogl {

    // Sampler class

    ogl::Sampler::Sampler() {
        glGenTextures(1, &id);
    };

    ogl::Sampler::Sampler(Sampler&& t) noexcept { // NEED TO CHECK
        id = t.id;
        t.id = 0;

        size = t.size;
        name = t.name;
        t_hasMipMap = t.t_hasMipMap;
        t_isSmooth = t.t_isSmooth;

        t.size = { 0, 0 };
        t.name.clear();
        t.name.push_back("");
        t.t_hasMipMap = false;
    }

    ogl::Sampler& ogl::Sampler::operator=(ogl::Sampler&& t) noexcept {
        id = t.id;
        t.id = 0;

        size = t.size;
        name = t.name;
        t_hasMipMap = t.t_hasMipMap;
        t_isSmooth = t.t_isSmooth;

        t.size = { 0, 0 };
        t.name.clear();
        t.name.push_back("");
        t.t_hasMipMap = false;

        return *this;
    }

    GLenum ogl::Sampler::getSamplerType() const noexcept { return 0; }

    void ogl::Sampler::bind(GLenum texture) const {
        glActiveTexture(texture);
        glBindTexture(getSamplerType(), id);
    }

    void ogl::Sampler::generateMipMap() {
        if (!t_hasMipMap || t_isRenderSampler) {
            glBindTexture(getSamplerType(), id);
            glGenerateTextureMipmap(id);
            if (t_isSmooth)
                glTexParameteri(getSamplerType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            else
                glTexParameteri(getSamplerType(), GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            t_hasMipMap = true;
        }
    }

    void ogl::Sampler::setSmooth(bool smooth) {
        if (smooth == t_isSmooth)
            return;

        t_isSmooth = smooth;
        glBindTexture(getSamplerType(), id);
        glTexParameteri(getSamplerType(), GL_TEXTURE_MAG_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);

        if (t_hasMipMap)
            glTexParameteri(getSamplerType(), GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
        else
            glTexParameteri(getSamplerType(), GL_TEXTURE_MIN_FILTER, t_isSmooth ? GL_LINEAR : GL_NEAREST);
    }

    float maxAnisotropy;
    void ogl::Sampler::setFilteringLevel(float _AFlevel) {
        static bool checkMaxAnisotropy = []() { // call only once to check max anisotropy level
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
            return true;
        } ();

        glBindTexture(getSamplerType(), id);
        AFlevel = (_AFlevel > maxAnisotropy) ? maxAnisotropy : _AFlevel;
        glTexParameterf(getSamplerType(), GL_TEXTURE_MAX_ANISOTROPY, AFlevel);
    }


    const float ogl::Sampler::getFilteringLevel() const { return AFlevel; }
    const GLuint ogl::Sampler::getId() const { return id; }
    const bool ogl::Sampler::hasMipMap() const { return t_hasMipMap; }
    const bool ogl::Sampler::isSmooth() const { return t_isSmooth; }
    const m3d::vec2<uint32_t> ogl::Sampler::getSize() const { return size; }
    const std::string& ogl::Sampler::getName(size_t i) const { return name[i]; }
    const bool ogl::Sampler::isRenderSampler() const { return t_isRenderSampler; }

    [[nodiscard]] GLuint ogl::Sampler::createColorAttachment(GLuint wx, GLuint wy, GLenum format) {
        if (loadOnce)
            return id;

        glBindTexture(getSamplerType(), id);
        glTexStorage2D(getSamplerType(), 1, format, wx, wy);
        setSmooth(true); // НЕ ТРОГАТЬ ЭТУ СТРОКУ И НИ ЗА ЧТО БЛЯТЬ В ЖИЗНИ НЕ МЕНЯТЬ НА setSmooth(t_hasMipMap) !

        loadOnce = true;
        return id;
    }

    [[nodiscard]] GLuint ogl::Sampler::createDepthAttachment(GLuint wx, GLuint wy) {
        return createColorAttachment(wx, wy, GL_DEPTH_COMPONENT32F);
    }

    Sampler::~Sampler() { glDeleteTextures(1, &id); }

    // Texture class

    GLenum ogl::Texture::getSamplerType() const noexcept { return GL_TEXTURE_2D; }

    GLuint ogl::Texture::loadFromFile(std::string filename) {
        if (loadOnce)
            return id;

        glBindTexture(GL_TEXTURE_2D, id);

        Image* img = loadImage(filename, GL_TEXTURE_2D);
        if (img == nullptr)
            return 0;

        // Установка параметров наложения текстуры
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        setFilteringLevel(AFlevel);

        t_hasMipMap = (img->getMipMapCount() > 1); // Сохраняем необходимые данные
        if (!t_hasMipMap)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        name.push_back(filename);
        size.x = img->getWidth();
        size.y = img->getHeight();

        setSmooth(true); // НЕ ТРОГАТЬ ЭТУ СТРОКУ И НИ ЗА ЧТО БЛЯТЬ В ЖИЗНИ НЕ МЕНЯТЬ НА setSmooth(t_hasMipMap) !

        delete img;
        Syslog::log("Texture loaded successfully: " + filename, Syslog::SUCSSES);
        loadOnce = true;
        return id;
    }

    // Cubemap class

    GLenum ogl::Cubemap::getSamplerType() const noexcept { return GL_TEXTURE_CUBE_MAP; }

    GLuint ogl::Cubemap::loadFromFile(std::string filenames[]) {
        if (loadOnce)
            return id;

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        t_hasMipMap = true;
        GLuint compressionType;
        ogl::Image* img;
        for (int i = 0; i < 6; i++) {
            img = ogl::loadImage(filenames[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
            if (img == nullptr)
                return 0;

            //std::cout << i << " | " << filenames[i] << "\n";
            name.push_back(filenames[i]);

            t_hasMipMap &= bool(img->getMipMapCount() - 1);
            if (i == 0) {
                compressionType = img->getCompressionType();
                size.x = img->getWidth();
                size.y = img->getHeight();
                if (size.x != size.y) {
                    std::cerr << "Error while loading cubemap: " << filenames[i] << " - each side must be a square \n";
                    return 0;
                }
            }
            else {
                if (compressionType != img->getCompressionType()) {
                    std::cerr << "Error while loading cubemap: " << filenames[i] << " - textures compression types must be same \n";
                    return 0;
                }
                if (size.x != img->getWidth() || size.y != img->getHeight()) {
                    std::cerr << "Error while loading cubemap: " << filenames[i] << " - each side must have same resolution \n";
                    return 0;
                }
            }
            delete img;
        }
        setSmooth(true); // НЕ ТРОГАТЬ ЭТУ СТРОКУ И НИ ЗА ЧТО БЛЯТЬ В ЖИЗНИ НЕ МЕНЯТЬ НА setSmooth(t_hasMipMap) !

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        setFilteringLevel(AFlevel);

        Syslog::log("Cubemap loaded successfully: " + std::string(filenames[0]), Syslog::SUCSSES);
        loadOnce = true;
        return id;
    }

}
