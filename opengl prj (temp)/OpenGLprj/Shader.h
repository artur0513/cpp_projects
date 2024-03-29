﻿#pragma once
#include "Material.h"
#include <string>
#include <unordered_map>
#include "Texture.h"
#include "Cubemap.h"
#include "3dMath/3dMath.h"
#include <fstream>
#include <iostream>
#include <glew.h>

namespace ogl {

	struct Material;

	// Шейдер вроде бы работает как надо
	class Shader {
	private:
		std::string vertexPath, fragmentPath;
		std::unordered_map<std::string, GLint> uniforms; // Чтобы не вызывать glGetUniformLocation каждый раз

		std::unordered_map<GLint, Texture*> textureTable; // Таблица со всеми текстурами, которые будут передаваться в uniform-ы в функции bindTextures()
		std::unordered_map<GLint, Cubemap*> cubemapTable;
		GLuint id = 0;

		GLint getUniformLocation(const std::string& name); // pretty useless after changing loading function, maybe delete?
	public:
		static GLint maxTextureUnits;
		[[nodiscard]] GLuint loadFromMemory(const std::string& vertexCode, const std::string& fragmentCode);
		[[nodiscard]] GLuint loadFromFile(const std::string& _vertexPath, const std::string& _fragmentPath);
		GLuint getId();
		void use();

		void setUniform(const std::string& name, int v);
		void setUniform(const std::string& name, float v);
		void setUniform(const std::string& name, Texture& v);
		void setUniform(const std::string& name, Cubemap& v);
		void setUniform(const std::string& name, m3d::vec2f& v);
		void setUniform(const std::string& name, m3d::vec3f& v);
		void setUniform(const std::string& name, m3d::vec4f& v);
		void setUniform(const std::string& name, const m3d::mat4f& v);
		void setUniform(const ogl::Material& mat);
		//add more uniforms if needed

		const std::string& getVertexPath() const;
		const std::string& getFragmentPath() const;

		void bindTextures();
		void printInfo();
		~Shader();
	};
}

