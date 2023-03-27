#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <glew.h>
#include "3dMath/3dMath.h"

struct OBJMeshPart { // maybe move to obj class as private field
    std::vector<m3d::vec3i> faces;
    std::vector<m3d::vec3i> textureCoords;
    std::vector<m3d::vec3i> normals;
    std::string materialName;
    bool hasMaterial = false;
};

class OBJMesh {
private:
    std::string objectName, mtllibFile;

    std::vector<m3d::vec3f> geomVertices;
    std::vector<m3d::vec2f> textureVertices;
    std::vector<m3d::vec3f> vertexNormals;

    std::vector<OBJMeshPart> meshParts;

    bool hasVertexNormales = false;
    bool hasTextureVertices = false;

    std::vector<float> parseCoordLine(std::string line);

    void parseFaceLine(std::string line, m3d::vec3i& face, m3d::vec3i& tex, m3d::vec3i& normal);

    std::string getLineName(std::string& line);

public:
    std::string& getObjectName();

    bool loadFromFile(std::string filename);

    GLuint passToGPU();

    void printInfo();
};