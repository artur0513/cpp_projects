#include "stdio.h"
#include "string.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

void loadTGA (const char* filename, GLenum textureType) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
        printf("Error loading texture: file not found");

    unsigned char header[18];
    fread(header, 1, 18, file);
    int width = header[13] * 256 + header[12];
    int height = header[15] * 256 + header[14];
    int bitsPerPixel = header[16];
    int bytesPerPixel = header[16] / 8;
    int size = width * height * bytesPerPixel;

    unsigned char* pixels = malloc(size*sizeof(char));

    unsigned char temp;
    if (header[2] == 2) //uncomressed image, other types not supported in that version
        fread(pixels, 1, size, file);

    fclose(file);
    glTexImage2D(textureType, 0, bytesPerPixel==4 ? GL_RGBA : GL_RGB, width, height, 0, bytesPerPixel==4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
    free(pixels);
};


int main(int argc, char* argv[]) {

	freopen("redir.txt", "w", stdout);
    for (int i = 0; i < argc; i++) {
		printf("%s \n", argv[i]);
    }


    if (argc != 7) {
		printf("invalid number of arguements. You should drag&drop 6 images on this application\n");
        return EXIT_FAILURE;
	}

    if (!glfwInit()) {
        printf("glfwInit error\n");
        return 0;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    GLFWwindow* offscreen_context = glfwCreateWindow(640, 480, "", NULL, NULL);
    glfwMakeContextCurrent(offscreen_context);

    GLenum err = glewInit();
    if (err != GLEW_OK){
        printf("glewInit error\n");
        return 0;
    }

    // =========================== OpenGL initializing over, start loading shader ===========================

    const char* vertexShaderCode =
    "#version 330 core \n"
    "layout (location = 0) in vec2 pos; \n"
    "out vec2 onscreen; \n"
    "void main() {  \n"
    "   gl_Position = vec4(pos, 0.0, 1.0); \n"
    "   onscreen = pos; }; \n";
    const char* fragmentShaderCode =
    "#version 330 core \n"
    "out vec4 FragColor; \n"
    "in vec2 onscreen; \n"
    "uniform samplerCube cubemap; \n"
    "void main() { \n"
    "   float phi = (onscreen.x + 1.0) * 3.14159; \n"
    "   float theta = (onscreen.y + 1.0) * 1.57079; \n"
    "   vec3 dir = vec3(sin(theta)*cos(phi), -cos(theta), sin(theta)*sin(phi)); \n"
    "   FragColor = texture(cubemap, dir); }; \n";

    // Создаем вершинный шейдер
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);

    // Создаем фрагментный шейдер
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);

    // Обьединяем вершинный и фрагментный шейдер в одну программу
    GLuint shaderId = glCreateProgram();
    glAttachShader(shaderId, vertexShader);
    glAttachShader(shaderId, fragmentShader);
    glLinkProgram(shaderId);

    // Читаем ошибки
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shaderId, 1024, NULL, infoLog);
    printf("%s \n",infoLog);

    glDeleteShader(vertexShader); // Удаляем ненужное
    glDeleteShader(fragmentShader);
    glUseProgram(shaderId);

    // =========================== End of loading shader, start loading cubemap ===========================

    GLuint textureId;
    glGenTextures(1, &textureId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    for (size_t i = 1; i < 7; i++) {
        loadTGA(argv[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i - 1);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY, 16.f);

    // =========================== End of loading cubemap, start creating VAO, VBO, EBO ===========================

    GLuint VAO = 0, VBO = 0, EBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    float vertices[] = {-1.0, 1.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0};
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // =========================== Create frambuffer ===========================

    GLuint FBO = 0;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    GLuint bufferTextureId;
    glGenTextures(1, &bufferTextureId);
    glBindTexture(GL_TEXTURE_2D, bufferTextureId);
    unsigned sizeX = 4320, sizeY = 2160;
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, sizeX, sizeY);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTextureId, 0);
    glViewport(0, 0, sizeX, sizeY);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer status: %i \n", status);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    // =========================== Render to framebuffer ===========================

    glUniform1i(glGetUniformLocation(shaderId, "cubemap"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    unsigned char* image = malloc(sizeX*sizeY*4*sizeof(char));
    glReadPixels(0, 0, sizeX, sizeY, GL_RGBA, GL_UNSIGNED_BYTE, image);

    FILE *result = fopen("result.tga", "wb");
    putc(0,result);
    putc(0,result);
    putc(2,result);                         /* uncompressed RGB */
    putc(0,result); putc(0,result);
    putc(0,result); putc(0,result);
    putc(0,result);
    putc(0,result); putc(0,result);           /* X origin */
    putc(0,result); putc(0,result);           /* y origin */
    putc((sizeX & 0x00FF),result);
    putc((sizeX & 0xFF00) / 256,result);
    putc((sizeY & 0x00FF),result);
    putc((sizeY & 0xFF00) / 256,result);
    putc(32,result);                        /* 24 bit bitmap */
    putc(0,result);
    fwrite(image, sizeX*sizeY*4, 1, result);

    fclose(result);
    free(image);

    // =========================== Free memory ===========================

    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderId);

    printf("Pano creating successful!");
}
