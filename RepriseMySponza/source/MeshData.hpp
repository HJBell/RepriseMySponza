#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tgl/tgl.h>

class MeshData
{
public:
	MeshData();
	~MeshData();

	int elementCount = 0;
	GLuint vao = 0;

	void Init(const sponza::Mesh& mesh);
	void BindVAO() const;

private:
	GLuint positionVBO = 0;
	GLuint normalVBO = 0;
	GLuint elementVBO = 0;
	GLuint textureCoordVBO = 0;
	
	void GenerateBuffer(GLuint& buffer, const void* data, GLsizeiptr size, GLenum bufferType, GLenum drawType);
};

