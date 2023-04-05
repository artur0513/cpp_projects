#include "Mesh.h"
#include <unordered_set>

MaterialLibrary* MaterialLibrary::getInstance() {
    static MaterialLibrary instance;
    return &instance;
}

Material* MaterialLibrary::getMaterial(std::string materialName) {
    auto mat_iterator = materials.find(materialName);
    if (mat_iterator != materials.end())
        return &mat_iterator->second;

    return nullptr;
}

bool OBJ::loadMaterials(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can`t open file and load materials: " << filename << "\n";
        file.close();
        return false;
    }

    MaterialLibrary* matlib = MaterialLibrary::getInstance();

    std::string line, name, currentMat;
    while (getline(file, line)) {
        name = getLineName(line);

        if (name == "newmtl") {
            matlib->materials.emplace(line, Material());
            currentMat = line;
        }
        else if (name == "map_Kd") {
            Texture *texture = new Texture(); // Memory leak, besause no texture manager now
            texture->loadFromFile(line);
            matlib->materials[currentMat].diffuseTexture = texture;
        }

    }

}

std::vector<float> OBJ::Mesh::parseCoordLine(std::string line) { // works OK!
    size_t i = 0;
    std::vector<float> numbers;
    while (i < line.size()) {
        if (isdigit(line[i]) || line[i] == '-') {
            size_t nextSpace = line.find(' ', i);
            nextSpace = (nextSpace == std::string::npos) ? line.size() : nextSpace;
            numbers.push_back(std::stof(line.substr(i, nextSpace - i)));
            
            i = nextSpace + 1;
        }
        else
            i++;
    }
    return numbers;
}

void OBJ::Mesh::parseFaceLine(std::string line) {
    unsigned numbers[9];
    size_t start = 0, end = line.find_first_of("/ "), i = 0;

    while (end != std::string::npos && i < 9) {
        if (end - start > 0) {
            try {
                numbers[i] = std::stoi(line.substr(start, end - start)) - 1;
                i++;
            }
            catch (std::invalid_argument) {};
        }
        start = end + 1;
        end = line.find_first_of("/ ", start);
    }
    try {
        numbers[i] = std::stoi(line.substr(start)) - 1;
        i++;
    }
    catch (std::invalid_argument) {};    
    
    if (hasVertexNormales && hasTextureVertices) {
        indices.insert(indices.end(), { OBJ::Index(numbers[0], numbers[1], numbers[2] ), OBJ::Index(numbers[3], numbers[4], numbers[5]), OBJ::Index(numbers[6], numbers[7], numbers[8]) });
    }
    else if (hasVertexNormales && !hasTextureVertices) {
        indices.insert(indices.end(), { OBJ::Index(numbers[0], 0, numbers[1]), OBJ::Index(numbers[2], 0, numbers[3]), OBJ::Index(numbers[4], 0, numbers[5]) });
    }
    else if (!hasVertexNormales && hasTextureVertices) {
        indices.insert(indices.end(), { OBJ::Index(numbers[0], numbers[1], 0), OBJ::Index(numbers[2], numbers[3], 0), OBJ::Index(numbers[4], numbers[5], 0) });
    }
    else {
        indices.insert(indices.end(), { OBJ::Index(numbers[0], 0, 0), OBJ::Index(numbers[1], 0, 0), OBJ::Index(numbers[2],0, 0) });
    }
}

std::string OBJ::getLineName(std::string& line) { // works OK!
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

void OBJ::Mesh::passToGPU() {
    glGenVertexArrays(1, &vdh.VAO);
    glGenBuffers(1, &vdh.VBO);
    glGenBuffers(1, &vdh.EBO);

    glBindVertexArray(vdh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vdh.VBO);
    glBufferData(GL_ARRAY_BUFFER, VBOvertices.size() * sizeof(Vertex), VBOvertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vdh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBOindices.size() * sizeof(unsigned), EBOindices.data(), GL_STATIC_DRAW);

    // Координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибуты текстурных координат
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
    glEnableVertexAttribArray(1);

    // Нормали
    glVertexAttribPointer(2, 3, GL_SHORT, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
}

std::string& OBJ::Mesh::getObjectName() { return objectName; }

bool OBJ::Mesh::loadFromFile(std::string _filename) {
    filename = _filename;
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
            auto coords = parseCoordLine(line);
            geomVertices.push_back(m3d::vec3f(coords[0], coords[1], coords[2]));
        }
        else if (name == "vt") {
            auto coords = parseCoordLine(line);
            textureVertices.push_back(m3d::vec2f(coords[0], coords[1]));
        }
        else if (name == "vn") {
            auto coords = parseCoordLine(line);
            vertexNormals.push_back(m3d::vec3f(coords[0], coords[1], coords[2]));
        }
        else if (name == "mtllib") {
            std::filesystem::path p(filename);
            mtllibFile = p.parent_path().string() + "\\" + line;
            loadMaterials(mtllibFile);
        }
        else if (name == "usemtl") {
            if (meshParts.size() == 0) {
                hasVertexNormales = vertexNormals.size();
                hasTextureVertices = textureVertices.size();
            }

            meshParts.push_back(OBJ::MeshPart());
            (meshParts.end() - 1)->firstIndex = indices.size();
            (meshParts.end() - 1)->materialName = line;
            (meshParts.end() - 1)->material = MaterialLibrary::getInstance()->getMaterial(line);
            (meshParts.end() - 1)->hasMaterial = true;
        }
        else if (name == "f") {
            if (meshParts.size() == 0) {
                hasVertexNormales = vertexNormals.size();
                hasTextureVertices = textureVertices.size();
                meshParts.push_back(OBJ::MeshPart());
            }

            parseFaceLine(line);
            (meshParts.end() - 1)->numOfIndices += 3;
        }

    }

    // Переводим в понятный формат - координаты, текстурные координаты, нормали
    auto hash = [](const OBJ::Index& x) {
        return (size_t)0;
    };

    auto equal = [](const OBJ::Index& x, const OBJ::Index& y) {
        return (x.vertexNum == y.vertexNum && x.textureNum == y.textureNum && x.normalNum == y.normalNum);
    };

    std::unordered_set<OBJ::Index, decltype(hash), decltype(equal)> rawIndices;

    for (auto& index : indices) {
        rawIndices.insert(index);
    }

    for (auto& uniqueIndex : rawIndices) {
        Vertex v;
        v.pos = geomVertices[uniqueIndex.vertexNum];
        if (hasTextureVertices)
            v.texCoord = textureVertices[uniqueIndex.textureNum] * 65536.f;
        if (hasVertexNormales) {
            m3d::vec3f norm = m3d::normalize(vertexNormals[uniqueIndex.normalNum]);
            v.normal = norm * 32767.f;
        }
        VBOvertices.push_back(v);
    }

    for (auto& index : indices) {
        EBOindices.push_back(std::distance(rawIndices.begin(), rawIndices.find(index)));
    }

    geomVertices.clear();
    textureVertices.clear();
    vertexNormals.clear();
    indices.clear();

    passToGPU();
    return true;
}

OBJ::Mesh::~Mesh() {
    glDeleteBuffers(1, &vdh.EBO);
    glDeleteBuffers(1, &vdh.VBO);
    glDeleteVertexArrays(1, &vdh.VAO);
}