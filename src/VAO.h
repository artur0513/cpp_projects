#pragma once
#include "3dMath/3dMath.h"
#include <GL/glew.h>
#include <vector>

namespace ogl {

    struct Vertex {
        m3d::vec3f pos;
        m3d::vec2f texCoord;
        m3d::vec3f normal;
    };

    class VAO { // Vertex Array Object
    private:
        GLuint VAOid = 0, VBOid = 0, EBOid = 0;
        GLsizei numVertices = 0;

        VAO(const VAO& v) = delete;
        VAO operator=(const VAO& v) = delete;
        VAO(VAO&& t) = delete;
        VAO& operator=(VAO&& t) = delete;

        void bind();
    public:
        VAO();

        void create(std::vector<ogl::Vertex>& vertices, std::vector<unsigned>& indices);

        void draw();

        ~VAO();
    };

}
