#include "Shader.h"
#include <cassert>

using namespace ogl;

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

GLuint Shader::loadFromMemory(const std::string& vertexCode, const std::string& fragmentCode) {
    const char* vertexCodeChar = vertexCode.c_str(), * fragmentCodeChar = fragmentCode.c_str();
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

GLuint Shader::loadFromFile(const std::string& _vertexPath, const std::string& _fragmentPath) {
    vertexPath = _vertexPath;
    fragmentPath = _fragmentPath;
    std::string vertexCodeStr = readFile(_vertexPath), fragmentCodeStr = readFile(_fragmentPath);
    return loadFromMemory(vertexCodeStr, fragmentCodeStr);
}

GLuint Shader::getId() {
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

    return location;
}

void Shader::setUniform(const std::string& name, int v) {
    // Trying to use a uniform that was previously used for a texture
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniform1i(getUniformLocation(name), v);
}

void Shader::setUniform(const std::string& name, float v) { 
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniform1f(getUniformLocation(name), v);
}

void Shader::setUniform(const std::string& name, m3d::vec2f& v) {
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniform2f(getUniformLocation(name), v.x, v.y);
}

void Shader::setUniform(const std::string& name, m3d::vec3f& v) {
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniform3f(getUniformLocation(name), v.x, v.y, v.z);
}

void Shader::setUniform(const std::string& name, m3d::vec4f& v) { 
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w);
}

void Shader::setUniform(const std::string& name, const m3d::mat4f& v) {
    assert(textureTable.find(getUniformLocation(name)) == textureTable.end() && cubemapTable.find(getUniformLocation(name)) == cubemapTable.end());
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, v());
}

void Shader::setUniform(const std::string& name, Texture& v) { 
    GLint uniformLocation = getUniformLocation(name);
    auto tableLocation = textureTable.find(uniformLocation);
    if (uniformLocation == -1)
        return;

    else if (tableLocation == textureTable.end())
        textureTable.insert(std::pair<GLint, Texture*>(uniformLocation, &v));
    else
        tableLocation->second = &v;
}

void Shader::setUniform(const std::string& name, Cubemap& v) {
    GLint uniformLocation = getUniformLocation(name);
    auto tableLocation = cubemapTable.find(uniformLocation);
    if (uniformLocation == -1)
        return;

    else if (tableLocation == cubemapTable.end())
        cubemapTable.insert(std::pair<GLint, Cubemap*>(uniformLocation, &v));
    else
        tableLocation->second = &v;
}

void Shader::setUniform(const ogl::Material& mat) {
    if (mat.diffuseTexture != nullptr)
        setUniform("map_Kd", *mat.diffuseTexture);
    // add more parametrs when needed
}

const std::string& Shader::getVertexPath() const {
    return vertexPath;
}
const std::string& Shader::getFragmentPath() const {
    return fragmentPath;
}

void Shader::bindTextures() {
    GLint index = 0;
    //std::cout << maxTextureUnits << "\n";
    for (auto& t : textureTable) {
        //assert(index < maxTextureUnits); // Trying to bind too many textures ( > maxTextureUnits)
        t.second->bind(GL_TEXTURE0 + index);
        glUniform1i(t.first, index);
        index++;
    }
    for (auto& c : cubemapTable) {
        //assert(index < maxTextureUnits); // Trying to bind too many textures ( > maxTextureUnits)
        c.second->bind(GL_TEXTURE0 + index);
        glUniform1i(c.first, index);
        index++;
    }
}

GLint Shader::getMaxTextureUnits() {
    GLint maxUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
    std::cout << "max texture units: " << maxUnits << "\n";
    return maxUnits;
}

void Shader::printInfo() {
    std::cout << "================ Shader info ================\n";
    std::cout << "Vertex Path: " + vertexPath << "\nFragment Path: " << fragmentPath << "\n";
    for (auto& u : uniforms)
        std::cout << "Uniform name: " << u.first << " location: " << u.second << "\n";

    std::cout << "Uniforms used for textures:\n";
    for (auto& t : textureTable)
         std::cout << "Location: " << t.first << " texture: " << t.second->getName() << "\n";
}

Shader::~Shader() { glDeleteProgram(id); }

