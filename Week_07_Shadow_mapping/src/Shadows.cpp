#include "Shadows.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include <imgui.h>
#include "Application_Log.h"

#define SHADOW_BUFFER_SIZE 1024

Shadows::Shadows()
{

}

Shadows::~Shadows()
{

}


bool Shadows::onCreate()
{
	// initialise the Gizmos helper class
	Gizmos::create();

	//Load the shaders for this program
	unsigned int vertexShader = Utility::loadShader("./resources/shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragmentShader = Utility::loadShader("./resources/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs[] = { "Position", "Colour", "Normal","Tex1" };
	const char* szOutputs[] = { "FragColor" };
	//bind the shaders to create our shader program
	m_programID = Utility::createProgram(
		vertexShader,
		0,
		0,
		0,
		fragmentShader,
		4, szInputs, 1, szOutputs);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Load the shadow shaders for this program
	vertexShader = Utility::loadShader("./resources/shaders/vertex_shadow.glsl", GL_VERTEX_SHADER);
	fragmentShader = Utility::loadShader("./resources/shaders/fragment_shadow.glsl", GL_FRAGMENT_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs2[] = { "Position", "Colour", "Normal","Tex1" };
	const char* szOutputs2[] = { "FragDepth" };
	//bind the shaders to create our shader program
	m_shadowProgramID = Utility::createProgram(
		vertexShader,
		0,
		0,
		0,
		fragmentShader,
		4, szInputs2, 1, szOutputs2);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_fbxModel = new FBXFile();
	m_fbxModel->load("./resources/models/ruinedtank/tank.fbx", FBXFile::UNITS_DECIMETER);

	//Generate our OpenGL Vertex and Index Buffers for rendering our FBX Model Data
	// OPENGL: genorate the VBO, IBO and VAO
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);
	glGenVertexArrays(1, &m_vao);

	// OPENGL: Bind  VAO, and then bind the VBO and IBO to the VAO
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	//There is no need to populate the vbo & ibo buffers with any data at this stage
	//this can be done when rendering each mesh component of the FBX model

	// enable the attribute locations that will be used on our shaders
	glEnableVertexAttribArray(0); //Pos
	glEnableVertexAttribArray(1); //Col
	glEnableVertexAttribArray(2); //Norm
	glEnableVertexAttribArray(3); //Tex1


								  // tell our shaders where the information within our buffers lie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), ((char *)0) + FBXVertex::PositionOffset);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::ColourOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::NormalOffset);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char *)0) + FBXVertex::TexCoord1Offset);

	// finally, where done describing our mesh to the shader
	// we can describe the next mesh
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//\===========================================================================================================
	//\Set up the frame buffer for rendering our shadows into
	m_FBO_texture = -1;
	SetupFrameBuffer(m_FBO, SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE, m_FBO_texture, m_FBO_depth_texture);
	//\===========================================================================================================
	//create a texture to hold the linear depth buffer samples
	//texture for linear depth buffer visualisation
	glGenTextures(1, &m_FBO_Linear_Depth);

	// bind the texture for editing
	glBindTexture(GL_TEXTURE_2D, m_FBO_Linear_Depth);

	// create the texture. Note, the width and height are the dimensions of the screen and the final
	// data pointer is 0 as we aren't including any initial data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_windowWidth, m_windowHeight, 0, GL_RED, GL_FLOAT, 0);

	// set the filtering if we intend to sample within a shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//\===========================================================================================================

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, (float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);

	m_modelMatrix = glm::mat4();

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_lightPos = glm::vec4(50.f, 25.f, 0.f, 1.f);

	return true;
}

void Shadows::Update(float a_deltaTime)
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

	//Update the shadow projection view matrix 
	glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(m_lightPos.xyz),
		glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, -100, 100);

	m_shadowProjectionViewMatrix = depthProjectionMatrix * depthViewMatrix;

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
	ImGui::Begin("Frame Rate");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(m_windowWidth - m_windowWidth *0.3, m_windowHeight - m_windowHeight*0.4));
	ImGui::SetNextWindowSize(ImVec2(m_windowWidth*0.3, m_windowHeight* 0.4));
	ImGui::Begin("Framebuffer");
	ImGui::BeginTabBar("FrameBuffer Textures");
	
	//ImGui::DrawTabsBackground();
	if (ImGui::BeginTabItem("Colour Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_FBO_texture;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.25, m_windowHeight*0.25), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Depth Buffer")) {
		ImTextureID texID = (void *)(intptr_t)m_FBO_Linear_Depth;
		ImGui::Image(texID, ImVec2(m_windowWidth*0.25, m_windowHeight*0.25), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();


	ImGui::End();

	static bool show_demo_window = true;
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

void Shadows::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//\===============================================================================
	//\ Draw the scene from the POV of the light
	//\===============================================================================
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE);
	DrawScene(m_shadowProgramID);

	//\===============================================================================
	//\ Draw the scene from the regular POV of the camera
	//\===============================================================================
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	DrawScene(m_programID);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);
	//\===============================================================================
	

}

