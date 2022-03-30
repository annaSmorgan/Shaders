#include "week_02Fullscreen_fragment_shader.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include <imgui.h>

#include "Application_Log.h"

week_02Fullscreen_fragment_shader::week_02Fullscreen_fragment_shader()
{

}

week_02Fullscreen_fragment_shader::~week_02Fullscreen_fragment_shader()
{

}

bool week_02Fullscreen_fragment_shader::onCreate()
{
	//load in shaders
	m_vertexShader = Utility::loadShader("../shaders/vertex.glsl", GL_VERTEX_SHADER);
	m_fragmentShader = Utility::loadShader("../shaders/fragment.glsl", GL_FRAGMENT_SHADER);

	//define shader input and output
	const char* szInputs[] = { "position", "colour" };
	const char* szOuputs[] = { "fragColour" };

	//link shaders together to make program
	m_programID = Utility::createProgram(m_vertexShader, 0, 0, 0, m_fragmentShader, 2, szInputs, 1, szOuputs);

	//delete shaders
	glDeleteShader(m_vertexShader);
	glDeleteShader(m_fragmentShader);

	//create vertex data
	Vertex* verts = new Vertex[4];
	verts[0].position = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	verts[1].position = glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
	verts[2].position = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
	verts[3].position = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

	verts[0].colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	verts[1].colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	verts[2].colour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	verts[3].colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//indicies 
	unsigned int indicies[] = { 0, 2, 1, 3, 1, 2};

	//generate vertex array object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//generate and bind vertex buffer
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), verts, GL_STATIC_DRAW);

	//set attributes for out buffer
	glEnableVertexAttribArray(0); //position
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0);
	glEnableVertexAttribArray(1); //colour
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);
	delete[] verts;

	//generate and define our index buffer
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indicies, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void week_02Fullscreen_fragment_shader::Update(float a_deltaTime)
{
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

void week_02Fullscreen_fragment_shader::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//use the shaders
	glUseProgram(m_programID);

	//get uniform location 
	int timeUniformLocation = glGetUniformLocation(m_programID, "uTime");
	int resolutionUniformLocation = glGetUniformLocation(m_programID, "uResolution");
	//send uniform data
	glUniform1f(timeUniformLocation, Utility::getTotalTime());
	glUniform2fv(resolutionUniformLocation, 1 , glm::value_ptr(glm::vec2(m_windowWidth, m_windowHeight)));

	//bind out vao to get the buffers we previously had bound 
	glBindVertexArray(m_vao);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (char*)0);
	glBindVertexArray(0);
	glUseProgram(0);


}

void week_02Fullscreen_fragment_shader::Destroy()
{
	glDeleteProgram(m_programID);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);

}

