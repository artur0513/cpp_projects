#include <iostream>
#include "3dMath/mat4.h"
#include "Shader.h"
#include "Syslog.h"
#include "Window.h"
#include "UI.h"
#include "3dMath/3dMath.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Font.h"

int main() {
    ogl::Window::init();

    ogl::Shader shader;
    if (!shader.loadFromFile("forTests/vertex.txt", "forTests/fragment.txt"))
        Syslog::log("Unable to load shader");

    // For test purposes START
    ogl::Cubemap st;
    std::string skybox1472[] = { "forTests/skybox1472/sky_l1escape1_bk.dds", "forTests/skybox1472/sky_l1escape1_fr.dds",
    "forTests/skybox1472/sky_l1escape1_up.dds" , "forTests/skybox1472/sky_l1escape1_down.dds" , "forTests/skybox1472/sky_l1escape1_lf.dds" , "forTests/skybox1472/sky_l1escape1_rt.dds" };
    st.loadFromFile(skybox1472);

    std::string skyboxBricks[] = { "forTests/bricks_nomips.dds", "forTests/bricks_nomips.dds",
    "forTests/bricks_nomips.dds" , "forTests/bricks_nomips.dds" , "forTests/bricks_nomips.dds" , "forTests/bricks_nomips.dds" };
    ogl::Cubemap st2;
    st2.loadFromFile(skyboxBricks);


    ogl::Font font("forTests/font/ui_font_letter_25_1600.dds", "forTests/font/ui_font_letter_25_1600.ini");
    // TEXTURES WITHOUT MIPMAPS APPEAR BLACK IN THIS FONT SHIT
    ogl::Text text;
    text.setFont(font);
    text.setText("AB C Def !@#");
    text.setColor(m3d::vec4f(1.0, 1.0, 1.0, 1.0));

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

    GLuint VAO = 0, VBO = 0, EBO = 0;
    ogl::Shader skyboxShader;
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

    skyboxShader.use();
    m3d::mat4f mat = m3d::mat4f().init_perspective_reversed(m3d::PersProjInfo());

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "before: " << err << "\n";
    }
    ogl::Window::setVsync(1);

    ogl::Performance::init();

    while (ogl::Window::isOpen()) {

        glBindVertexArray(VAO);
        ogl::Window::getFramebuffer()->bind();
        skyboxShader.use();
        mat = mat * m3d::mat4f().init_rotation_Y(0.001);
        skyboxShader.setUniform("skybox", &st);
        skyboxShader.setUniform("skybox2", &st2);
        skyboxShader.setUniform("matrix", mat);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

        ogl::Performance::render();
        ogl::Window::refresh();
    }

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "after: " << err << "\n";
    }

    return 0;
}
