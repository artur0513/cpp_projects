#pragma once
#include "Texture.h"
#include "Shader.h"

namespace ogl::Resources {

	//Shader* getShader();
	Texture* getTexture(std::string name);
	//Cubemap* getCubemap();

	void freeMemory();
}

