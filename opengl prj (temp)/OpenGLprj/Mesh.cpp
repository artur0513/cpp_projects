#include "Mesh.h"
#include <unordered_set>

std::vector<float> OBJMesh::parseCoordLine(std::string line) { // works OK!
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

void OBJMesh::parseFaceLine(std::string line, OBJMeshPart& p) {
    // Бля, сделать по нормальному, че за кал, можно же в линию все хранить, смотри passToGPU, которой тут вообще не должно быть

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
        p.indices.insert(p.indices.end(), { OBJIndex(numbers[0], numbers[1], numbers[2] ), OBJIndex(numbers[3], numbers[4], numbers[5]), OBJIndex(numbers[6], numbers[7], numbers[8]) });
    }
    else if (hasVertexNormales && !hasTextureVertices) {
        p.indices.insert(p.indices.end(), { OBJIndex(numbers[0], 0, numbers[1]), OBJIndex(numbers[2], 0, numbers[3]), OBJIndex(numbers[4], 0, numbers[5]) });
    }
    else if (!hasVertexNormales && hasTextureVertices) {
        p.indices.insert(p.indices.end(), { OBJIndex(numbers[0], numbers[1], 0), OBJIndex(numbers[2], numbers[3], 0), OBJIndex(numbers[4], numbers[5], 0) });
    }
    else {
        p.indices.insert(p.indices.end(), { OBJIndex(numbers[0], 0, 0), OBJIndex(numbers[1], 0, 0), OBJIndex(numbers[2],0, 0) });
    }
}

std::string OBJMesh::getLineName(std::string& line) { // works OK!
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

std::string& OBJMesh::getObjectName() { return objectName; }

bool OBJMesh::loadFromFile(std::string _filename) {
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
            mtllibFile = line;
        }
        else if (name == "usemtl") {
            if (meshParts.size() == 0) {
                hasVertexNormales = vertexNormals.size();
                hasTextureVertices = textureVertices.size();
            }

            meshParts.push_back(OBJMeshPart());
            (meshParts.end() - 1)->materialName = line;
            (meshParts.end() - 1)->hasMaterial = true;
        }
        else if (name == "f") {
            if (meshParts.size() == 0) {
                hasVertexNormales = vertexNormals.size();
                hasTextureVertices = textureVertices.size();
                meshParts.push_back(OBJMeshPart());
            }

            parseFaceLine(line, *(meshParts.end()  - 1));
        }

    }
}

void OBJMesh::printInfo() {
    std::cout << "Filename: " << filename << "\n";
    std::cout << "Object name: " << objectName << "  Material file: " << mtllibFile << "\n";
    std::cout << "hasVertexNormales: " << hasVertexNormales << "  hasTextureVertices: " << hasTextureVertices << "\n";

    std::cout << "geomVertices: " << "\n";
    for (auto& v : geomVertices)
        std::cout << v << "\n";

    if (hasTextureVertices) {
        std::cout << "textureVertices: " << "\n";
        for (auto& v : textureVertices)
            std::cout << v << "\n";
    }

    if (hasVertexNormales) {
        std::cout << "vertexNormals: " << "\n";
        for (auto& v : vertexNormals)
            std::cout << v << "\n";
    }

    std::cout << "meshParts: " << "\n";
    for (auto& p : meshParts) {
        std::cout << "   hasMaterial: " << p.hasMaterial << "\n";
        std::cout << "   materialName: " << p.materialName << "\n";

        std::cout << "   indices: " << "\n";
        for (auto& v : p.indices)
            std::cout << "   " << v.vertexNum << " " << v.textureNum << " " << v.normalNum << "\n";
        std::cout << std::endl;
    }
}

GLuint OBJMesh::passToGPU() {
    // Формат - координаты, текстурные координаты, нормали
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    auto hash = [](const OBJIndex& x) {
        return (size_t)0;
    };

    auto equal = [](const OBJIndex& x, const OBJIndex& y) {
        return (x.vertexNum == y.vertexNum && x.textureNum == y.textureNum && x.normalNum == y.normalNum);
    };

    std::unordered_set<OBJIndex, decltype(hash), decltype(equal)> rawIndices;
    //std::vector<size_t> meshPartsDelimiter;

    for (auto& meshPart : meshParts) {
        for (auto& index : meshPart.indices) {
            rawIndices.insert(index);
        }
    }

    //std::cout << "============================================\n";
    for (auto& uniqueIndex : rawIndices) {
        Vertex v;
        v.pos = geomVertices[uniqueIndex.vertexNum];
        if (hasTextureVertices)
            v.texCoord = textureVertices[uniqueIndex.textureNum] * 65536.f;
        if (hasVertexNormales) {
            m3d::vec3f norm = m3d::normalize(vertexNormals[uniqueIndex.normalNum]);
            v.normal = norm * 32767.f;
        }
        vertices.push_back(v);
        //std::cout << v.pos << " " << v.texCoord << " " << v.normal << "\n";
    }

    for (auto& meshPart : meshParts) {
        for (auto& index : meshPart.indices) {
            indices.push_back(std::distance(rawIndices.begin(), rawIndices.find(index)));
        }
    }

    //for (int i = 0; i < indices.size(); i += 3) {
    //  std::cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << "\n";
    //}

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    // Координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибуты текстурных координат
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
    glEnableVertexAttribArray(1);

    // Нормали
    glVertexAttribPointer(2, 3, GL_SHORT, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);

    geomVertices.clear();
    textureVertices.clear();
    vertexNormals.clear();
    meshParts.clear();
    
    return VAO;
}