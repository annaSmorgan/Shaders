#include "SSAO.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include "Application_Log.h"


#include <stb_image.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

SSAO::SSAO()
{

}

SSAO::~SSAO()
{

}

bool SSAO::onCreate()
{
	// initialise the Gizmos helper class
	Gizmos::create();

	//Load the shaders for this program
	m_fbo_vertexShader = Utility::loadShader("./shaders/vertex.glsl", GL_VERTEX_SHADER);
	m_fbo_fragmentShader = Utility::loadShader("./shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szfboInputs[] = { "Position", "Colour", "Normal", "Tangent", "BiTangent", "Tex1" };
	const char* szfboOutputs[] = { "RT_Diffuse", "RT_Normal", "RT_World" };
	//bind the shaders to create our shader program
	m_fbo_program = Utility::createProgram(
		m_fbo_vertexShader,
		0,
		0,
		0,
		m_fbo_fragmentShader,
		6, szfboInputs, 3, szfboOutputs);

	//\==========================================================================================
	//\ load the full screen shaders for this program
	//\==========================================================================================
	//Load the shaders for this program
	m_vertexShader = Utility::loadShader("./shaders/fs_vertex.glsl", GL_VERTEX_SHADER);
	m_fragmentShader = Utility::loadShader("./shaders/fs_fragment.glsl", GL_FRAGMENT_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs[] = { "Position", "Tex1" };
	const char* szOutputs[] = { "FragColor" };
	//bind the shaders to create our shader program
	m_programID = Utility::createProgram(
		m_vertexShader,
		0,
		0,
		0,
		m_fragmentShader,
		2, szInputs, 1, szOutputs);

	m_fbxModel = new FBXFile();
	m_fbxModel->load("./models/ruinedtank/tank.fbx", FBXFile::UNITS_DECIMETER);
	//the tank model has normal textures, these textures are not correctly associated with the model so they will need to be loaded in separatey
	LoadImageFromFile("./models/ruinedtank/left_engine_norm.png", m_textureID[0]);
	LoadImageFromFile("./models/ruinedtank/turret_norm.png", m_textureID[1]);

	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *pMesh = m_fbxModel->getMeshByIndex(i);
		if (strcmp("left_engine_phong1", pMesh->m_material->name) == 0 || strcmp("right_engine_phong1", pMesh->m_material->name) == 0)
		{
			memcpy(pMesh->m_material->textureFilenames[FBXMaterial::NormalTexture],"left_engine_norm.png", 20);
			pMesh->m_material->textureIDs[FBXMaterial::NormalTexture] = m_textureID[0];
			pMesh->m_material->textureTiling[FBXMaterial::NormalTexture] = glm::vec2(1.0f, 1.0f);
		}
		if (strcmp("barricade_phong1", pMesh->m_material->name) == 0 || strcmp("turret_phong1", pMesh->m_material->name) ==0)
		{
			memcpy(pMesh->m_material->textureFilenames[FBXMaterial::NormalTexture], "turret_norm.png", 15);
			pMesh->m_material->textureIDs[FBXMaterial::NormalTexture] = m_textureID[1];
			pMesh->m_material->textureTiling[FBXMaterial::NormalTexture] = glm::vec2(1.0f, 1.0f);
		}
	}
	//Generate our OpenGL Vertex and Index Buffers for rendering our FBX Model Data
	// OPENGL: genorate the VBO, IBO and VAO
	glGenBuffers(2, m_vbo);
	glGenBuffers(2, m_ibo);
	glGenVertexArrays(2, m_vao);

	// OPENGL: Bind  VAO, and then bind the VBO and IBO to the VAO
	glBindVertexArray(m_vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo[0]);

	//There is no need to populate the vbo & ibo buffers with any data at this stage
	//this can be done when rendering each mesh component of the FBX model

	// enable the attribute locations that will be used on our shaders
	glEnableVertexAttribArray(0); //Pos
	glEnableVertexAttribArray(1); //Col
	glEnableVertexAttribArray(2); //Norm
	glEnableVertexAttribArray(3); //Tangent
	glEnableVertexAttribArray(4); //BiTangent
	glEnableVertexAttribArray(5); //Tex1


								  // tell our shaders where the information within our buffers lie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), ((char *)0) + FBXVertex::PositionOffset);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::ColourOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::NormalOffset);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::TangentOffset);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::BiNormalOffset);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::TexCoord1Offset);

	// finally, where done describing our mesh to the shader
	// we can describe the next mesh
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//implement the full screen attributes 
	SVertex vertexData[4];
	vertexData[0] = SVertex(glm::vec4(-1.f, -1.f, 0.f, 1.f), glm::vec2(0.f, 0.f));
	vertexData[1] = SVertex(glm::vec4( 1.f, -1.f, 0.f, 1.f), glm::vec2(1.f, 0.f));
	vertexData[2] = SVertex(glm::vec4(-1.f,  1.f, 0.f, 1.f), glm::vec2(0.f, 1.f));
	vertexData[3] = SVertex(glm::vec4( 1.f,  1.f, 0.f, 1.f), glm::vec2(1.f, 1.f));

	unsigned int elements[6] = { 0, 1, 2, 1, 3, 2 };
	//bind openGL and fill buffers
	glBindVertexArray(m_vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SVertex), vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), elements, GL_STATIC_DRAW);
	//enable attributes
	glEnableVertexAttribArray(0); //Pos
	glEnableVertexAttribArray(1); //Tex1
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SVertex), ((char *)0) + SVertex::PositionOffset);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(SVertex), ((char *)0) + SVertex::TexCoord1Offset);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, (float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);

	m_modelMatrix = glm::mat4();

	Application_Log* log = Application_Log::Get();
	if (log != nullptr)
	{
		int maxAttachments = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
		log->addLog(LOG_LEVEL::LOG_INFO, "Num FBO colour attachments supported: %i", maxAttachments);
	}
	//\==========================================================================================================
	//\Create FBO for G-Pass
	//\==========================================================================================================
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	//set up the FBO diffuse render texture bind to colour attachment 0
	glGenTextures(1, &m_fbo_diffuse_texture);
	glBindTexture(GL_TEXTURE_2D, m_fbo_diffuse_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fbo_diffuse_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//set up the FBO normals render texture bind to colour attachment 1
	glGenTextures(1, &m_fbo_normal_texture);
	glBindTexture(GL_TEXTURE_2D, m_fbo_normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_fbo_normal_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//set up the FBO world space render texture bind to colour attachment 2
	glGenTextures(1, &m_fbo_worldspace_texture);
	glBindTexture(GL_TEXTURE_2D, m_fbo_worldspace_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_fbo_worldspace_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//attach depth texture
	glGenTextures(1, &m_fbo_depth_texture);
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_windowWidth, m_windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_fbo_depth_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		if (log != nullptr)
		{
			log->addLog(LOG_LEVEL::LOG_ERROR, "Framebuffer status incomplete!");
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	//Create the linear depth texture
	//generate the texture here to avoid any gl warnings
	glGenTextures(1, &m_fbo_linear_depth);
	glBindTexture(GL_TEXTURE_2D, m_fbo_linear_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_windowWidth,
		m_windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_lightPos = glm::vec4(50.f, 25.f, 0.f, 1.f);
		
	return true;
}

void SSAO::Update(float a_deltaTime)
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));

	// rotate point
	float s = sinf(a_deltaTime * 0.2f);
	float c = cosf(a_deltaTime * 0.2f);
	glm::vec4 temp = m_lightPos;
	m_lightPos.x = temp.x * c - temp.z * s;
	m_lightPos.z = temp.x * s + temp.z * c;
	m_lightPos.w = 0;
	m_lightPos = glm::normalize(m_lightPos) * 25.f;
	Gizmos::addBox(m_lightPos.xyz, glm::vec3(0.2f, 0.2f, 0.2f), true, glm::vec4(1.0f, 0.85f, 0.05f, 1.f));


	//Update our FBX model
	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = m_fbxModel->getMeshByIndex(i);
		// if you move an object around within your scene
		// children nodes are not updated until this function is called.
		mesh->updateGlobalTransform();
	}

	//\==========================================================================================================
	//\ ImGui render view to show what is held in the FBO texture position
	//\==========================================================================================================
	ImGui::SetNextWindowPos(ImVec2(m_windowWidth - m_windowWidth * 0.3f, 0.f));
	ImGui::SetNextWindowSize(ImVec2(m_windowWidth*0.3f, m_windowHeight* 0.35f));
	ImGui::Begin("Framebuffer");
	ImGui::BeginTabBar("FrameBuffer Textures");

	//ImGui::DrawTabsBackground();
	if (ImGui::BeginTabItem("Diffuse Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_fbo_diffuse_texture;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.28f, m_windowHeight*0.25f), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Normal Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_fbo_normal_texture;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.28f, m_windowHeight*0.25f), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("World Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_fbo_worldspace_texture;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.28f, m_windowHeight*0.25f), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Depth Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_fbo_linear_depth;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.28f, m_windowHeight*0.25f), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();


	ImGui::End();

	static bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
	Application_Log* log = Application_Log::Get();
	if (log != nullptr && show_demo_window)
	{
		log->showLog(&show_demo_window);
	}
	//show application log window
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(m_window, GLFW_KEY_L) == GLFW_PRESS) {
		show_demo_window = !show_demo_window;
	}

	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void SSAO::Draw()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// clear the backbuffer
	glClearColor(0.45f, 0.45f, 0.45f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

#pragma region First Pass Render
	
	//bing our shader program
	glUseProgram(m_fbo_program);
	//bind our vertex array object
	glBindVertexArray(m_vao[0]);

	//get our shaders uniform location for our projectionViewMatrix and then use glUniformMatrix4fv to fill it with the correct data
	unsigned int projectionViewUniform = glGetUniformLocation(m_fbo_program, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));

	//pass throught the view matrix
	unsigned int viewMatrixUniform = glGetUniformLocation(m_fbo_program, "ViewMatrix");
	glUniformMatrix4fv(viewMatrixUniform, 1, false, glm::value_ptr(viewMatrix));

	//pass our camera position to our fragment shader uniform
	unsigned int cameraPosUniform = glGetUniformLocation(m_fbo_program, "cameraPosition");
	glUniform4fv(cameraPosUniform, 1, glm::value_ptr(m_cameraMatrix[3]));


	//pass the directional light direction to our fragment shader
	glm::vec4 lightDir = -m_lightPos;
	lightDir.w = 0.f;
	lightDir = glm::normalize(lightDir);
	unsigned int lightDirUniform = glGetUniformLocation(m_fbo_program, "lightDirection");
	glUniform4fv(lightDirUniform, 1, glm::value_ptr(lightDir));

	//Draw our FBX Model
	//lets draw a few random dragons in the scene
	
	// for each mesh in the model...
	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *pMesh = m_fbxModel->getMeshByIndex(i);

		glm::mat4 m4Model = pMesh->m_globalTransform;
		unsigned int modelUniform = glGetUniformLocation(m_fbo_program, "Model");
		glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(m4Model));
		//send the normal matrix in view space!
		//this is only beneficial if a model has a non-uniform scale or non-orthoganal model matrix
		glm::mat4 m4Normal = glm::transpose(glm::inverse(viewMatrix * m4Model));// *m_modelMatrix;
		unsigned int normalMatrixUniform = glGetUniformLocation(m_fbo_program, "NormalMatrix");
		glUniformMatrix4fv(normalMatrixUniform, 1, false, glm::value_ptr(m4Normal));

		unsigned int texUniformID = glGetUniformLocation(m_fbo_program, "DiffuseTexture");
		glUniform1i(texUniformID, 0);
		//set our active texture, and bind our loaded texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::DiffuseTexture]);

		//As the model for the tank has it's normal data bound incorrectly we need to send through our loaded normal textures as the model contains none.
		texUniformID = glGetUniformLocation(m_fbo_program, "NormalTexture");
		glUniform1i(texUniformID, 1);
		//set our active texture, and bind our loaded texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::NormalTexture]);
		//glUniform1i(useNormalTextureUniform, 1);

		// Send the vertex data to the VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo[0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		glDrawElements(GL_TRIANGLES, (GLsizei)pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}
	
	glBindVertexArray(0);
	glUseProgram(0);
	
