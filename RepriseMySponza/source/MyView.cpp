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
	ambientShaderProgram = loadShaderProgram("resource:///ambient_vs.glsl", "resource:///ambient_fs.glsl");
	directionalLightShaderProgram = loadShaderProgram("resource:///dir_vs.glsl", "resource:///dir_fs.glsl");
	pointLightShaderProgram = loadShaderProgram("resource:///point_vs.glsl", "resource:///point_fs.glsl");
	spotLightShaderProgram = loadShaderProgram("resource:///spot_vs.glsl", "resource:///spot_fs.glsl");

	// Load the mesh data.
	loadMeshData();

	// Creating the uniform buffer blocks.
	createUniformBufferObjects();
	
	// Loading textures.
	loadTexture("resource:///hex.png");
	loadTexture("resource:///marble.png");

	// Enabling polygons to be culled when they leave the window.
	glEnable(GL_CULL_FACE);

	// Setting the polygon rasterization mode.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	// Deleting the shader programs.
	glDeleteProgram(ambientShaderProgram);
	glDeleteProgram(directionalLightShaderProgram);
	glDeleteProgram(pointLightShaderProgram);
	glDeleteProgram(spotLightShaderProgram);

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
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculating the aspect ratio.
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = viewportSize[2] / (float)viewportSize[3];



	// -----------------Ambient pass-----------------

	glUseProgram(ambientShaderProgram);

	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_TRUE);

	glDepthFunc(GL_LESS);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glDisable(GL_BLEND);

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

	// Memcopying the data for the per frame uniforms.
	glBindBuffer(GL_UNIFORM_BUFFER, ambPassPerFrameUniformsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perFrameUniforms), &perFrameUniforms);


	// Drawing the sponza meshes.
	for (auto mesh : sponzaMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		// Loop through the instances and populate the uniform buffer block.
		for (int i = 0; i < instanceCount; i++)
		{
			auto instance = scene_->getInstanceById(instanceIDs[i]);

			// Setting the xforms in the uniform buffer.
			perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
			perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

			// Setting the material properties in the uniform buffer.
			auto material = scene_->getMaterialById(instance.getMaterialId());
			perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
			perModelUniforms.instances[i].shininess = material.getShininess();
			perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
			perModelUniforms.instances[i].isShiny = material.isShiny();
		}

		glBindBuffer(GL_UNIFORM_BUFFER, ambPassPerModelUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

		setShaderTexture("resource:///marble.png", ambientShaderProgram, "cpp_Texture", GL_TEXTURE0, 0);

		// Drawing the instance.
		glBindVertexArray(mesh.second.vao);
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
		glBindVertexArray(0);
	}

	// Drawing the friends meshes.
	for (auto mesh : friendsMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		// Loop through the instances and populate the uniform buffer block.
		for (int i = 0; i < instanceCount; i++)
		{
			auto instance = scene_->getInstanceById(instanceIDs[i]);

			// Setting the xforms in the uniform buffer.
			perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
			perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

			// Setting the material properties in the uniform buffer.
			auto material = scene_->getMaterialById(instance.getMaterialId());
			perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
			perModelUniforms.instances[i].shininess = material.getShininess();
			perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
			perModelUniforms.instances[i].isShiny = material.isShiny();
		}

		glBindBuffer(GL_UNIFORM_BUFFER, ambPassPerModelUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

		// Drawing the instance.
		glBindVertexArray(mesh.second.vao);
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
		glBindVertexArray(0);
	}


	// -----------------Directional Light pass-----------------
		
	glUseProgram(directionalLightShaderProgram);

	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_FALSE);

	glDepthFunc(GL_EQUAL);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_BLEND);

	glBlendEquation(GL_FUNC_ADD);

	glBlendFunc(GL_ONE, GL_ONE);

	// Memcopying the data for the per frame uniforms.
	glBindBuffer(GL_UNIFORM_BUFFER, dirPassPerFrameUniformsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perFrameUniforms), &perFrameUniforms);

	for (auto light : scene_->getAllDirectionalLights())
	{
		DirectionalLightUniforms directionalLightUniform;
		directionalLightUniform.light.direction = Utils::SponzaToGLMVec3(light.getDirection());
		directionalLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());

		// Memcopying the data for the per frame uniforms.
		glBindBuffer(GL_UNIFORM_BUFFER, dirPassDirLightUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(directionalLightUniform), &directionalLightUniform);

		// Drawing the sponza meshes.
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, dirPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			setShaderTexture("resource:///marble.png", directionalLightShaderProgram, "cpp_Texture", GL_TEXTURE0, 0);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}

		// Drawing the friends meshes.
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, dirPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}
	}




	// -----------------Point Light pass-----------------

	glUseProgram(pointLightShaderProgram);

	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_FALSE);

	glDepthFunc(GL_EQUAL);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_BLEND);

	glBlendEquation(GL_FUNC_ADD);

	glBlendFunc(GL_ONE, GL_ONE);

	// Memcopying the data for the per frame uniforms.
	glBindBuffer(GL_UNIFORM_BUFFER, pointPassPerFrameUniformsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perFrameUniforms), &perFrameUniforms);

	for (auto light : scene_->getAllPointLights())
	{
		PointLightUniforms pointLightUniform;
		pointLightUniform.light.position = Utils::SponzaToGLMVec3(light.getPosition());
		pointLightUniform.light.range = light.getRange();
		pointLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());

		// Memcopying the data for the per frame uniforms.
		glBindBuffer(GL_UNIFORM_BUFFER, pointPassPointLightUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(pointLightUniform), &pointLightUniform);

		// Drawing the sponza meshes.
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, pointPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			setShaderTexture("resource:///marble.png", pointLightShaderProgram, "cpp_Texture", GL_TEXTURE0, 0);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}

		// Drawing the friends meshes.
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, pointPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}
	}







	// -----------------Spot Light pass-----------------

	glUseProgram(spotLightShaderProgram);

	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_FALSE);

	glDepthFunc(GL_EQUAL);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_BLEND);

	glBlendEquation(GL_FUNC_ADD);

	glBlendFunc(GL_ONE, GL_ONE);

	// Memcopying the data for the per frame uniforms.
	glBindBuffer(GL_UNIFORM_BUFFER, spotPassPerFrameUniformsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perFrameUniforms), &perFrameUniforms);

	for (auto light : scene_->getAllSpotLights())
	{
		SpotLightUniforms spotLightUniform;
		spotLightUniform.light.position = Utils::SponzaToGLMVec3(light.getPosition());
		spotLightUniform.light.range = light.getRange();
		spotLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());
		spotLightUniform.light.angle = light.getConeAngleDegrees();
		spotLightUniform.light.direction = Utils::SponzaToGLMVec3(light.getDirection());

		// Memcopying the data for the per frame uniforms.
		glBindBuffer(GL_UNIFORM_BUFFER, spotPassSpotLightUniformsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(spotLightUniform), &spotLightUniform);

		// Drawing the sponza meshes.
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, spotPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			setShaderTexture("resource:///marble.png", spotLightShaderProgram, "cpp_Texture", GL_TEXTURE0, 0);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}

		// Drawing the friends meshes.
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			// Loop through the instances and populate the uniform buffer block.
			for (int i = 0; i < instanceCount; i++)
			{
				auto instance = scene_->getInstanceById(instanceIDs[i]);

				// Setting the xforms in the uniform buffer.
				perModelUniforms.instances[i].modelXform = Utils::SponzaMat3ToGLMMat4(instance.getTransformationMatrix());
				perModelUniforms.instances[i].mvpXform = projection * view * perModelUniforms.instances[i].modelXform;

				// Setting the material properties in the uniform buffer.
				auto material = scene_->getMaterialById(instance.getMaterialId());
				perModelUniforms.instances[i].diffuse = Utils::SponzaToGLMVec3(material.getDiffuseColour());
				perModelUniforms.instances[i].shininess = material.getShininess();
				perModelUniforms.instances[i].specular = Utils::SponzaToGLMVec3(material.getSpecularColour());
				perModelUniforms.instances[i].isShiny = material.isShiny();
			}

			glBindBuffer(GL_UNIFORM_BUFFER, spotPassPerModelUniformsUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(perModelUniforms), &perModelUniforms);

			// Drawing the instance.
			glBindVertexArray(mesh.second.vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);
		}
	}
	

	


	

	

	

	//// Setting the directional lights in the uniform buffer.
	//const auto directionalLights = scene_->getAllDirectionalLights();
	//const auto directionalLightCount = (directionalLights.size() <= MAX_LIGHT_COUNT) ? directionalLights.size() : MAX_LIGHT_COUNT;
	//perFrameUniforms.directionalLightCount = directionalLightCount;
	//for (unsigned i = 0; i < directionalLightCount; i++)
	//{
	//	perFrameUniforms.directionalLights[i].direction = Utils::SponzaToGLMVec3(directionalLights[i].getDirection());	
	//	perFrameUniforms.directionalLights[i].intensity = Utils::SponzaToGLMVec3(directionalLights[i].getIntensity());
	//}

	//// Setting the point lights in the uniform buffer.
	//const auto pointLights = scene_->getAllPointLights();
	//const auto pointLightCount = (pointLights.size() <= MAX_LIGHT_COUNT) ? pointLights.size() : MAX_LIGHT_COUNT;
	//perFrameUniforms.pointLightCount = pointLightCount;
	//for (unsigned i = 0; i < pointLightCount; i++)
	//{
	//	perFrameUniforms.pointLights[i].position = Utils::SponzaToGLMVec3(pointLights[i].getPosition());
	//	perFrameUniforms.pointLights[i].range = pointLights[i].getRange();
	//	perFrameUniforms.pointLights[i].intensity = Utils::SponzaToGLMVec3(pointLights[i].getIntensity());
	//}

	//// Setting the spot lights in the uniform buffer.
	//const auto spotLights = scene_->getAllSpotLights();
	//const auto spotLightCount = (spotLights.size() <= MAX_LIGHT_COUNT) ? spotLights.size() : MAX_LIGHT_COUNT;
	//perFrameUniforms.spotLightCount = spotLightCount;
	//for (unsigned i = 0; i < spotLightCount; i++)
	//{
	//	perFrameUniforms.spotLights[i].position = Utils::SponzaToGLMVec3(spotLights[i].getPosition());
	//	perFrameUniforms.spotLights[i].range = spotLights[i].getRange();
	//	perFrameUniforms.spotLights[i].intensity = Utils::SponzaToGLMVec3(spotLights[i].getIntensity());
	//	perFrameUniforms.spotLights[i].angle = spotLights[i].getConeAngleDegrees();
	//	perFrameUniforms.spotLights[i].direction = Utils::SponzaToGLMVec3(spotLights[i].getDirection());
	//}

	
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
	//-------------------------Ambient Pass-------------------------

	// Creating per frame uniform buffer object.
	glGenBuffers(1, &ambPassPerFrameUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, ambPassPerFrameUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ambPassPerFrameUniformsUBO);
	glUniformBlockBinding(ambientShaderProgram, glGetUniformBlockIndex(ambientShaderProgram, "cpp_PerFrameUniforms"), 0);

	// Creating per model uniform buffer object.
	glGenBuffers(1, &ambPassPerModelUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, ambPassPerModelUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ambPassPerModelUniformsUBO);
	glUniformBlockBinding(ambientShaderProgram, glGetUniformBlockIndex(ambientShaderProgram, "cpp_PerModelUniforms"), 1);


	//-------------------------Directional Light Pass-------------------------

	// Creating directional light uniform buffer object.
	glGenBuffers(1, &dirPassDirLightUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, dirPassDirLightUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, dirPassDirLightUniformsUBO);
	glUniformBlockBinding(directionalLightShaderProgram, glGetUniformBlockIndex(directionalLightShaderProgram, "cpp_DirectionalLightUniforms"), 2);

	// Creating per frame uniform buffer object.
	glGenBuffers(1, &dirPassPerFrameUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, dirPassPerFrameUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, dirPassPerFrameUniformsUBO);
	glUniformBlockBinding(directionalLightShaderProgram, glGetUniformBlockIndex(directionalLightShaderProgram, "cpp_PerFrameUniforms"), 3);

	// Creating per model uniform buffer object.
	glGenBuffers(1, &dirPassPerModelUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, dirPassPerModelUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, dirPassPerModelUniformsUBO);
	glUniformBlockBinding(directionalLightShaderProgram, glGetUniformBlockIndex(directionalLightShaderProgram, "cpp_PerModelUniforms"), 4);


	//-------------------------Point Light Pass-------------------------

	// Creating point light uniform buffer object.
	glGenBuffers(1, &pointPassPointLightUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, pointPassPointLightUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, pointPassPointLightUniformsUBO);
	glUniformBlockBinding(pointLightShaderProgram, glGetUniformBlockIndex(pointLightShaderProgram, "cpp_PointLightUniforms"), 5);

	// Creating per frame uniform buffer object.
	glGenBuffers(1, &pointPassPerFrameUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, pointPassPerFrameUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, pointPassPerFrameUniformsUBO);
	glUniformBlockBinding(pointLightShaderProgram, glGetUniformBlockIndex(pointLightShaderProgram, "cpp_PerFrameUniforms"), 6);

	// Creating per model uniform buffer object.
	glGenBuffers(1, &pointPassPerModelUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, pointPassPerModelUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 7, pointPassPerModelUniformsUBO);
	glUniformBlockBinding(pointLightShaderProgram, glGetUniformBlockIndex(pointLightShaderProgram, "cpp_PerModelUniforms"), 7);


	//-------------------------Spot Light Pass-------------------------

	// Creating spot light uniform buffer object.
	glGenBuffers(1, &spotPassSpotLightUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, spotPassSpotLightUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 8, spotPassSpotLightUniformsUBO);
	glUniformBlockBinding(spotLightShaderProgram, glGetUniformBlockIndex(spotLightShaderProgram, "cpp_SpotLightUniforms"), 8);

	// Creating per frame uniform buffer object.
	glGenBuffers(1, &spotPassPerFrameUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, spotPassPerFrameUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 9, spotPassPerFrameUniformsUBO);
	glUniformBlockBinding(spotLightShaderProgram, glGetUniformBlockIndex(spotLightShaderProgram, "cpp_PerFrameUniforms"), 9);

	// Creating per model uniform buffer object.
	glGenBuffers(1, &spotPassPerModelUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, spotPassPerModelUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, spotPassPerModelUniformsUBO);
	glUniformBlockBinding(spotLightShaderProgram, glGetUniformBlockIndex(spotLightShaderProgram, "cpp_PerModelUniforms"), 10);
}
