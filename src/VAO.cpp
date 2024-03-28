#include "VAO.h"

ogl::VAO::VAO() {}

void ogl::VAO::create(std::vector<ogl::Vertex>& vertices, std::vector<unsigned>& indices) {
    glGenVertexArrays(1, &VAOid);
    glGenBuffers(1, &VBOid);
    glGenBuffers(1, &EBOid);

    glBindVertexArray(VAOid);

    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ogl::Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, sizeof(ogl::Vertex::pos)/4, GL_FLOAT, GL_FALSE, sizeof(ogl::Vertex), (void*)(offsetof(ogl::Vertex, pos)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, sizeof(ogl::Vertex::texCoord)/4, GL_FLOAT, GL_FALSE, sizeof(ogl::Vertex), (void*)(offsetof(ogl::Vertex, texCoord)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, sizeof(ogl::Vertex::normal)/4, GL_SHORT, GL_TRUE, sizeof(ogl::Vertex), (void*)(offsetof(ogl::Vertex, normal)));
    glEnableVertexAttribArray(2);

    numVertices = indices.size();
}

void ogl::VAO::bind() {
    glBindVertexArray(VAOid);
}

void ogl::VAO::draw() {
    glBindVertexArray(VAOid);
    glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, 0);
}

ogl::VAO::~VAO() {
    glDeleteBuffers(1, &EBOid); // ignores no existing buffers
    glDeleteBuffers(1, &VBOid);
    glDeleteVertexArrays(1, &VAOid);
}
