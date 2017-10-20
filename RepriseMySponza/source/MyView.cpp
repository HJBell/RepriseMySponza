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

void MyView::ToggleSkybox()
{
	mRenderSkybox = !mRenderSkybox;
}


//------------------------------------------Private Functions-----------------------------------------

void MyView::windowViewWillStart(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
    assert(scene_ != nullptr);

	// Creating the ambient pass shader program.
	mAmbShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///ambient_fs.glsl");
	mAmbShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 0);
	mAmbShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 1);

	// Creating the direction light pass shader program.
	mDirShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///dir_fs.glsl");
	mDirShaderProgram.CreateUniformBuffer("cpp_DirectionalLightUniforms", sizeof(DirectionalLightUniforms), 2);
	mDirShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 3);
	mDirShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 4);

	// Creating the point light pass shader program.
	mPointShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///point_fs.glsl");
	mPointShaderProgram.CreateUniformBuffer("cpp_PointLightUniforms", sizeof(PointLightUniforms), 5);
	mPointShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 6);
	mPointShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 7);

	// Creating the spot light pass shader program.
	mSpotShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///spot_fs.glsl");
	mSpotShaderProgram.CreateUniformBuffer("cpp_SpotLightUniforms", sizeof(SpotLightUniforms), 8);
	mSpotShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 9);
	mSpotShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 10);
	
	// Load the mesh data.
	LoadMeshData();
	
	// Loading textures.
	LoadTexture("resource:///hex.png");
	LoadTexture("resource:///marble.png");




	glm::vec3 SkyBoxVertices[36] =
	{
		glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f),
		glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f, -10.0f,  10.0f),
		glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f, -10.0f),
		glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, -10.0f,  10.0f),
		glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f, -10.0f,  10.0f),
		glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f, -10.0f, -10.0f)
	};


	//------------------------------skybox------------------------------

	mSkyboxShaderProgram.Init("resource:///skybox_vs.glsl", "resource:///skybox_fs.glsl");
	mSkyboxShaderProgram.CreateUniformBuffer("cpp_SkyboxUniforms", sizeof(SkyboxUniforms), 11);


	glGenBuffers(1, &mSkyboxPositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mSkyboxPositionVBO);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), SkyBoxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &mSkyboxVAO);
	glBindVertexArray(mSkyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mSkyboxPositionVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTextures(1, &mSkyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyboxTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	for (size_t i = 0; i < 6; ++i) {
		const std::string url = "resource:///skybox_stormy_" + std::to_string(i) + ".png";
		tygra::Image img = tygra::createImageFromPngFile(url);
		if (!img.doesContainData()) {
			throw std::runtime_error("failed to load " + url);
		}
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, img.width(), img.height(), 0,
			pixel_formats[img.componentsPerPixel()],
			img.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, img.pixelData());
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	// tell GL to wrap on the edges to neighbouring faces
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


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

	glDeleteBuffers(1, &mSkyboxPositionVBO);
	glDeleteTextures(1, &mSkyboxTexture);
	glDeleteVertexArrays(1, &mSkyboxVAO);
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

	// Creating the per frame and per model structs.
	PerFrameUniforms perFrameUniforms;

	// Getting the camera data for the frame.
	sponza::Camera camera = scene_->getCamera();
	auto camDir = Utils::SponzaToGLMVec3(camera.getDirection());
	auto upDir = Utils::SponzaToGLMVec3(scene_->getUpDirection());

	// Populating the per frame uniform buffer.
	perFrameUniforms.cameraPos = Utils::SponzaToGLMVec3(camera.getPosition());
	perFrameUniforms.ambientIntensity = Utils::SponzaToGLMVec3(scene_->getAmbientLightIntensity());

	// Calculating the projection and view matrices.
	glm::mat4 projection = glm::perspective(glm::radians(camera.getVerticalFieldOfViewInDegrees()),
		aspectRatio, camera.getNearPlaneDistance(),
		camera.getFarPlaneDistance());	
	glm::mat4 view = glm::lookAt(perFrameUniforms.cameraPos, perFrameUniforms.cameraPos + camDir, upDir);




	// --------------------Skybox--------------------

	if (mRenderSkybox)
	{
		mSkyboxShaderProgram.Use();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		SkyboxUniforms skyboxUniforms;
		skyboxUniforms.cameraPos = Utils::SponzaToGLMVec3(camera.getPosition());
		skyboxUniforms.viewProjectionXform = projection * view;
		mSkyboxShaderProgram.SetUniformBuffer("cpp_SkyboxUniforms", &skyboxUniforms, sizeof(skyboxUniforms));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyboxTexture);
		glBindVertexArray(mSkyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}	


	// --------------------Populating the per model uniform buffers--------------------
	
	std::vector<PerModelUniforms> sponzaPerModelUniforms;
	for (auto mesh : sponzaMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		PerModelUniforms perModelUniforms;

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
		sponzaPerModelUniforms.push_back(perModelUniforms);
	}


	std::vector<PerModelUniforms> friendsPerModelUniforms;
	for (auto mesh : friendsMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		PerModelUniforms perModelUniforms;

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
		friendsPerModelUniforms.push_back(perModelUniforms);
	}


	// -----------------Ambient pass-----------------

	mAmbShaderProgram.Use();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_BLEND);

	// Setting the per frame uniform buffer.
	mAmbShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

	// Drawing the sponza meshes.
	int i = 0;
	for (auto mesh : sponzaMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		mAmbShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(sponzaPerModelUniforms[i]), sizeof((sponzaPerModelUniforms[i])));
		mAmbShaderProgram.SetTextureUniform(textures["resource:///marble.png"], "cpp_Texture");

		glBindVertexArray(sponzaMeshes[meshID].vao);
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
		glBindVertexArray(0);

		i++;
	}

	// Drawing the friends meshes.
	i = 0;
	for (auto mesh : friendsMeshes)
	{
		int meshID = mesh.first;
		auto instanceIDs = scene_->getInstancesByMeshId(meshID);
		int instanceCount = instanceIDs.size();

		mAmbShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(friendsPerModelUniforms[i]), sizeof((friendsPerModelUniforms[i])));

		glBindVertexArray(friendsMeshes[meshID].vao);
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
		glBindVertexArray(0);

		i++;
	}


	// -----------------Directional Light pass-----------------

	mDirShaderProgram.Use();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// Setting the per frame uniform buffer.
	mDirShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

	for (auto light : scene_->getAllDirectionalLights())
	{
		DirectionalLightUniforms directionalLightUniform;
		directionalLightUniform.light.direction = Utils::SponzaToGLMVec3(light.getDirection());
		directionalLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());

		mDirShaderProgram.SetUniformBuffer("cpp_DirectionalLightUniforms", &directionalLightUniform, sizeof(directionalLightUniform));

		// Drawing the sponza meshes.
		int i = 0;
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mDirShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(sponzaPerModelUniforms[i]), sizeof((sponzaPerModelUniforms[i])));
			mDirShaderProgram.SetTextureUniform(textures["resource:///marble.png"], "cpp_Texture");

			glBindVertexArray(sponzaMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}

		// Drawing the friends meshes.
		i = 0;
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mDirShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(friendsPerModelUniforms[i]), sizeof((friendsPerModelUniforms[i])));

			glBindVertexArray(friendsMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}
	}


	// -----------------Point Light pass-----------------

	mPointShaderProgram.Use();

	// Setting the per frame uniform buffer.
	mPointShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

	for (auto light : scene_->getAllPointLights())
	{
		PointLightUniforms pointLightUniform;
		pointLightUniform.light.position = Utils::SponzaToGLMVec3(light.getPosition());
		pointLightUniform.light.range = light.getRange();
		pointLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());

		mPointShaderProgram.SetUniformBuffer("cpp_PointLightUniforms", &pointLightUniform, sizeof(pointLightUniform));

		// Drawing the sponza meshes.
		int i = 0;
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mPointShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(sponzaPerModelUniforms[i]), sizeof((sponzaPerModelUniforms[i])));
			mPointShaderProgram.SetTextureUniform(textures["resource:///marble.png"], "cpp_Texture");

			glBindVertexArray(sponzaMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}

		// Drawing the friends meshes.
		i = 0;
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mPointShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(friendsPerModelUniforms[i]), sizeof((friendsPerModelUniforms[i])));

			glBindVertexArray(friendsMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}
	}


	// -----------------Spot Light pass-----------------

	mSpotShaderProgram.Use();

	// Setting the per frame uniform buffer.
	mSpotShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

	for (auto light : scene_->getAllSpotLights())
	{
		SpotLightUniforms spotLightUniform;
		spotLightUniform.light.position = Utils::SponzaToGLMVec3(light.getPosition());
		spotLightUniform.light.range = light.getRange();
		spotLightUniform.light.intensity = Utils::SponzaToGLMVec3(light.getIntensity());
		spotLightUniform.light.angle = light.getConeAngleDegrees();
		spotLightUniform.light.direction = Utils::SponzaToGLMVec3(light.getDirection());

		mSpotShaderProgram.SetUniformBuffer("cpp_SpotLightUniforms", &spotLightUniform, sizeof(spotLightUniform));

		// Drawing the sponza meshes.
		int i = 0;
		for (auto mesh : sponzaMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mSpotShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(sponzaPerModelUniforms[i]), sizeof((sponzaPerModelUniforms[i])));
			mSpotShaderProgram.SetTextureUniform(textures["resource:///marble.png"], "cpp_Texture");

			glBindVertexArray(sponzaMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}

		// Drawing the friends meshes.
		i = 0;
		for (auto mesh : friendsMeshes)
		{
			int meshID = mesh.first;
			auto instanceIDs = scene_->getInstancesByMeshId(meshID);
			int instanceCount = instanceIDs.size();

			mSpotShaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(friendsPerModelUniforms[i]), sizeof((friendsPerModelUniforms[i])));

			glBindVertexArray(friendsMeshes[meshID].vao);
			glDrawElementsInstanced(GL_TRIANGLES, mesh.second.elementCount, GL_UNSIGNED_INT, 0, instanceCount);
			glBindVertexArray(0);

			i++;
		}
	}
}

void MyView::LoadMeshData()
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


void MyView::LoadTexture(std::string name)
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
	else std::cerr << "Warning : Texture '" << name << "' does not contain any data." << std::endl;
}
