#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>


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
	std::string	vertexShaderPath = "resource:///reprise_vs.glsl";
	std::string	fragmentShaderPath = "resource:///reprise_fs.glsl";

private: // Functions.
    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	GLuint loadShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath) const;
	GLuint loadShader(std::string shaderPath, GLuint shaderType) const;
	void loadMeshData();
	void LoadTexture(std::string name);
	bool SetShaderTexture(std::string name, GLuint shaderProgram, std::string targetName, GLenum activeTexture, int index);
	
};

