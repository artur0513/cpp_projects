#include "Resources.h"
#include <deque>

std::deque<ogl::Texture*> textures;
std::deque<ogl::Cubemap*> cubemaps;
std::deque<ogl::Shader*> shaders;

ogl::Texture* findTexture(std::string& name) {
    name = std::filesystem::relative(name).string();
    for (auto t : textures) {
        if (t->getName() == name)
            return t;
    }
    return nullptr;
}

ogl::Texture* ogl::Resources::getTexture(std::string name) {
    ogl::Texture* t = findTexture(name);
    std::cout << "requested texture: " << name << "\n";
    if (t != nullptr)
        return t;

    std::cout << "not found, loading texture: " << name << "\n";
    t = new ogl::Texture();
    if (!t->loadFromFile(name))
        return nullptr;

    std::cout << "succsess!\n";
    textures.push_back(t);
    return t;
}