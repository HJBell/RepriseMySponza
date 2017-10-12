#include "MyView.hpp"
#include "Utils.hpp""

#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>


//------------------------------------------Public Interface------------------------------------------

MyView::MyView()
{
	
}


MyView::~MyView() 
{

}


void MyView::setScene(const sponza::Context * sponza)
{
    scene_ = sponza;
}


//------------------------------------------Private Functions-----------------------------------------

void MyView::windowViewWillStart(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
    assert(scene_ != nullptr);

	// Loading the shader program.
	shaderProgram = loadShaderProgram(vertexShaderPath, fragmentShaderPath);

	// Load the mesh data.
	loadMeshData();

	// Creating the uniform buffer blocks.
	createUniformBufferObjects();

	// Loading textures.
	loadTexture("resource:///hex.png");
	loadTexture("resource:///marble.png");

	// Enabling the OpenGL depth test.
	glEnable(GL_DEPTH_TEST);

	// Enabling polygons to be culled when they leave the window.
	glEnable(GL_CULL_FACE);

	// Setting the polygon rasterization mode.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Enabling the shader program.
	glUseProgram(shaderProgram);
}


void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
	// Setting the OpenGL viewport position and size.
    glViewport(0, 0, width, height);

	// Specifying clear values for the colour buffers (0-1).
	glClearColor(0.f, 0.f, 0.25f, 0.f);
}


void MyView::windowViewDidStop(tygra::Window * window)
{
	// Deleting the shader program.
	glDeleteProgram(shaderProgram);

	// Deleting the textures.
	for (auto tex : textures)
	{
		glDeleteTextures(1, &tex.second);
	}

	// Deleting the buffers for each sponza mesh.
	for (auto sponzaMesh : sponzaMeshes)
	{
		glDeleteBuffers(1, &sponzaMesh.second.positionVBO);
		glDeleteBuffers(1, &sponzaMesh.second.normalVBO);
		glDeleteBuffers(1, &sponzaMesh.second.textureCoordVBO);
		glDeleteBuffers(1, &sponzaMesh.second.elementVBO);
		glDeleteVertexArrays(1, &sponzaMesh.second.vao);
	}

	// Deleting the buffers for each friends mesh.
	for (auto friendMesh : friendsMeshes)
	{
		glDeleteBuffers(1, &friendMesh.second.positionVBO);
		glDeleteBuffers(1, &friendMesh.second.normalVBO);
		glDeleteBuffers(1, &friendMesh.second.elementVBO);
		glDeleteVertexArrays(1, &friendMesh.second.vao);
	}
}


