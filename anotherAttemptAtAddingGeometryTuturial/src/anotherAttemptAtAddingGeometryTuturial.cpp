#include "anotherAttemptAtAddingGeometryTuturial.h"
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

anotherAttemptAtAddingGeometryTuturial::anotherAttemptAtAddingGeometryTuturial()
{

}

anotherAttemptAtAddingGeometryTuturial::~anotherAttemptAtAddingGeometryTuturial()
{

}

bool anotherAttemptAtAddingGeometryTuturial::onCreate()
{
	m_currentState = 0; //state 0 is draw a triangle state 1 is draw a box
	// initialise the Gizmos helper class
	Gizmos::create();
	//limit the maximum vertices we can have
	m_maxIndices = 4;
	//Load the shaders for this program
	m_vertexShader = Utility::loadShader("../resources/shaders/vertex.glsl", GL_VERTEX_SHADER);
	m_fragmentShader = Utility::loadShader("../resources/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	//here we have an additional shader to load and bind the geometry shader
	//This is the only change to our loading code for this example as the geometry shader
	//feeds into the fragment shader the outputs remain the same.
	m_geometryShader = Utility::loadShader("../resources/shaders/geometry.glsl", GL_GEOMETRY_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs[] = { "Position", "Colour" };
	const char* szOutputs[] = { "FragColor" };
	//bind the shaders to create our shader program
	m_programID = Utility::createProgram(
		m_vertexShader,
		0,
		0,
		m_geometryShader,
		m_fragmentShader,
		2, szInputs, 1, szOutputs);

	m_vertices = new SVertex[4];

	//Generate and bind our Vertex Array Object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//Generate our Vertex Buffer Object
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SVertex), m_vertices, GL_DYNAMIC_DRAW);
	//Define our vertex attribute data for this buffer we have to do this for our vertex input data
	//Position, Normal & Colour this attribute order is taken from our szInputs string array order
	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //colour
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(SVertex), ((char*)0) + 16);

	//generate and bind an index buffer
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	//the thrid argument of glBufferData is the data that will fill this buffer here it is set to nullptr
	//so this buffer will just have empty data in it
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxIndices * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);

	glBindVertexArray(0);
	//unbind our current vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//unbind our current index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Set our vertex and Index buffers in here as they don't get altered for this programs run through
	//set the vertices for our billboard -- there is no reason this couldn't be in the init function 
	//as these vertice data do not change over time. 

	m_vertices[0].pos = glm::vec4(-4.f, 0.f, 0.f, 1.f);
	m_vertices[0].colour = glm::vec4(1.f, 0.f, 0.f, 1.f);

	m_vertices[1].pos = glm::vec4(4.f, 0.f, 0.f, 1.f);
	m_vertices[1].colour = glm::vec4(0.f, 1.f, 0.f, 1.f);

	m_vertices[2].pos = glm::vec4(-4.f, 4.f, 0.f, 1.f);
	m_vertices[2].colour = glm::vec4(0.f, 0.f, 1.f, 1.f);

	m_vertices[3].pos = glm::vec4(4.f, 4.f, 0.f, 1.f);
	m_vertices[3].colour = glm::vec4(1.f, 1.f, 1.f, 1.f);

	//bind our vertex buffer and fill it with our mertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SVertex), m_vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//bind our index buffer and specify the indices order in the indices array		
	unsigned int indices[] = { 0, 1, 2, 3 };
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	LoadImageFromFile("../resources/images/starAlpha.png", m_textureID);

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void anotherAttemptAtAddingGeometryTuturial::Update(float a_deltaTime)
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
	// add a 20x20 grid on the XZ-plane
	for (int i = 0; i < 21; ++i)
	{
		Gizmos::addLine(glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));

		Gizmos::addLine(glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));
	}


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

void anotherAttemptAtAddingGeometryTuturial::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);

	//bing our shader program
	glUseProgram(m_programID);
	//bind our vertex array object
	glBindVertexArray(m_vao);


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

	//depending on the state call draw with glDrawElements to draw our buffer
	//glDrawElements uses the index array in our index buffer to draw the vertices in our vertex buffer
	glDrawElements(GL_POINTS, m_maxIndices, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
	

}

void anotherAttemptAtAddingGeometryTuturial::Destroy()
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

bool anotherAttemptAtAddingGeometryTuturial::LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID)
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

