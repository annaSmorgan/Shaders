#include "week_001VAO.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <iostream>
#include <imgui.h>

#include "Application_Log.h"

week_001VAO::week_001VAO()
{

}

week_001VAO::~week_001VAO()
{

}

bool week_001VAO::onCreate()
{
	// initialise the Gizmos helper class
	Gizmos::create();

	m_maxVerts = 36;

	//load shader
	m_vertexShader = Utility::loadShader("../shaders/vertex.glsl", GL_VERTEX_SHADER);
	m_fragmentShader = Utility::loadShader("../shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	
	//define shader input and output
	const char* szInputs[] = { "position", "colour" };
	const char* szOuputs[] = { "fragColour" };

	//link shaders together to make program
	m_programID = Utility::createProgram(m_vertexShader, 0, 0, 0, m_fragmentShader, 2, szInputs, 1, szOuputs);

	//create vertices for cube
	m_vertices = new Vertex[8];
	//make a 2 unit cube 
	glm::vec4 vX(2.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 vY(0.0f, 2.0f, 0.0f, 0.0f);
	glm::vec4 vZ(0.0f, 0.0f, 2.0f, 0.0f);

	//centre of cube at following position
	glm::vec4 centre(0.0f, 0.0f, 0.0f, 1.0f);

	//create back part of cube
	m_vertices[0].position = centre - vX - vY - vZ;
	m_vertices[1].position = centre - vX + vY - vZ;
	m_vertices[2].position = centre + vX + vY - vZ;
	m_vertices[3].position = centre + vX - vY - vZ;

	//create front part of cube
	m_vertices[4].position = centre - vX - vY + vZ;
	m_vertices[5].position = centre - vX + vY + vZ;
	m_vertices[6].position = centre + vX + vY + vZ;
	m_vertices[7].position = centre + vX - vY + vZ;

	//colour
	for (int i = 0; i < 8; ++i)
	{
		m_vertices[i].colour = glm::vec4(((float)(i % 3) / 3.0f), ((float)(i % 4) / 4.0f), ((float)(i % 7) / 7), 1.0f);
	}

	//indicies 
	unsigned int indicies[] = {
	
		2, 1, 0, 3, 2, 0,
		5, 6, 4, 6, 7, 4, 
		4, 3, 0, 7, 3, 4,
		1, 2, 5, 2, 6, 5,
		0, 1, 4, 1, 5, 4,
		2, 3, 7, 6, 2, 7

	};

	//generate vertex array object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//generate and bind vertex buffer
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vertex), m_vertices, GL_DYNAMIC_DRAW);
	
	//set attributes for out buffer
	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //colour
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	//generate and define our index buffer
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxVerts * sizeof(unsigned int), indicies, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

void week_001VAO::Update(float a_deltaTime)
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

void week_001VAO::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);
	
	//use the shaders
	glUseProgram(m_programID);

	//bind out vao to get the buffers we previously had bound 
	glBindVertexArray(m_vao);

	//set shader uniform data from cpu to gpu
	unsigned int ProjectViewUniformLocation = glGetUniformLocation(m_programID, "ProjectionView");
	if (ProjectViewUniformLocation < 0)
	{
		Application_Log* log = Application_Log::Create();
		if (log != nullptr)
		{

			log->addLog(LOG_LEVEL::LOG_WARNING, "Warning: Uniform Location: ProjectionView not found in shader!\n");
		}
	}
	glUniformMatrix4fv(ProjectViewUniformLocation, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));
	glCullFace(GL_FRONT);
	glDrawElements(GL_TRIANGLES, m_maxVerts, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);

}

void week_001VAO::Destroy()
{

	Gizmos::destroy();

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);

	glDeleteProgram(m_programID);
	glDeleteShader(m_vertexShader);
	glDeleteShader(m_fragmentShader);
}

