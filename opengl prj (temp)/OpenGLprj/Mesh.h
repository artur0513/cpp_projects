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

namespace OBJ {
    struct Index {
        unsigned vertexNum = 0, textureNum = 0, normalNum = 0;
    };

    struct MeshPart {
        size_t firstIndex = 0, numOfIndices = 0;
        std::string materialName;
        bool hasMaterial = false;
    };

    class Mesh {
    private:
        std::string objectName, mtllibFile, filename;

        // Следующие векторы хранят инфу в том виде, как она есть в исходном OBJ файле 
        std::vector<m3d::vec3f> geomVertices;
        std::vector<m3d::vec2f> textureVertices;
        std::vector<m3d::vec3f> vertexNormals;
        std::vector<Index> indices;
        
        // Следующие векторы хранят инфу в том виде, в котором будут отправлены видеокарте
        std::vector<MeshPart> meshParts; // Тут нужно разделение на обьекты и их части
        std::vector<Vertex> VBOvertices;
        std::vector<unsigned> EBOindices;

        bool hasVertexNormales = false;
        bool hasTextureVertices = false;

        std::vector<float> parseCoordLine(std::string line);
        void parseFaceLine(std::string line);
        std::string getLineName(std::string& line);
        void passToGPU();
    public:
        unsigned VBO, VAO, EBO;
        std::string& getObjectName();
        bool loadFromFile(std::string filename);
        ~Mesh();
    };

}