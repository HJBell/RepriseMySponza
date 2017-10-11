#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>

struct DirectionalLight
{
	glm::vec3 direction;
	float PADDING1;
	glm::vec3 intensity;
	float PADDING2;
};

struct PointLight
{
	glm::vec3 position;
	float range;
	glm::vec3 intensity;
	float PADDING0;
};

struct SpotLight
{
	glm::vec3 position;
	float range;
	glm::vec3 intensity;
	float angle;
	glm::vec3 direction;
	float PADDING0;
};

struct PerFrameUniforms
{
	glm::vec3 cameraPos;
	float PADDING0;
	glm::vec3 ambientIntensity;
	float PADDING1;
	DirectionalLight directionalLights[32];
	int directionalLightCount;
	float PADDING2[3];
	PointLight pointLights[32];
	int pointLightCount;
	float PADDING3[3];
	SpotLight spotLights[32];
	int spotLightCount;
};

struct PerModelUniforms
{
	glm::mat4 mvpXform;
	glm::mat4 modelXform;
	glm::vec3 diffuse;
	float shininess;
	glm::vec3 specular;
};

struct MeshGL
{
	GLuint positionVBO;
	GLuint normalVBO;
	GLuint textureCoordVBO;
	GLuint elementVBO;
	GLuint vao;
	int elementCount;

	MeshGL() :
		positionVBO(0),
		elementVBO(0),
		vao(0),
		elementCount(0) {}
};

class MyView : public tygra::WindowViewDelegate
{
public: // Functions.
    MyView();
    ~MyView();
    void setScene(const sponza::Context * sponza);
	void RecompileShaders();

private: // Members.
	const sponza::Context * scene_;
	GLuint shaderProgram;
	std::map<sponza::MeshId, MeshGL> meshes;
	std::map<std::string, GLuint> textures;

	GLuint perFrameUniformsUBO;
	GLuint perModelUniformsUBO;

	std::string	vertexShaderPath = "resource:///reprise_vs.glsl";
	std::string	fragmentShaderPath = "resource:///reprise_fs.glsl";
	int maxLightsPerType = 32;

private: // Functions.
    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	GLuint loadShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath) const;
	GLuint loadShader(std::string shaderPath, GLuint shaderType) const;
	void loadMeshData();
	void loadTexture(std::string name);
	bool setShaderTexture(std::string name, GLuint shaderProgram, std::string targetName, GLenum activeTexture, int index);
	void createUniformBufferObjects();
};

