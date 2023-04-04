#pragma once
#include <unordered_map>
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"

// Delete all other managers, use only this

class ResourceManager {
private:
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, Shader*> shaders;

	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, VertexDataHandle> meshes;

public:
	ResourceManager();

	Texture* getTexture();
	Shader* getShader();
	Material getMaterial();

	~ResourceManager();
};

