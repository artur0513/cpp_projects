#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <glew.h>
#include "3dMath/3dMath.h"
#include "Texture.h"
#include "Material.h"

namespace ogl {

    struct VertexDataHandle {
        GLuint VAO = 0, VBO = 0, EBO = 0;
    };

    struct Vertex {
        m3d::vec3f pos;
        m3d::vec2f texCoord;
        m3d::vec3<GLshort> normal;
    };

    struct MeshPart {
        size_t firstIndex = 0, numOfIndices = 0;
        ogl::Material material;
        bool hasMaterial = false;
    };

}

namespace OBJ {
    std::string getLineName(std::string& line);

    struct Index {
        unsigned vertexNum = 0, textureNum = 0, normalNum = 0;
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
        std::vector<ogl::Vertex> VBOvertices;
        std::vector<unsigned> EBOindices;

        bool hasVertexNormales = false;
        bool hasTextureVertices = false;

        std::vector<float> parseCoordLine(std::string line);
        void parseFaceLine(std::string line);
        void passToGPU();
    public:
        std::vector<ogl::MeshPart> meshParts; // Тут нужно разделение на обьекты и их части

        ogl::VertexDataHandle vdh;
        std::string& getObjectName();
        bool loadFromFile(std::string filename);
        ~Mesh();
    };

}