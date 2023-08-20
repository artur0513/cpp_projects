#include "Skybox.h"

ogl::Shader skyboxShader;
GLuint VAO = 0, VBO = 0, EBO = 0;

ogl::Cubemap* currentCubemap;
ogl::Camera* currentCamera;
m3d::mat4f* currentCameraMatrix;

// non optimized version, can be used only one poly, like in my javascript cubemap render
float skyboxVertices[] = {
1.0, 1.0, -1.0,
1.0, -1.0, -1.0,
1.0, 1.0, 1.0,
1.0, -1.0, 1.0,
-1.0, 1.0, -1.0,
-1.0, -1.0, -1.0,
-1.0, 1.0, 1.0,
-1.0, -1.0, 1.0,
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ogl::Skybox::setCamera(ogl::Camera& cam) {
    currentCamera = &cam;
}

void ogl::Skybox::setSkyboxCubemap(ogl::Cubemap& c) {
    currentCubemap = &c;
}

void ogl::Skybox::renderSkybox() {
    // We can use line below to use z-buffer in read-only mode
    // No need to fix skybox fragment shader for reverse-z
    // but need to draw skybox before all other stuff
    //glDepthMask(GL_FALSE); 

    glBindVertexArray(VAO);
    skyboxShader.use();
    skyboxShader.setUniform("matrix", currentCamera->getSkyboxTransform());
    skyboxShader.setUniform("skybox", *currentCubemap);
    skyboxShader.bindTextures();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    //glDepthMask(GL_TRUE);
}