#pragma endregion

#pragma region Get Linear Depth Texture
	//get the recently created depth buffer texture from the FBO and linearise it
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	//Create a glfloat (float) buffer/array to store the depth texture
	GLfloat* pixels = new GLfloat[m_windowWidth*m_windowHeight];
	//get the depth texture data from the gpu
	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
	//linerise each pixel value
	float zNear = 0.1f;
	float zFar = 100.f;
	for (unsigned int i = 0; i < m_windowWidth * m_windowHeight; ++i)
	{
		float depthSample = pixels[i];
		float zLinear = (2.0f * zNear) / (zFar + zNear - depthSample * (zFar - zNear));
		pixels[i] = zLinear;
	}

	glBindTexture(GL_TEXTURE_2D, m_fbo_linear_depth);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_windowWidth,
		m_windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] pixels;
#pragma endregion

	//bind the framebuffer for rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Clear out everything on the back buffer, this is now our render target
		
#pragma region Fullscreen Render Pass

	glUseProgram(m_programID);

	//pass our camera position to our fragment shader uniform
	glm::vec3 cm = (viewMatrix * m_cameraMatrix[3]).xyz;
	cameraPosUniform = glGetUniformLocation(m_programID, "camera_pos");
	glUniform3fv(cameraPosUniform, 1, glm::value_ptr(cm));

	glm::vec3 ld = (viewMatrix * lightDir).xyz;
	lightDirUniform = glGetUniformLocation(m_programID, "lightDir");
	glUniform3fv(lightDirUniform, 1, glm::value_ptr(ld));

	glm::vec2 screenDim = glm::vec2(m_windowWidth, m_windowHeight);
	unsigned int screenSizeUniform = glGetUniformLocation(m_programID, "screenSize");
	glUniform2fv(screenSizeUniform, 1, glm::value_ptr(screenDim));

	
	glBindVertexArray(m_vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo[1]);
	unsigned int texUniformID = glGetUniformLocation(m_programID, "DiffuseTexture");
	glUniform1i(texUniformID, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_diffuse_texture);
	texUniformID = glGetUniformLocation(m_programID, "NormalTexture");
	glUniform1i(texUniformID, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_fbo_normal_texture);
	texUniformID = glGetUniformLocation(m_programID, "WorldTexture");
	glUniform1i(texUniformID, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_fbo_worldspace_texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);

