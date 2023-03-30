#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <glew.h>
#include "3dMath/3dMath.h"

struct Vertex {
    m3d::vec3f pos;
    m3d::vec2<GLushort> texCoord;
    m3d::vec3<GLshort> normal;
};

struct OBJIndex {
    unsigned vertexNum = 0, textureNum = 0, normalNum = 0;
};

struct OBJMeshPart { // maybe move to obj class as private field
    std::vector<OBJIndex> indices;
    std::string materialName;
    bool hasMaterial = false;
};

class OBJMesh {
private:
    std::string objectName, mtllibFile, filename;

    std::vector<m3d::vec3f> geomVertices;
    std::vector<m3d::vec2f> textureVertices;
    std::vector<m3d::vec3f> vertexNormals;

    std::vector<OBJMeshPart> meshParts;

    bool hasVertexNormales = false;
    bool hasTextureVertices = false;

    std::vector<float> parseCoordLine(std::string line);

    void parseFaceLine(std::string line, OBJMeshPart& p);

    std::string getLineName(std::string& line);

public:
    std::string& getObjectName();

    bool loadFromFile(std::string filename);

    GLuint passToGPU();

    void printInfo();
};