void MyView::windowViewRender(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
    assert(scene_ != nullptr);

	// Clearing the contents of the buffers from the previous frame.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculating the aspect ratio.
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = viewportSize[2] / (float)viewportSize[3];

	// Creating the projection matrix.
	sponza::Camera camera = scene_->getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(camera.getVerticalFieldOfViewInDegrees()),
		aspectRatio, camera.getNearPlaneDistance(),
		camera.getFarPlaneDistance());

	// Creating the uniform buffer object structs.
	PerFrameUniforms perFrameUniforms;
	PerModelUniforms perModelUniforms;

	// Creating the view matrix.
	perFrameUniforms.cameraPos = Utils::SponzaToGLMVec3(camera.getPosition());
	auto camDir = Utils::SponzaToGLMVec3(camera.getDirection());
	auto upDir = Utils::SponzaToGLMVec3(scene_->getUpDirection());
	glm::mat4 view = glm::lookAt(perFrameUniforms.cameraPos, perFrameUniforms.cameraPos + camDir, upDir);

	// Setting the ambient intensity in the uniform buffer.
	perFrameUniforms.ambientIntensity = Utils::SponzaToGLMVec3(scene_->getAmbientLightIntensity());

	// Setting the directional lights in the uniform buffer.
	const auto directionalLights = scene_->getAllDirectionalLights();
	const auto directionalLightCount = (directionalLights.size() <= maxLightsPerType) ? directionalLights.size() : maxLightsPerType;
	perFrameUniforms.directionalLightCount = directionalLightCount;
	for (unsigned i = 0; i < directionalLightCount; i++)
	{
		perFrameUniforms.directionalLights[i].direction = Utils::SponzaToGLMVec3(directionalLights[i].getDirection());	
		perFrameUniforms.directionalLights[i].intensity = Utils::SponzaToGLMVec3(directionalLights[i].getIntensity());
	}

	// Setting the point lights in the uniform buffer.
	const auto pointLights = scene_->getAllPointLights();
	const auto pointLightCount = (pointLights.size() <= maxLightsPerType) ? pointLights.size() : maxLightsPerType;
	perFrameUniforms.pointLightCount = pointLightCount;
	for (unsigned i = 0; i < pointLightCount; i++)
	{
		perFrameUniforms.pointLights[i].position = Utils::SponzaToGLMVec3(pointLights[i].getPosition());
		perFrameUniforms.pointLights[i].range = pointLights[i].getRange();
		perFrameUniforms.pointLights[i].intensity = Utils::SponzaToGLMVec3(pointLights[i].getIntensity());
	}

	// Setting the spot lights in the uniform buffer.
	const auto spotLights = scene_->getAllSpotLights();
	const auto spotLightCount = (spotLights.size() <= maxLightsPerType) ? spotLights.size() : maxLightsPerType;
	perFrameUniforms.spotLightCount = spotLightCount;
	for (unsigned i = 0; i < spotLightCount; i++)
	{
		perFrameUniforms.spotLights[i].position = Utils::SponzaToGLMVec3(spotLights[i].getPosition());
		perFrameUniforms.spotLights[i].range = spotLights[i].getRange();
		perFrameUniforms.spotLights[i].intensity = Utils::SponzaToGLMVec3(spotLights[i].getIntensity());
		perFrameUniforms.spotLights[i].angle = spotLights[i].getConeAngleDegrees();
		perFrameUniforms.spotLights[i].direction = Utils::SponzaToGLMVec3(spotLights[i].getDirection());
	}

	// Memcopying the data for the per frame uniforms.
	glBindBuffer(GL_UNIFORM_BUFFER, perFrameUniformsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perFrameUniforms), &perFrameUniforms);	

	// Looping through all instances in the scene and drawing them.
	for(auto instance : scene_->getAllInstances())
	{
		// Setting the xforms in the uniform buffer.
		perModelUniforms.modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
		perModelUniforms.mvpXform = projection * view * perModelUniforms.modelXform;

		// Setting the material properties in the uniform buffer.
		auto material = scene_->getMaterialById(instance.getMaterialId());
		perModelUniforms.diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
		perModelUniforms.shininess = material.getShininess();
		perModelUniforms.specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
		perModelUniforms.isShiny = material.isShiny();

		// Memcopying the data for the per frame uniforms.
		glBindBuffer(GL_UNIFORM_BUFFER, perModelUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

		// Sponza instance drawing.
		if (sponzaMeshes.find(instance.getMeshId()) != sponzaMeshes.end())
		{
			// Populating the texture uniform variabes.
			setShaderTexture("resource:///marble.png", shaderProgram, "cpp_Texture", GL_TEXTURE0, 0);

			// Getting the mesh of the current instance.
			const SponzaMeshGL& sponzaMesh = sponzaMeshes[instance.getMeshId()];

			// Drawing the instance.
			glBindVertexArray(sponzaMesh.vao);
			glDrawElements(GL_TRIANGLES, sponzaMesh.elementCount, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		// Friends instance drawing.
		else
		{
			// Getting the mesh of the current instance.
			const FriendsMeshGL& friendMesh = friendsMeshes[instance.getMeshId()];

			// Drawing the instance.
			glBindVertexArray(friendMesh.vao);
			glDrawElements(GL_TRIANGLES, friendMesh.elementCount, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}		
	}
}


GLuint MyView::loadShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath) const
{
	// Load the shaders.
	GLuint vertexShader = loadShader(vertexShaderPath, GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	// Link the shader program.
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check that the shader program linked correctly.
	GLint linkSuccessful = GL_FALSE;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccessful);
	if (linkSuccessful != GL_TRUE)
	{
		int infoLogLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(shaderProgram, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error compiling shader program : " << &infoLog[0] << std::endl;
	}

	// Return the address of the shader program.
	return shaderProgram;
}


GLuint MyView::loadShader(std::string shaderPath, GLuint shaderType) const
{
	// Terminating the program if an invalid shader type is provided.
	assert(shaderType == GL_VERTEX_SHADER ||
			shaderType == GL_FRAGMENT_SHADER || 
			shaderType == GL_GEOMETRY_SHADER);

	// Create the shader object.
	GLuint shader = glCreateShader(shaderType);

	// Read the shader code from its file.
	std::string shaderString = tygra::createStringFromFile(shaderPath);
	auto shaderCString = shaderString.c_str();

	// Compile the shader.
	glShaderSource(shader, 1, &shaderCString, NULL);
	glCompileShader(shader);

	// Check the shader compiled correctly.
	GLint compileSuccessful = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccessful);
	if (compileSuccessful != GL_TRUE)
	{
		int infoLogLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error compiling shader '" << shaderPath << "' : " << &infoLog[0] << std::endl;
	}

	// Return the shader.
	return shader;
}


void MyView::loadMeshData()
{
	// Load all meshes from Sponza geometry builder.
	sponza::GeometryBuilder geometryBuilder;
	std::vector<sponza::Mesh> meshData = geometryBuilder.getAllMeshes();

	// Loop through the meshes.
	for (const auto& mesh : meshData)
	{
		// Friends meshes.
		if (mesh.getTextureCoordinateArray().size() <= 0)
		{
			// Break the mesh down into its components.
			const auto& positions = mesh.getPositionArray();
			const auto& normals = mesh.getNormalArray();
			const auto& elements = mesh.getElementArray();

			// Create the position vertex buffer object.
			glGenBuffers(1, &friendsMeshes[mesh.getId()].positionVBO);
			glBindBuffer(GL_ARRAY_BUFFER, friendsMeshes[mesh.getId()].positionVBO);
			glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Create the normal vertex buffer object.
			glGenBuffers(1, &friendsMeshes[mesh.getId()].normalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, friendsMeshes[mesh.getId()].normalVBO);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Create the element vertex buffer object.
			glGenBuffers(1, &friendsMeshes[mesh.getId()].elementVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, friendsMeshes[mesh.getId()].elementVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			friendsMeshes[mesh.getId()].elementCount = elements.size();

			// Create the vertex array object.
			glGenVertexArrays(1, &friendsMeshes[mesh.getId()].vao);
			glBindVertexArray(friendsMeshes[mesh.getId()].vao);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, friendsMeshes[mesh.getId()].elementVBO);

			glBindBuffer(GL_ARRAY_BUFFER, friendsMeshes[mesh.getId()].positionVBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

			glBindBuffer(GL_ARRAY_BUFFER, friendsMeshes[mesh.getId()].normalVBO);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		// Sponza meshes.
		else
		{
			// Break the mesh down into its components.
			const auto& positions = mesh.getPositionArray();
			const auto& normals = mesh.getNormalArray();
			const auto& elements = mesh.getElementArray();
			const auto& textureCoords = mesh.getTextureCoordinateArray();

			// Create the position vertex buffer object.
			glGenBuffers(1, &sponzaMeshes[mesh.getId()].positionVBO);
			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].positionVBO);
			glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Create the normal vertex buffer object.
			glGenBuffers(1, &sponzaMeshes[mesh.getId()].normalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].normalVBO);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Create the element vertex buffer object.
			glGenBuffers(1, &sponzaMeshes[mesh.getId()].elementVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].elementVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			sponzaMeshes[mesh.getId()].elementCount = elements.size();

			// Create the texture coord vertex buffer object.
			glGenBuffers(1, &sponzaMeshes[mesh.getId()].textureCoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].textureCoordVBO);
			glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(glm::vec2), textureCoords.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Create the vertex array object.
			glGenVertexArrays(1, &sponzaMeshes[mesh.getId()].vao);
			glBindVertexArray(sponzaMeshes[mesh.getId()].vao);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].elementVBO);

			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].positionVBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].normalVBO);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

			glBindBuffer(GL_ARRAY_BUFFER, sponzaMeshes[mesh.getId()].textureCoordVBO);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}	
	}
}


