#include "Material.h"
#include <deque>

std::deque<ogl::Material> materials;

std::string getLineName(std::string& line) { // copy from mesh.cpp
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


ogl::Material* findMaterial(std::string& name) {
    for (auto& t : materials) {
        if (t.name == name)
            return &t;
    }
    return nullptr;
}

ogl::Material ogl::Resources::getMaterial(std::string materialName) {
    std::cout << "Requested material: " << materialName << "\n";
    Material* mat = findMaterial(materialName);
    if (mat != nullptr)
        return *mat;
    std::cout << "Material not found!\n";
    return Material();
}

bool ogl::Resources::loadMaterials(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can`t open file and load materials: " << filename << "\n";
        file.close();
        return false;
    }

    std::string line, name;
    ogl::Material* currentMat = nullptr;
    while (getline(file, line)) {
        name = getLineName(line);

        if (name == "newmtl") {
            ogl::Material newmtl;
            newmtl.name = line;
            std::cout << "loading material: " << line << "\n";

            if (findMaterial(name) == nullptr) {
                materials.push_back(newmtl);
                currentMat = &materials[materials.size() - 1];
            }
        }
        else if (name == "map_Kd" && currentMat != nullptr) {
            currentMat->diffuseTexture = ogl::Resources::getTexture(line);
        }

    }
}
