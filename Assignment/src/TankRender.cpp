#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "stdafx.h"

#include "TankRender.h"
#include "ShaderLoader.h"
#include "FireParticles.h"
#include "DeferredRender.h"
#include "Model.h"
#include "PointLight.h"
#include "DirLight.h"
#include "SpotLight.h"


#define SHADOW_BUFFER_SIZE 1024

TankRender::TankRender()
{

}

TankRender::~TankRender()
{

}

bool TankRender::onCreate()
{
	// initialise the Gizmos helper class
	Gizmos::create();

	//pointer connections
	shaders = new ShaderLoader();
	shadow = new Shadow();
	particle = new FireParticles();
	models = new Model();
	water = new WaterTess();
	illumination = new DeferredRender();
	pointLight = new PointLight();
	dirLight = new DirLight();
	spotLight = new SpotLight();

	//load in the shaders
	shaders->LoadShaders();
	shadow->LoadShadowShaders();

	//load in our Texture

	LoadImageFromFile("./resources/images/position.png", m_randomTexture);
	LoadImageFromFile("./resources/images/starAlpha.png", m_textureID);
	LoadImageFromFile("./resources/images/burnedWood/kavicsok_COLOR.jpg", m_DiffuseTex);
	LoadImageFromFile("./resources/images/burnedWood/kavicsok_NRM.jpg", m_NormalTex);
	LoadImageFromFile("./resources/images/burnedWood/kavicsok_SPEC.jpg", m_SpecularTex);
	LoadImageFromFile("./resources/images/burnedWood/kavicsok_DISP.jpg", m_DisplacementTex);
	LoadImageFromFile("./resources/images/burnedWood/kavicsok_OCC.jpg", m_OcclusionTex);

	////the tank model has normal textures, these textures are not correctly associated with the model so they will need to be loaded in separatey
	//LoadImageFromFile("./models/ruinedtank/left_engine_norm.png", m_textureDRID[0]);
	//LoadImageFromFile("./models/ruinedtank/turret_norm.png", m_textureDRID[1]);

	//run the create function in the other classes
	models->Create();
	particle->onCreate(m_windowWidth, m_windowHeight, glm::vec4(-0.5f, -9.f, 5.f, 1.f));
	water->onCreate(m_windowWidth, m_windowHeight);
	//illumination->onCreate(m_windowWidth, m_windowHeight); not sure how to get it to work with a different model


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

	// set the clear colour and enable depth testing and backface culling
	glClearColor(backgroundR, backgroundG, backgroundB, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	return true;
}

void TankRender::Update(float a_deltaTime)
{
	glClearColor(backgroundR, backgroundG, backgroundB, 1.0f);

	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));

	//update all the lights
	pointLight->Update();

	dirLight->Update(a_deltaTime);

	spotLight->Update();

	//shadow projection matrix
	shadow->ShadowProjectionMatrix(dirLight->direction);

	particle->Update(a_deltaTime); //update the particles

	models->Update(a_deltaTime); //update the models

	water->Update(a_deltaTime); //update the tesselation


	//illumination->Update(a_deltaTime, m_windowWidth, m_windowHeight, m_window); not sure how to convert to new model

	//update UI with IMGUI
	DrawUI();

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

void TankRender::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pointLight->Draw();
	dirLight->Draw();
	spotLight->Draw();

	//\===============================================================================
	//\ Draw the scene from the POV of the light
	//\===============================================================================
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE);
	DrawScene(shadow->m_shadowProgramID);

	//\===============================================================================
	//\ Draw the scene from the regular POV of the camera
	//\===============================================================================
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	DrawScene(shaders->m_programID);
	DrawScene(water->t_programID);
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);
	//\===============================================================================

	particle->GeoDraw(m_randomTexture, m_textureID, m_cameraMatrix, m_projectionMatrix);
	//illumination->Draw(m_windowWidth, m_windowHeight); not sure how to convert to new model

}