void MyView::loadTexture(std::string name)
{
	//Checking the texture is not already loaded.
	if (textures.find(name) != textures.end()) return;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Loading the texture.
	tygra::Image texture = tygra::createImageFromPngFile(name);

	//Checking the texture contains data.
	if (texture.doesContainData())
	{
		//Loading the texture and storing its ID in the 'textures' map.
		glGenTextures(1, &textures[name]);
		glBindTexture(GL_TEXTURE_2D, textures[name]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };

		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			texture.width(),
			texture.height(),
			0,
			pixel_formats[texture.componentsPerPixel()],
			texture.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
			: GL_UNSIGNED_SHORT,
			texture.pixelData());

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else std::cout << "Warning : Texture '" << name << "' does not contain any data." << std::endl;
}

bool MyView::setShaderTexture(std::string name, GLuint shaderProgram, std::string targetName, GLenum activeTexture, int index)
{
	//Checking if the texture is loaded.
	if (textures.find(name) != textures.end())
	{
		//Binding the texture to a uniform variable.
		glActiveTexture(activeTexture);
		glBindTexture(GL_TEXTURE_2D, textures[name]);
		glUniform1i(glGetUniformLocation(shaderProgram, targetName.c_str()), index);
		return true;
	}
	return false;
}

void MyView::createUniformBufferObjects()
{
	// Creating per frame uniform buffer object.
	glGenBuffers(1, &perFrameUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, perFrameUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, perFrameUniformsUBO);
	glUniformBlockBinding(shaderProgram, glGetUniformBlockIndex(shaderProgram, "cpp_PerFrameUniforms"), 0);

	// Creating per model uniform buffer object.
	glGenBuffers(1, &perModelUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, perModelUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, perModelUniformsUBO);
	glUniformBlockBinding(shaderProgram, glGetUniformBlockIndex(shaderProgram, "cpp_PerModelUniforms"), 1);
}
