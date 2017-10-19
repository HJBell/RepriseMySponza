#pragma once

#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>



class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	void Use() const;
	void Init(std::string vertexShaderPath, std::string fragmentShaderPath);
	void CreateUniformBuffer(std::string name, GLsizeiptr size, int index);
	void SetUniformBuffer(std::string name, const void * data, GLsizeiptr size);
	void SetTextureUniform(GLuint textureID, std::string uniformName);

private:
	GLuint mProgramID;
	std::unordered_map<std::string, GLuint> mUniformBuffers;

	GLuint LoadShader(std::string shaderPath, GLuint shaderType);
};