void TankRender::DrawScene(unsigned int a_programID)
{
	//bing our shader program
	glUseProgram(a_programID);
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	//get our shaders uniform location for our projectionViewMatrix and then use glUniformMatrix4fv to fill it with the correct data
	unsigned int projectionViewUniform = glGetUniformLocation(a_programID, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));

	//pass throught the view matrix
	unsigned int viewMatrixUniform = glGetUniformLocation(a_programID, "ViewMatrix");
	glUniformMatrix4fv(viewMatrixUniform, 1, false, glm::value_ptr(viewMatrix));

	//pass our camera position to our fragment shader uniform
	unsigned int cameraPosUniform = glGetUniformLocation(a_programID, "cameraPosition");
	glUniform4fv(cameraPosUniform, 1, glm::value_ptr(m_cameraMatrix[3]));

	////pass the directional light direction to our fragment shader
	//glm::vec4 lightDir = -m_lightPos;
	//lightDir.w = 0.f;
	//lightDir = glm::normalize(lightDir);

	//direction light - sendidng over all the point light data to shaders
	unsigned int dirLightUniformDir = glGetUniformLocation(a_programID, "dirLight.direction");
	glUniform4fv(dirLightUniformDir, 1, glm::value_ptr(-dirLight->direction));
	unsigned int dirLightUniformAm = glGetUniformLocation(a_programID, "dirLight.lights.ambient");
	glUniform3fv(dirLightUniformAm, 1, glm::value_ptr(dirLight->ambient));
	unsigned int dirLightUniformDif = glGetUniformLocation(a_programID, "dirLight.lights.diffuse");
	glUniform3fv(dirLightUniformDif, 1, glm::value_ptr(dirLight->diffuse));
	unsigned int dirLightUniformSpec = glGetUniformLocation(a_programID, "dirLight.lights.specular");
	glUniform3fv(dirLightUniformSpec, 1, glm::value_ptr(dirLight->specular));
	unsigned int dirLightUniformCol = glGetUniformLocation(a_programID, "dirLight.lights.colour");
	glUniform3fv(dirLightUniformCol, 1, glm::value_ptr(dirLight->colour));

	//point light - sendidng over all the point light data to shaders
	unsigned int pointLightUniformAm = glGetUniformLocation(a_programID, "pointLight.plights.lights.ambient");
	glUniform3fv(pointLightUniformAm, 1, glm::value_ptr(pointLight->ambient));
	unsigned int pointLightUniformDif = glGetUniformLocation(a_programID, "pointLight.plights.lights.diffuse");
	glUniform3fv(pointLightUniformDif, 1, glm::value_ptr(pointLight->diffuse));
	unsigned int pointLightUniformSpec = glGetUniformLocation(a_programID, "pointLight.plights.lights.specular");
	glUniform3fv(pointLightUniformSpec, 1, glm::value_ptr(pointLight->specular));
	unsigned int pointLightUniformCol = glGetUniformLocation(a_programID, "pointLight.plights.lights.colour");
	glUniform3fv(pointLightUniformCol, 1, glm::value_ptr(pointLight->colour));
	unsigned int pointLightUniformPos = glGetUniformLocation(a_programID, "pointLight.plights.position");
	glUniform4fv(pointLightUniformPos, 1, glm::value_ptr(pointLight->lightPos));
	unsigned int pointLightUniformCon = glGetUniformLocation(a_programID, "pointLight.plights.constant");
	glUniform1f(pointLightUniformCon, pointLight->constant);
	unsigned int pointLightUniformLin = glGetUniformLocation(a_programID, "pointLight.plights.linear");
	glUniform1f(pointLightUniformLin, pointLight->linear);
	unsigned int pointLightUniformQu = glGetUniformLocation(a_programID, "pointLight.plights.quadratic");
	glUniform1f(pointLightUniformQu, pointLight->quadratic);
	unsigned int pointLightUniformAng = glGetUniformLocation(a_programID, "pointLight.angle");
	glUniform1f(pointLightUniformAng, -1);
	unsigned int pointLightUniformDir = glGetUniformLocation(a_programID, "pointLight.direction");
	glUniform4fv(pointLightUniformDir, 1, glm::value_ptr(spotLight->direction));

	//spot light - sendidng over all the point light data to shaders
	unsigned int spotLightUniformPos = glGetUniformLocation(a_programID, "spotLight.plights.position");
	glUniform4fv(spotLightUniformPos, 1, glm::value_ptr(spotLight->lightPos));
	unsigned int spotLightUniformAm = glGetUniformLocation(a_programID, "spotLight.plights.lights.ambient");
	glUniform3fv(spotLightUniformAm, 1, glm::value_ptr(spotLight->ambient));
	unsigned int spotLightUniformDif = glGetUniformLocation(a_programID, "spotLight.plights.lights.diffuse");
	glUniform3fv(spotLightUniformDif, 1, glm::value_ptr(spotLight->diffuse));
	unsigned int spotLightUniformSpec = glGetUniformLocation(a_programID, "spotLight.plights.lights.specular");
	glUniform3fv(spotLightUniformSpec, 1, glm::value_ptr(spotLight->specular));
	unsigned int spotLightUniformCol = glGetUniformLocation(a_programID, "spotLight.plights.lights.colour");
	glUniform3fv(spotLightUniformCol, 1, glm::value_ptr(spotLight->colour));
	unsigned int spotLightUniformCon = glGetUniformLocation(a_programID, "spotLight.plights.constant");
	glUniform1f(spotLightUniformCon, spotLight->constant);
	unsigned int spotLightUniformLin = glGetUniformLocation(a_programID, "spotLight.plights.linear");
	glUniform1f(spotLightUniformLin, spotLight->linear);
	unsigned int spotLightUniformQu = glGetUniformLocation(a_programID, "spotLight.plights.quadratic");
	glUniform1f(spotLightUniformQu, spotLight->quadratic);
	unsigned int spotLightUniformDir = glGetUniformLocation(a_programID, "spotLight.direction");
	glUniform4fv(spotLightUniformDir, 1, glm::value_ptr(spotLight->direction));
	unsigned int spotLightUniformAng = glGetUniformLocation(a_programID, "spotLight.angle");
	glUniform1f(spotLightUniformAng, spotLight->angle);

	//draw shadows
	shadow->DrawShadow(a_programID);
	models->Draw(a_programID, modelPosX, modelPosY, modelPosZ, modelScale);
	water->Draw(m_cameraMatrix, m_projectionMatrix, m_DiffuseTex, m_NormalTex, m_SpecularTex, m_DisplacementTex, m_OcclusionTex, a_programID);

	//Set our active texture, and bind our loaded texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_FBO_depth_texture);

	glBindVertexArray(0);
	glUseProgram(0);

}

