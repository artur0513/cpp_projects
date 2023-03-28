#include "Shader.h"

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

GLuint Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCodeStr = readFile(vertexPath), fragmentCodeStr = readFile(fragmentPath);
    const char* vertexCodeChar = vertexCodeStr.c_str(), * fragmentCodeChar = fragmentCodeStr.c_str();
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
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
        return 0;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
        return 0;
    }

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
        return 0;
    }

    // Удаляем ненужное
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return id;
}

void Shader::use() { glUseProgram(id); }

GLint Shader::getUniformLocation(const std::string& name) {
    auto uniform = uniforms.find(name);

    if (uniform != uniforms.end())
        return uniform->second;

    GLint location = glGetUniformLocation(id, name.c_str());
    if (location != -1)
        uniforms.insert(std::pair<std::string, GLint>(name, location));

    std::cout << "location: " << location << "\n";
    return location;
}

void Shader::setUniform(const std::string& name, int v) { glUniform1i(getUniformLocation(name), v); }
void Shader::setUniform(const std::string& name, float v) { glUniform1f(getUniformLocation(name), v); }
void Shader::setUniform(const std::string& name, Texture& v) { glUniform1i(getUniformLocation(name), v.getId()); }
void Shader::setUniform(const std::string& name, m3d::vec2f& v) { glUniform2f(getUniformLocation(name), v.x, v.y); }
void Shader::setUniform(const std::string& name, m3d::vec3f& v) { glUniform3f(getUniformLocation(name), v.x, v.y, v.z); }
void Shader::setUniform(const std::string& name, m3d::vec4f& v) { glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w); }
void Shader::setUniform(const std::string& name, m3d::mat4f& v) { glUniformMatrix4fv(getUniformLocation(name), 16, GL_TRUE, v()); }

Shader::~Shader() { glDeleteProgram(id); }

