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
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
    }

    // Удаляем ненужное
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return id;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    loadFromFile(vertexPath, fragmentPath);
}

void Shader::use() { glUseProgram(id); }