void TankRender::Destroy()
{
	//destroy all the references within the other classes then delete the reference to them
	shaders->Destroy();
	shadow->DestroyShadow();
	particle->Destroy();
	models->Destroy();
	water->Destroy();
	//illumination->Destroy();

	delete shaders;
	delete shadow;
	delete particle;
	delete models;
	delete water;
	delete illumination;

	glDeleteTextures(1, &m_textureID);
	glDeleteTextures(1, &m_randomTexture);

	Gizmos::destroy();
}

void TankRender::SetupFrameBuffer(unsigned int& a_fbo,
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

bool TankRender::LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID) //loads image from file and saves it as a texture in texture id
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

void TankRender::DrawUI() //creates the boxes and settings on the ui using imgui
{
	ImGui::Begin("Frame Rate");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


	ImGui::SetNextWindowPos(ImVec2(m_windowWidth - m_windowWidth * 0.14, m_windowHeight - m_windowHeight * 1));
	ImGui::SetNextWindowSize(ImVec2(m_windowWidth * 0.14, m_windowHeight * 0.5));
	ImGui::Begin("Settings");

	ImGui::BeginChild("Framebuffer", ImVec2(m_windowWidth * 0.15, m_windowHeight * 0.14));
	ImGui::BeginTabBar("FrameBuffer Textures");

	//ImGui::DrawTabsBackground();
	if (ImGui::BeginTabItem("Colour Buffer")) {
		ImTextureID texID = (void*)(intptr_t)m_FBO_texture;
		ImGui::Image(texID, ImVec2(m_windowWidth * 0.13, m_windowHeight * 0.1), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Depth Buffer")) {
		ImTextureID texID = (void*)(intptr_t)m_FBO_Linear_Depth;
		ImGui::Image(texID, ImVec2(m_windowWidth * 0.13, m_windowHeight * 0.1), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();


	ImGui::EndChild();

	ImGui::BeginChild("Model", ImVec2(m_windowWidth * 0.15, m_windowHeight * 0.14));
	ImGui::Text("Model - Cat position");
	ImGui::InputFloat("X", &modelPosX, 1.0f);
	ImGui::InputFloat("Y", &modelPosY, 1.0f);
	ImGui::InputFloat("Z", &modelPosZ, 1.0f);
	ImGui::Text("Model - Cat scale");
	ImGui::InputFloat("Size", &modelScale, 1.0f);

	ImGui::EndChild();

	//ImGui::BeginChild("Light", ImVec2(m_windowWidth * 0.15, m_windowHeight * 0.1));
	//ImGui::Text("Light - Colour");
	//ImGui::SliderFloat("R", &colourR, 0.0f, 1.0f);
	//ImGui::SliderFloat("G", &colourG, 0.0f, 1.0f);
	//ImGui::SliderFloat("B", &colourB, 0.0f, 1.0f);

	//ImGui::EndChild();

	ImGui::BeginChild("Background", ImVec2(m_windowWidth * 0.15, m_windowHeight * 0.1));
	ImGui::Text("Background - Colour");
	ImGui::SliderFloat("R", &backgroundR, 0.0f, 1.0f);
	ImGui::SliderFloat("G", &backgroundG, 0.0f, 1.0f);
	ImGui::SliderFloat("B", &backgroundB, 0.0f, 1.0f);

	ImGui::EndChild();
	ImGui::End();
}