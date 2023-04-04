#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <glew.h>
#include "3dMath/3dMath.h"
#include "Texture.h"
#include "Material.h"

struct VertexDataHandle {
    GLuint VAO = 0, VBO = 0, EBO = 0;
};

struct Vertex {
    m3d::vec3f pos;
    m3d::vec2<GLushort> texCoord;
    m3d::vec3<GLshort> normal;
};

namespace OBJ {
    bool loadMaterials(std::string filename);
}

class MaterialLibrary {
protected:
    friend bool OBJ::loadMaterials(std::string filename);

    std::unordered_map<std::string, Material> materials;

    MaterialLibrary() {};
    MaterialLibrary(const MaterialLibrary& r) {};
    MaterialLibrary operator=(const MaterialLibrary& r) {};
    MaterialLibrary(MaterialLibrary&& r) noexcept {};
    MaterialLibrary& operator=(const MaterialLibrary&& r) noexcept {};
public:
    static MaterialLibrary* getInstance();

    Material* getMaterial(std::string materialName);
};

namespace OBJ {
    // переделать всю эту хуету в одну функцию

    std::string getLineName(std::string& line);

    struct Index {
        unsigned vertexNum = 0, textureNum = 0, normalNum = 0;
    };

    struct MeshPart {
        size_t firstIndex = 0, numOfIndices = 0;
        Material* material;
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
        std::vector<Vertex> VBOvertices;
        std::vector<unsigned> EBOindices;

        bool hasVertexNormales = false;
        bool hasTextureVertices = false;

        std::vector<float> parseCoordLine(std::string line);
        void parseFaceLine(std::string line);
        void passToGPU();
    public:
        std::vector<MeshPart> meshParts; // Тут нужно разделение на обьекты и их части

        VertexDataHandle vdh;
        std::string& getObjectName();
        bool loadFromFile(std::string filename);
        ~Mesh();
    };

}