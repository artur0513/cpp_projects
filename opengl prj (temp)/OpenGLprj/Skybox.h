#pragma once
#include "Cubemap.h"
#include "Shader.h"

namespace ogl::Skybox {

	void initSkybox();

	void setCameraMatrix(m3d::mat4f& mat);
	
	void setSkyboxCubemap(ogl::Cubemap& c);

	void renderSkybox();

}

