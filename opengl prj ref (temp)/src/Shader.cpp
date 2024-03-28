#include "Shader.h"
#include "Syslog.h"

namespace ogl {
    ShaderUniform::ShaderUniform(GLenum _type, GLint _size, GLint _location, std::string _name) {
        type = _type;
        size = _size;
        location = _location;
        name = _name;
    }
    GLenum ShaderUniform::getType() const { return type; }
    GLint ShaderUniform::getSize() const { return size; }
    GLint ShaderUniform::getLocation() const { return location; }
    const std::string& ShaderUniform::getName() const { return name; }


    // https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
    std::string readFile(const std::string& path) {
        constexpr auto read_size = std::size_t(4096);
        std::ifstream file(path);

        std::string out;
        std::string buf(read_size, '\0');
        while (file.read(&buf[0], read_size)) {
            out.append(buf, 0, file.gcount());
        }
        out.append(buf, 0, file.gcount());
        return out;
    }

    void Shader::use() noexcept { glUseProgram(id); }
    GLuint Shader::getId() noexcept { return id; }

    bool Shader::loadFromString(const std::string& vertexCode, const std::string& fragmentCode, bool noFilePath) noexcept {
        if (noFilePath){
            vertexFilePath = "Loaded from string, no file path";
            fragmentFilePath = "Loaded from string, no file path";
        }

        const char* vertexCodeChar = vertexCode.c_str(), *fragmentCodeChar = fragmentCode.c_str();
        GLuint vertexShader, fragmentShader;

        // Создаем вершинный шейдер
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexCodeChar, NULL);
        glCompileShader(vertexShader);

        // Создаем фрагментный шейдер
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentCodeChar, NULL);
        glCompileShader(fragmentShader);

        // Обьединяем вершинный и фрагментный шейдер в одну программу
        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);

        // Читаем ошибки
        GLint success, overallSuccess = 1;
        GLchar infoLog[1024];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        overallSuccess *= success;
        glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
        if (std::string(infoLog).size() != 0)
            Syslog::log("Warnings and error from vertex shader " + vertexFilePath + ": " + infoLog, Syslog::WARNING);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        overallSuccess *= success;
        glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
        if (std::string(infoLog).size() != 0)
            Syslog::log("Warnings and error from fragment shader " + fragmentFilePath + ": " + infoLog, Syslog::WARNING);

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        overallSuccess *= success;
        glGetProgramInfoLog(id, 1024, NULL, infoLog);
        if (std::string(infoLog).size() != 0)
            Syslog::log("Warnings and error from linking shaders " + vertexFilePath + " and " + fragmentFilePath + ": " + infoLog, Syslog::WARNING);
        if (!overallSuccess) {
            Syslog::log("Error compilating/linking shader!\n", Syslog::ERROR);
            return 0;
        }

        // Analyzing uniforms and creating table
        GLint count; // number of active uniforms
        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        const GLsizei bufSize = 32; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length

        glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count); // Узнаем число юниформ
        glUseProgram(id);
        for (int i = 0; i < count; i++) {
            glGetActiveUniform(id, (GLuint)i, bufSize, &length, &size, &type, name); // Узнаем параметры всех униформ
            // looks like always true: i == glGetUniformLocation(id, name)

            if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE) { // Если тип - текстура, то в отдельный массив сохраняем, так как их сложнее биндить
                textureUniforms.emplace_back(type, size, i, std::string(name));
                glUniform1i(textureUniforms.back().getLocation(), (GLint)(textureUniforms.size()-1));
            }
            else
                mathUniforms.emplace_back(type, size, i, std::string(name)); // Другие типы (численные), сохраняем тоже отдельно
        }

        glDeleteShader(vertexShader); // Удаляем ненужное
        glDeleteShader(fragmentShader);
        Syslog::log("Shader loaded successfully: " + vertexFilePath + " , " + fragmentFilePath, Syslog::SUCSSES);
        return id;
    }

    bool Shader::loadFromFile(const std::string& _vertexFilePath, const std::string& _fragmentFilePath) noexcept {
        vertexFilePath = _vertexFilePath;
        fragmentFilePath = _fragmentFilePath;
        return loadFromString(readFile(vertexFilePath), readFile(fragmentFilePath), false);
    }

    // Поиск нужной юниформы по имени в нужном массиве. Если ищем текстуру, то в textureUniforms, иначе в mathUniforms.
    std::vector<ShaderUniform>::const_iterator Shader::findUniform(const std::vector<ShaderUniform>& vec, const std::string& name) noexcept {
        for (auto uniform = vec.begin(); uniform < vec.end(); uniform++)
            if (uniform->getName() == name)
                return uniform;
        return vec.end();
    }

    void Shader::setUniform(const std::string& name, int v) noexcept {
        auto uniform = findUniform(mathUniforms, name); // Юниформу типа int ищем в mathUniforms
        if (uniform == mathUniforms.end()) // Если не нашли, то ничего не делаем
            return;
        if (uniform->getType() != GL_INT) // Если тип найденной переменной на самом деле не int, то тоже ничего не делаем.
            return;
        glUniform1i(uniform->getLocation(), v); // Если все ок, то устанавливаем значение униформы
    }

    void Shader::setUniform(const std::string& name, float v) noexcept {
        auto uniform = findUniform(mathUniforms, name);
        if (uniform == mathUniforms.end())
            return;
        if (uniform->getType() != GL_FLOAT)
            return;
        glUniform1f(uniform->getLocation(), v);
    }

    void Shader::setUniform(const std::string& name, const m3d::vec2f& v) noexcept {
        auto uniform = findUniform(mathUniforms, name);
        if (uniform == mathUniforms.end())
            return;
        if (uniform->getType() != GL_FLOAT_VEC2)
            return;
        glUniform2f(uniform->getLocation(), v.x, v.y);
    }

    void Shader::setUniform(const std::string& name, const m3d::vec3f& v) noexcept {
        auto uniform = findUniform(mathUniforms, name);
        if (uniform == mathUniforms.end())
            return;
        if (uniform->getType() != GL_FLOAT_VEC3)
            return;
        glUniform3f(uniform->getLocation(), v.x, v.y, v.z);
    }

    void Shader::setUniform(const std::string& name, const m3d::vec4f& v) noexcept {
        auto uniform = findUniform(mathUniforms, name);
        if (uniform == mathUniforms.end())
            return;
        if (uniform->getType() != GL_FLOAT_VEC4)
            return;
        glUniform4f(uniform->getLocation(), v.x, v.y, v.z, v.w);
    }

    void Shader::setUniform(const std::string& name, const m3d::mat4f& v) noexcept {
        auto uniform = findUniform(mathUniforms, name);
        if (uniform == mathUniforms.end())
            return;
        if (uniform->getType() != GL_FLOAT_MAT4)
            return;
        glUniformMatrix4fv(uniform->getLocation(), 1, GL_FALSE, v());
    }


    void Shader::setUniform(const std::string& name, const ogl::Sampler* v) noexcept {
        auto uniform = findUniform(textureUniforms, name);
        if (uniform == textureUniforms.end())
            return;
        //if (uniform->getType() != v->getSamplerType()) INCORRECT CHECK! one starts with gl_sampler_..., other with gl_texture_...
        //   return;
        int texUnit = uniform - textureUniforms.begin();
        v->bind(GL_TEXTURE0 + texUnit);
        glUniform1i(uniform->getLocation(), texUnit);
    }

}
