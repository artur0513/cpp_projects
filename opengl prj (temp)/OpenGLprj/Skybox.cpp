#include "Skybox.h"

ogl::Shader skyboxShader;
GLuint VAO = 0, VBO = 0, EBO = 0;

ogl::Cubemap* currentCubemap;
m3d::mat4f* currentCameraMatrix;

float skyboxVertices[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
};

GLushort indices[]{
    0, 6, 4,
    0, 2, 6,
    0, 3, 2,
    0, 1, 3,
    2, 7, 6,
    2, 3, 7,
    4, 6, 7,
    4, 7, 5,
    0, 4, 5,
    0, 5, 1,
    1, 5, 7,
    1, 7, 3,
};

void ogl::Skybox::initSkybox() {
    if (!skyboxShader.loadFromFile("forTests/skyboxVertShader.txt", "forTests/skyboxFragShader.txt"))
        std::cerr << "Error loading skybox shader \n";

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ogl::Skybox::setCameraMatrix(m3d::mat4f& mat) {
    currentCameraMatrix = &mat;
}

void ogl::Skybox::setSkyboxCubemap(ogl::Cubemap& c) {
    currentCubemap = &c;
}

void ogl::Skybox::renderSkybox() {
    glBindVertexArray(VAO);
    skyboxShader.use();
    skyboxShader.setUniform("cameraMatrix", *currentCameraMatrix);
    skyboxShader.setUniform("skybox", *currentCubemap);
    skyboxShader.bindTextures();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}
