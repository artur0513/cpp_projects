#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <glew.h>

class Shader {
private:
	GLuint id;

	GLuint loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	void use();

	//set uniform functions...
};

