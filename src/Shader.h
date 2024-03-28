#pragma once
#include "Syslog.h"
#include <GL/glew.h>
#include "3dMath/3dMath.h"
#include <vector>
#include "Texture.h"

namespace ogl {
    struct ShaderUniform {
    private:
        GLenum type;
        GLint size, location;
        std::string name;
    public:
        ShaderUniform(GLenum _type, GLint _size, GLint _location, std::string _name);
        GLenum getType() const;
        GLint getSize() const;
        GLint getLocation() const;
        const std::string& getName() const;
    };


    class Shader {
    private:
        std::vector<ShaderUniform> mathUniforms, textureUniforms;
        std::string vertexFilePath, fragmentFilePath;
        GLuint id = 0;

        std::vector<ShaderUniform>::const_iterator findUniform(const std::vector<ShaderUniform>& vec, const std::string& name) noexcept;
    public:
        bool loadFromString(const std::string& vertexCode, const std::string& fragmentCode, bool noFilePath = true) noexcept;
        bool loadFromFile(const std::string& _vertexFilePath, const std::string& _fragmentFilePath) noexcept;

        void use() noexcept;
        GLuint getId() noexcept;

        void setUniform(const std::string& name, int v) noexcept;
        void setUniform(const std::string& name, float v) noexcept;
        void setUniform(const std::string& name, const ogl::Sampler* v) noexcept;
        void setUniform(const std::string& name, const m3d::vec2f& v) noexcept;
        void setUniform(const std::string& name, const m3d::vec3f& v) noexcept;
        void setUniform(const std::string& name, const m3d::vec4f& v) noexcept;
        void setUniform(const std::string& name, const m3d::mat4f& v) noexcept;
    };

}

