#pragma once
#include "Cubemap.h"
#include "Shader.h"
#include "Camera.h"

namespace ogl::Skybox {

	void initSkybox();

	void setCamera(ogl::Camera& cam);
	
	void setSkyboxCubemap(ogl::Cubemap& c);

	void renderSkybox();

}

