#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "3dMath/3dMath.h"

struct Material {
    //std::vector<std::pair<std::string>>
};

struct OBJMeshPart { // maybe move to obj class as private field
    std::vector<m3d::vec3i> faces;
    Material* mat;
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

    template<class T>
    std::vector<T> parseCoordLine(std::string line) { // works OK!
        static_assert(std::is_arithmetic<T>::value, "This function can be used only with arithmetic types");
        size_t i = 0;
        std::vector<T> numbers;
        while (i < line.size()) {
            if (isdigit(line[i])) {
                size_t nextSpace = line.find(' ', i);
                nextSpace = (nextSpace == std::string::npos) ? line.size() : nextSpace;

                if constexpr (std::is_floating_point<T>::value)
                    numbers.push_back(std::stof(line.substr(i, nextSpace - i)));
                else
                    numbers.push_back(std::stoi(line.substr(i, nextSpace - i)));

                i = nextSpace + 1;
            }
            else
                i++;

        }
        return numbers;
    }

    m3d::vec3i parseFaceLine(std::string line) {

    }

    std::string getLineName(std::string& line) { // works OK!
        size_t first = line.find_first_not_of(" "), last = line.find(" ", first);
        last = (last == std::string::npos) ? line.size() : last;

        if (first < line.size()) {
            std::string ret = line.substr(first, last - first);
            line.erase(0, last + 1);
            return ret;
        }
        else
            return "";
    }

public:
    std::string& getObjectName() { return objectName; }

    bool loadFromFile(std::string filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Can`t open file and load mesh: " << filename << "\n";
            file.close();
            return false;
        }

        std::string line, name;
        while (getline(file, line)) {
            name = getLineName(line);
            if (name == "#")
                continue;
            else if (name == "o")
                objectName = line;
            else if (name == "v") {
                auto coords = parseCoordLine<float>(line);
                geomVertices.push_back(m3d::vec3f(coords[0], coords[1], coords[2]));
            }
            else if (name == "vt") {
                auto coords = parseCoordLine<float>(line);
                textureVertices.push_back(m3d::vec2f(coords[0], coords[1]));
            }
            else if (name == "vn") {
                auto coords = parseCoordLine<float>(line);
                vertexNormals.push_back(m3d::vec3f(coords[0], coords[1], coords[2]));
            }
            else if (name == "usemtl") {

            }
            else if (name == "f") {

            }
            
        }

    }
};

int main()
{
    OBJMesh m;
    m.loadFromFile("1.obj");
    std::cout << m.getObjectName();
}
