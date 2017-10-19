#include "ShaderProgram.hpp"
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>



ShaderProgram::ShaderProgram()
{
}


ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(mProgramID);
}


//--------------------------------Public Functions--------------------------------

void ShaderProgram::Init(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	// Load the shaders.
	GLuint vertexShaderID = LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
	GLuint fragmentShaderID = LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	// Link the shader program.
	mProgramID = glCreateProgram();
	glAttachShader(mProgramID, vertexShaderID);
	glAttachShader(mProgramID, fragmentShaderID);
	glLinkProgram(mProgramID);

	// Check that the shader program linked correctly.
	GLint linkSuccessful = GL_FALSE;
	glGetProgramiv(mProgramID, GL_LINK_STATUS, &linkSuccessful);
	if (linkSuccessful != GL_TRUE)
	{
		int infoLogLength = 0;
		glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(mProgramID, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error compiling shader program : " << std::endl << &infoLog[0] << std::endl;
	}
}

void ShaderProgram::Use() const
{
	glUseProgram(mProgramID);
}

void ShaderProgram::CreateUniformBuffer(std::string name, GLsizeiptr size, int index)
{
	glGenBuffers(1, &mUniformBuffers[name]);
	glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers[name]);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, mUniformBuffers[name]);
	glUniformBlockBinding(mProgramID, glGetUniformBlockIndex(mProgramID, name.c_str()), index);
}

void ShaderProgram::SetUniformBuffer(std::string name, const void * data, GLsizeiptr size)
{
	glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers[name]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void ShaderProgram::SetTextureUniform(GLuint textureID, std::string uniformName)
{
	// Binding the texture to a uniform variable.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(glGetUniformLocation(mProgramID, uniformName.c_str()), 0);
}


//--------------------------------Private Functions--------------------------------

GLuint ShaderProgram::LoadShader(std::string shaderPath, GLuint shaderType)
{
	// Create the shader object.
	GLuint shaderID = glCreateShader(shaderType);

	// Read the shader code from its file.
	std::string shaderString = tygra::createStringFromFile(shaderPath);
	auto shaderCString = shaderString.c_str();

	// Compile the shader.
	glShaderSource(shaderID, 1, &shaderCString, NULL);
	glCompileShader(shaderID);

	// Check the shader compiled correctly.
	GLint compileSuccessful = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileSuccessful);
	if (compileSuccessful != GL_TRUE)
	{
		int infoLogLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error compiling shader '" << shaderPath << "' : " << std::endl << &infoLog[0] << std::endl;
	}

	return shaderID;
}