void Shadows::DrawScene(unsigned int a_programID)
{
	//bing our shader program
	glUseProgram(a_programID);
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	//bind our vertex array object
	glBindVertexArray(m_vao);

	unsigned int shadowProjectionViewUniform = glGetUniformLocation(a_programID, "ShadowProjectionView");
	glUniformMatrix4fv(shadowProjectionViewUniform, 1, false, glm::value_ptr(m_shadowProjectionViewMatrix));

	//get our shaders uniform location for our projectionViewMatrix and then use glUniformMatrix4fv to fill it with the correct data
	unsigned int projectionViewUniform = glGetUniformLocation(a_programID, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));

	//pass throught the view matrix
	unsigned int viewMatrixUniform = glGetUniformLocation(a_programID, "ViewMatrix");
	glUniformMatrix4fv(viewMatrixUniform, 1, false, glm::value_ptr(viewMatrix));

	//pass our camera position to our fragment shader uniform
	unsigned int cameraPosUniform = glGetUniformLocation(a_programID, "cameraPosition");
	glUniform4fv(cameraPosUniform, 1, glm::value_ptr(m_cameraMatrix[3]));

	//pass the directional light direction to our fragment shader
	glm::vec4 lightDir = -m_lightPos;
	lightDir.w = 0.f;
	lightDir = glm::normalize(lightDir);
	unsigned int lightDirUniform = glGetUniformLocation(a_programID, "lightDirection");
	glUniform4fv(lightDirUniform, 1, glm::value_ptr(lightDir));

	//Set the shadow texture
	unsigned int shadowTexUniformID = glGetUniformLocation(a_programID, "ShadowTexture");
	glUniform1i(shadowTexUniformID, 1);
	//Set our active texture, and bind our loaded texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_FBO_depth_texture);

	//Draw our FBX Model
	// for each mesh in the model...
	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode* pMesh = m_fbxModel->getMeshByIndex(i);

		// send the Model
		glm::mat4 m4Model = pMesh->m_globalTransform;// *m_modelMatrix;
		unsigned int modelUniform = glGetUniformLocation(a_programID, "Model");
		glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(m4Model));
		//send the normal matrix
		//this is only beneficial if a model has a non-uniform scale or non-orthoganal model matrix
		glm::mat4 m4Normal = glm::transpose(glm::inverse(pMesh->m_globalTransform));// *m_modelMatrix;
		unsigned int normalMatrixUniform = glGetUniformLocation(a_programID, "NormalMatrix");
		glUniformMatrix4fv(normalMatrixUniform, 1, false, glm::value_ptr(m4Normal));

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
		unsigned int texUniformID = glGetUniformLocation(a_programID, "DiffuseTexture");
		glUniform1i(texUniformID, 0);
		//set our active texture, and bind our loaded texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::DiffuseTexture]);

		// Send the vertex data to the VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
	glUseProgram(0);

}

void Shadows::Destroy()
{

	m_fbxModel->unload();
	delete m_fbxModel;

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_programID);
	
	Gizmos::destroy();
}

void Shadows::SetupFrameBuffer(unsigned int& a_fbo, 
	unsigned int a_targetWidth,
	unsigned int a_targetHeight,
	unsigned int& a_renderTexture,
	unsigned int& a_depthTexture)
{
	//\======================================================================================
	// Create our frame buffer object
	//\=====================================================================================
	// this would be within your onCreate() function
	glGenFramebuffers(1, &a_fbo);

	// bind the framebuffer for editing
	glBindFramebuffer(GL_FRAMEBUFFER, a_fbo);

	if (a_renderTexture != -1)
	{
		// create a texture to be attached to the framebuffer, stored in the derived app class as a member
		glGenTextures(1, &a_renderTexture);

		// bind the texture for editing
		glBindTexture(GL_TEXTURE_2D, a_renderTexture);

		// create the texture. Note, the width and height are the dimensions of the screen and the final
		// data pointer is 0 as we aren't including any initial data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a_targetWidth, a_targetHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// set the filtering if we intend to sample within a shader
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, a_renderTexture, 0);
	}
	// m_FBO_depth would be a member for the derived application class
	glGenTextures(1, &a_depthTexture);
	glBindTexture(GL_TEXTURE_2D, a_depthTexture);

	// note the use of GL_DEPTH_COMPONENT32F and GL_DEPTH_COMPONENT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, a_targetWidth,
		a_targetHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// attach it as a GL_DEPTH_ATTACHMENT
	// attach the texture to the 0th color attachment of the framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, a_depthTexture, 0);

	// Here we tell the framebuffer which color attachments we will be drawing to and how many
	if (a_renderTexture != -1)
	{
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
	}
	else
	{
		glDrawBuffer(GL_NONE);
	}
	// if Status doesn't equal GL_FRAMEBUFFER_COMPLETE there has been an error when creating it
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer Error!\n");

	// binding 0 to the framebuffer slot unbinds the framebuffer and means future render calls will be sent to 
	// the back buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

