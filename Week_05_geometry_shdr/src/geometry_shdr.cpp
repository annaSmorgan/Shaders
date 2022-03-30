#include "geometry_shdr.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include <imgui.h>

#include "Application_Log.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <stb_image.h>





GeometryShdr::GeometryShdr()
{

}

GeometryShdr::~GeometryShdr()
{

}

bool GeometryShdr::onCreate()
{
	m_currentState = 0; //state 0 is draw a triangle state 1 is draw a box
	// initialise the Gizmos helper class
	Gizmos::create();
	//limit the maximum vertices we can have
	m_maxIndices = 4;
	LoadImageFromFile("../resources/images/position.png", m_randomTexture);
#pragma region TRANSFORM FEEDBACK
	m_tf_curr = 0;
	m_tf_prev = 1;

	m_tfVertexShader = Utility::loadShader("../resources/shaders/tf_vertex.glsl", GL_VERTEX_SHADER);
	m_tfGeometryShader = Utility::loadShader("../resources/shaders/tf_geom.glsl", GL_GEOMETRY_SHADER);
	const char* tfInputs[] = { "particleType", "particlePos", "particleVelocity", "particleAge" };
	const char* tfOutputs[] = { "Type", "Position", "Velocity", "Age" };
	m_tfProgram = Utility::createTransformFeedbackProgram(m_tfVertexShader, 0,
		0, m_tfGeometryShader, 4, tfInputs, 4, tfOutputs);

	struct Particle
	{
		float Type;
		glm::vec4 Pos;
		glm::vec4 Vel;
		float Age;
	};
	Particle launcher;
	launcher.Type = 0.f;
	launcher.Pos = glm::vec4(0.f, 0.f, 0.f, 1.f);
	launcher.Vel = glm::vec4(0.f, 1.f, 0.f, 0.f);
	launcher.Age = 0.f;

	//tx feedback buffers
	glGenTransformFeedbacks(2, m_tf);
	glGenVertexArrays(2, m_tfVAO);
	glGenBuffers(2, m_vb);
	srand(time(nullptr));
	for (int i = 0; i < 2; ++i)
	{
		glBindVertexArray(m_tfVAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_vb[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * 1000, nullptr, GL_DYNAMIC_DRAW);
		for (int j = 0; j < 10; ++j)
		{
			glm::vec4 rotVel = glm::vec4(glm::rotate(glm::vec3(0.f, 0.5f, 0.f), j * 1.f, glm::vec3(1.f, 0.f, 0.f)), 0.f);
			launcher.Vel = rotVel;
			glBufferSubData(GL_ARRAY_BUFFER, j * sizeof(Particle), sizeof(Particle), &launcher);
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)20);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_tf[i]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vb[i]);

	}

	m_particleCount = 0;
	//atomic counter to track particles
	glGenBuffers(1, &m_atomicBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &m_particleCount, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	glGenBuffers(1, &m_acReadBuffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, m_acReadBuffer);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_READ);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

	glBindVertexArray(0);
#pragma endregion
	//Load the shaders for this program
	m_vertexShader = Utility::loadShader("../resources/shaders/vertex.glsl", GL_VERTEX_SHADER);
	m_fragmentShader = Utility::loadShader("../resources/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	//here we have an additional shader to load and bind the geometry shader
	//This is the only change to our loading code for this example as the geometry shader
	//feeds into the fragment shader the outputs remain the same.
	m_geometryShader = Utility::loadShader("../resources/shaders/geometry.glsl", GL_GEOMETRY_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs[] = { "particleType", "particlePos", "particleVelocity", "particleAge" };
	const char* szOutputs[] = { "FragColor" };
	//bind the shaders to create our shader program
	m_programID = Utility::createProgram(
		m_vertexShader,
		0,
		0,
		m_geometryShader,
		m_fragmentShader,
		4, szInputs, 1, szOutputs);

	
	LoadImageFromFile("../resources/images/starAlpha.png", m_textureID);
	
	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );
	
	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth/(float)m_windowHeight, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void GeometryShdr::Update(float a_deltaTime)
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );
	// add a 20x20 grid on the XZ-plane
	for ( int i = 0 ; i < 21 ; ++i )
	{
		Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
		
		Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	}

	if (ImGui::Begin("Particle Details"))
	{
		ImGui::Text("Particle Count: %u", m_particleCount);
	}
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
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void GeometryShdr::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);

	glUseProgram(m_tfProgram);
	//no frag shader so disable rasterisation
	glEnable(GL_RASTERIZER_DISCARD);
	unsigned int deltaT = glGetUniformLocation(m_tfProgram, "deltaTime");
	glUniform1f(deltaT, Utility::getDeltaTime());
	unsigned int totalT = glGetUniformLocation(m_tfProgram, "totalTime");
	glUniform1f(totalT, Utility::getTotalTime());
	unsigned int randTexUniformID = glGetUniformLocation(m_tfProgram, "randomTexture");
	glUniform1i(randTexUniformID, 0);
	//set active texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_randomTexture);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicBuffer);
	glBindVertexArray(m_tfVAO[m_tf_curr]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_tf[m_tf_prev]);

	glBeginTransformFeedback(GL_POINTS);

	static bool firstDraw = true;
	if (firstDraw)
	{
		glDrawArrays(GL_POINTS, 0, 10);
		firstDraw = false;
	}
	else
	{
		glDrawTransformFeedback(GL_POINTS, m_tf[m_tf_curr]);
	}
	glEndTransformFeedback();
	glBindVertexArray(0);
	//turn rasterisation back on
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicBuffer);

	//read counter through temporary buyffer and reset counter
	glBindBuffer(GL_COPY_WRITE_BUFFER, m_acReadBuffer);
	glCopyBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLuint));
	unsigned int* counterValue = (unsigned int*)glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, 
		sizeof(GLuint), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	m_particleCount = counterValue[0];
	counterValue[0] = 0;
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_ATOMIC_COUNTER_BUFFER, 0, 0, sizeof(GLuint));
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glUseProgram(0);
	   	  
	//bing our shader program
	glUseProgram(m_programID);
	

	//get our shaders uniform location for our projectionViewMatrix and then use glUniformMatrix4fv to fill it with the correct data
	unsigned int projectionViewUniform = glGetUniformLocation(m_programID, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));

	unsigned int cameraPosUniform = glGetUniformLocation(m_programID, "cameraPosition");
	glUniform4fv(cameraPosUniform, 1, glm::value_ptr(m_cameraMatrix[3]));

	//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
	unsigned int texUniformID = glGetUniformLocation(m_programID, "textureLoc");
	glUniform1i(texUniformID, 0);
	//set our active texture, and bind our loaded texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//bind our vertex array object
	glBindVertexArray(m_tfVAO[m_tf_prev]);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);
	glBlendEquation(GL_MAX);

	//depending on the state call draw with glDrawElements to draw our buffer
	//glDrawElements uses the index array in our index buffer to draw the vertices in our vertex buffer
	glDrawTransformFeedback(GL_POINTS, m_tf[m_tf_prev]);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glBindVertexArray(0);
	glUseProgram(0);
	
	m_tf_curr = m_tf_prev;
	m_tf_prev = (m_tf_prev + 1) & 0x1;

}

void GeometryShdr::Destroy()
{

	Gizmos::destroy();
	glDeleteTextures(1, &m_textureID);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_programID);
	glDeleteShader(m_fragmentShader);
	glDeleteShader(m_geometryShader);
	glDeleteShader(m_vertexShader);
}

bool GeometryShdr::LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID)
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

