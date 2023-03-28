#pragma once
#include <string>
#include "Texture.h"
#include "3dMath/3dMath.h"
#include <fstream>
#include <iostream>
#include <map>
#include <glew.h>

// Шейдер вроде бы работает как надо
class Shader {
private:
	std::map<std::string, GLint> uniforms; // Чтобы не вызывать glGetUniformLocation каждый раз
	GLuint id = 0;

	GLint getUniformLocation(const std::string& name);
public:
	GLuint loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
	void use();

	void setUniform(const std::string& name, int v);
	void setUniform(const std::string& name, float v);
	void setUniform(const std::string& name, Texture& v);
	void setUniform(const std::string& name, m3d::vec2f& v);
	void setUniform(const std::string& name, m3d::vec3f& v);
	void setUniform(const std::string& name, m3d::vec4f& v);
	void setUniform(const std::string& name, m3d::mat4f& v);
	//add more uniforms if needed

	~Shader();
};

