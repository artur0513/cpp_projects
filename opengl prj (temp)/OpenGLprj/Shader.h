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
	std::string vertexPath, fragmentPath;
	std::map<std::string, GLint> uniforms; // Чтобы не вызывать glGetUniformLocation каждый раз
	std::map<GLint, Texture*> textureTable; // Таблица со всеми текстурами, которые будут передаваться в uniform-ы в функции bindTextures()
	GLuint id = 0;

	GLint getMaxTextureUnits();
	GLint getUniformLocation(const std::string& name);
public:
	GLint maxTextureUnits = getMaxTextureUnits();
	GLuint loadFromFile(const std::string& _vertexPath, const std::string& _fragmentPath);
	GLuint getId();
	void use();

	void setUniform(const std::string& name, int v);
	void setUniform(const std::string& name, float v);
	void setUniform(const std::string& name, Texture& v);
	void setUniform(const std::string& name, Texture* v);
	void setUniform(const std::string& name, m3d::vec2f& v);
	void setUniform(const std::string& name, m3d::vec3f& v);
	void setUniform(const std::string& name, m3d::vec4f& v);
	void setUniform(const std::string& name, m3d::mat4f& v);
	//add more uniforms if needed

	void bindTextures();
	void printInfo();
	~Shader();
};

