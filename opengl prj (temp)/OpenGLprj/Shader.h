#pragma once
#include <string>
#include "Texture.h"
#include "Cubemap.h"
#include "Mesh.h"
#include "3dMath/3dMath.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <glew.h>

namespace ogl {

	// Шейдер вроде бы работает как надо
	class Shader {
	private:
		//static Shader* activeShader; // Шейдер, который в данный момент используется

		std::string vertexPath, fragmentPath;
		std::unordered_map<std::string, GLint> uniforms; // Чтобы не вызывать glGetUniformLocation каждый раз

		std::unordered_map<GLint, Texture*> textureTable; // Таблица со всеми текстурами, которые будут передаваться в uniform-ы в функции bindTextures()
		std::unordered_map<GLint, Cubemap*> cubemapTable;
		GLuint id = 0;

		GLint getMaxTextureUnits();
		GLint getUniformLocation(const std::string& name);
	public:
		GLint maxTextureUnits = getMaxTextureUnits();
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
		void setUniform(const Material& mat);
		//add more uniforms if needed

		void bindTextures();
		void printInfo();
		~Shader();
	};

	class ShaderManager {
	private:
		std::unordered_map<std::string, Shader*> shaders;

		ShaderManager() {};
		ShaderManager(const ShaderManager& r) {};
		ShaderManager operator=(const ShaderManager& r) {};
		ShaderManager(ShaderManager&& r) noexcept {};
		ShaderManager& operator=(const ShaderManager&& r) noexcept {};
	public:
		static ShaderManager* getInstance();

		Shader* getShader(std::string vertexPath, std::string fragmentPath);

		~ShaderManager();
	};

}

