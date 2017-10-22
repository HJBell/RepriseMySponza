#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>
#include "ShaderProgram.hpp"
#include "MeshData.hpp"

#define MAX_LIGHT_COUNT 32
#define MAX_INSTANCE_COUNT 64


//----------------------Structures----------------------

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
public:
    MyView();
    ~MyView();

    void setScene(const sponza::Context * sponza);
	void ToggleSkybox();

private:
	const sponza::Context * scene_;

	ShaderProgram mSkyboxShaderProgram;
	ShaderProgram mAmbShaderProgram;
	ShaderProgram mDirShaderProgram;
	ShaderProgram mPointShaderProgram;
	ShaderProgram mSpotShaderProgram;

	std::map<sponza::MeshId, MeshData> mMeshes;
	std::map<std::string, GLuint> mTextures;

	bool mRenderSkybox = false;
	
	GLuint mSkyboxTexture;
	GLuint mSkyboxPositionVBO;
	GLuint mSkyboxVAO;

	std::vector<PerModelUniforms> perModelUniforms;

    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;	
	void LoadTexture(std::string name);
	void DrawMeshesInstanced(ShaderProgram& shaderProgram);
};



