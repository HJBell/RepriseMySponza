#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>
#include "ShaderProgram.hpp"

#define MAX_LIGHT_COUNT 32
#define MAX_INSTANCE_COUNT 64


//----------------------Structures----------------------

struct FriendsMeshGL
{
	GLuint positionVBO;
	GLuint normalVBO;
	GLuint elementVBO;
	GLuint vao;
	int elementCount;

	FriendsMeshGL() :
		positionVBO(0),
		normalVBO(0),
		elementVBO(0),
		vao(0),
		elementCount(0) {}
};

struct SponzaMeshGL
{
	GLuint positionVBO;
	GLuint normalVBO;
	GLuint textureCoordVBO;
	GLuint elementVBO;
	GLuint vao;
	int elementCount;

	SponzaMeshGL() :
		positionVBO(0),
		normalVBO(0),
		textureCoordVBO(0),
		elementVBO(0),
		vao(0),
		elementCount(0) {}
};

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

struct InstanceData
{
	glm::mat4 mvpXform;
	glm::mat4 modelXform;
	glm::vec3 diffuse;
	float shininess;
	glm::vec3 specular;
	int isShiny;
};


//----------------------Uniform Buffer Blocks----------------------

struct PerFrameUniforms
{
	glm::vec3 cameraPos;
	float PADDING0;
	glm::vec3 ambientIntensity;
};

struct PerModelUniforms
{
	InstanceData instances[MAX_INSTANCE_COUNT];
};

struct DirectionalLightUniforms
{
	DirectionalLight light;
};

struct PointLightUniforms
{
	PointLight light;
};

struct SpotLightUniforms
{
	SpotLight light;
};

struct SkyboxUniforms
{
	glm::mat4 viewProjectionXform;
	glm::vec3 cameraPos;
};


//----------------------MyView----------------------

class MyView : public tygra::WindowViewDelegate
{
public: // Functions.
    MyView();
    ~MyView();
    void setScene(const sponza::Context * sponza);

private: // Members.
	const sponza::Context * scene_;

	ShaderProgram mAmbShaderProgram;
	ShaderProgram mDirShaderProgram;
	ShaderProgram mPointShaderProgram;
	ShaderProgram mSpotShaderProgram;

	std::map<sponza::MeshId, SponzaMeshGL> sponzaMeshes;
	std::map<sponza::MeshId, FriendsMeshGL> friendsMeshes;
	std::map<std::string, GLuint> textures;

	ShaderProgram mSkyboxShaderProgram;
	GLuint mSkyboxTexture;
	GLuint mSkyboxPositionVBO;
	GLuint mSkyboxVAO;

private: // Functions.
    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	void LoadMeshData();
	void LoadTexture(std::string name);
};



