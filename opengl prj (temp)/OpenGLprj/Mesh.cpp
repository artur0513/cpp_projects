#include "Mesh.h"

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

void OBJMesh::parseFaceLine(std::string line, m3d::vec3i& face, m3d::vec3i& tex, m3d::vec3i& normal) {
    // Ѕл€, сделать по нормальному, че за кал, можно же в линию все хранить, смотри passToGPU, которой тут вообще не должно быть

    int numbers[9];
    size_t start = 0, end = line.find_first_of("/ "), i = 0;

    while (end != std::string::npos && i < 9) {
        if (end - start > 0) {
            try {
                numbers[i] = std::stoi(line.substr(start, end - start));
                i++;
            }
            catch (std::invalid_argument) {};
        }
        start = end + 1;
        end = line.find_first_of("/ ", start);
    }
    try {
        numbers[i] = std::stoi(line.substr(start));
        i++;
    }
    catch (std::invalid_argument) {};
    
    if (hasVertexNormales && hasTextureVertices) {
        face = { numbers[0], numbers[3], numbers[6] };
        tex = { numbers[1], numbers[4], numbers[7] };
        normal = { numbers[2], numbers[5], numbers[8] };
    }
    else if (hasVertexNormales && !hasTextureVertices) {
        face = { numbers[0], numbers[2], numbers[4] };
        normal = { numbers[1], numbers[3], numbers[5] };
    }
    else if (!hasVertexNormales && hasTextureVertices) {
        face = { numbers[0], numbers[2], numbers[4] };
        tex = { numbers[1], numbers[3], numbers[5] };
    }
    else {
        face = { numbers[0], numbers[1], numbers[2] };
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

bool OBJMesh::loadFromFile(std::string filename) {
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

            m3d::vec3i face, tex, normal;
            parseFaceLine(line, face, tex, normal);
            (meshParts.end() - 1)->faces.push_back(face);
            (meshParts.end() - 1)->textureCoords.push_back(tex);
            (meshParts.end() - 1)->normals.push_back(normal);
        }

    }
}

void OBJMesh::printInfo() {
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

        std::cout << "   faces: " << "\n";
        for (auto& v : p.faces)
            std::cout << "   " << v << "\n";

        if (hasTextureVertices) {
            std::cout << "   textureVertices: " << "\n";
            for (auto& v : p.textureCoords)
                std::cout << "   " << v << "\n";
        }

        if (hasVertexNormales) {
            std::cout << "   vertexNormals: " << "\n";
            for (auto& v : p.normals)
                std::cout << "   " << v << "\n";
        }
        std::cout << std::endl;
    }
}

GLuint OBJMesh::passToGPU() {
    /*
    // ”казание вершин (и буфера(ов)) и настройка вершинных атрибутов
    float vertices[] = {
         // координаты        // цвета            // текстурные координаты
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // верхн€€ права€ вершина
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // нижн€€ права€ вершина
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // нижн€€ лева€ вершина
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // верхн€€ лева€ вершина 
    };
    unsigned int indices[] = {
        0, 1, 3, // первый треугольник
        1, 2, 3  // второй треугольник
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //  оординатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	
    // ÷ветовые атрибуты
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	
    // јтрибуты текстурных координат
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    */
}