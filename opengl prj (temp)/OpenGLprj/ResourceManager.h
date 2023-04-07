#pragma once
#include <unordered_map>
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"

// Delete all other managers, use only this
// Or maybe just use funtions + namespace here
class ResourceManager {
private:
	std::unordered_map<std::string, ogl::Texture*> textures;
	std::unordered_map<std::string, ogl::Shader*> shaders;

	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, VertexDataHandle> meshes;

public:
	ResourceManager();

	ogl::Texture* getTexture();
	ogl::Shader* getShader();
	Material getMaterial();

	~ResourceManager();
};

