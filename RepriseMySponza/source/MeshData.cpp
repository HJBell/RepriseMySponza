#include "MeshData.hpp"
#include <sponza/sponza.hpp>
#include <glm/glm.hpp>


MeshData::MeshData()
{
}


MeshData::~MeshData()
{
	glDeleteBuffers(1, &positionVBO);
	glDeleteBuffers(1, &normalVBO);
	if(textureCoordVBO!=0)
		glDeleteBuffers(1, &textureCoordVBO);
	glDeleteBuffers(1, &elementVBO);
	glDeleteVertexArrays(1, &vao);
}

void MeshData::Init(const sponza::Mesh& mesh)
{
	// Break the mesh down into its components.
	const auto& positions = mesh.getPositionArray();
	const auto& normals = mesh.getNormalArray();
	const auto& elements = mesh.getElementArray();
	const auto& textureCoords = mesh.getTextureCoordinateArray();

	// Create the VBOs.
	GenerateBuffer(positionVBO, positions.data(), positions.size() * sizeof(glm::vec3), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	GenerateBuffer(normalVBO, normals.data(), normals.size() * sizeof(glm::vec3), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	GenerateBuffer(elementVBO, elements.data(), elements.size() * sizeof(unsigned int), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	if(textureCoords.size() > 0)
		GenerateBuffer(textureCoordVBO, textureCoords.data(), textureCoords.size() * sizeof(glm::vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);

	// Record the element count.
	elementCount = elements.size();

	// Create the vertex array object.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVBO);

	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

	if (textureCoords.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, textureCoordVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshData::BindVAO() const
{
	glBindVertexArray(vao);
}

void MeshData::GenerateBuffer(GLuint& buffer, const void* data, GLsizeiptr size, GLenum bufferType, GLenum drawType)
{
	glGenBuffers(1, &buffer);
	glBindBuffer(bufferType, buffer);
	glBufferData(bufferType, size, data, drawType);
	glBindBuffer(bufferType, 0);
}