#pragma endregion

}

void SSAO::Destroy()
{

	m_fbxModel->unload();
	delete m_fbxModel;

	glDeleteBuffers(2, m_vbo);
	glDeleteBuffers(2, m_ibo);
	glDeleteVertexArrays(2, m_vao);
	glDeleteProgram(m_programID);
	glDeleteShader(m_fragmentShader);
	glDeleteShader(m_vertexShader);
	glDeleteProgram(m_fbo_program);
	glDeleteShader(m_fbo_fragmentShader);
	glDeleteShader(m_fbo_vertexShader);

	Gizmos::destroy();
}

bool SSAO::LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID)
{
	int width = 0, height = 0, channels = 0;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(a_filePath.c_str(), &width, &height, &channels, 0);
	if (imageData != nullptr)
	{
		a_textureID = 0;
		glGenTextures(1, &a_textureID);

		glBindTexture(GL_TEXTURE_2D, a_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (channels == 1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R, width, height, 0, GL_R, GL_UNSIGNED_BYTE, imageData);
		}
		if (channels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(imageData);
		Application_Log* log = Application_Log::Get();
		if (log != nullptr)
		{
			log->addLog(LOG_LEVEL::LOG_ERROR, "Successfully loaded texture : %s", a_filePath.c_str());
		}
		return true;
	}
	else
	{
		Application_Log* log = Application_Log::Get();
		if (log != nullptr)
		{
			log->addLog(LOG_LEVEL::LOG_ERROR, "Failed to load texture : %s", a_filePath.c_str());
		}
	}
	return false;
}

