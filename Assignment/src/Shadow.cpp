#include "stdafx.h"
#include "Shadow.h"

//creates shadows around the model
Shadow::Shadow()
{
}

Shadow::~Shadow()
{
}

void Shadow::LoadShadowShaders()
{
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
	DeleteShaders();

}

void Shadow::ShadowProjectionMatrix(glm::vec4 a_lightPos)
{
	//Update the shadow projection view matrix 
	glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(a_lightPos.xyz),
		glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, -100, 100);

	m_shadowProjectionViewMatrix = depthProjectionMatrix * depthViewMatrix;
}

void Shadow::DrawShadow(unsigned int a_programID)
{
	//get shaders uniform
	unsigned int shadowProjectionViewUniform = glGetUniformLocation(a_programID, "ShadowProjectionView");
	glUniformMatrix4fv(shadowProjectionViewUniform, 1, false, glm::value_ptr(m_shadowProjectionViewMatrix));

	//Set the shadow texture
	unsigned int shadowTexUniformID = glGetUniformLocation(a_programID, "ShadowTexture");
	glUniform1i(shadowTexUniformID, 1);
}

void Shadow::DestroyShadow()
{
	glDeleteProgram(m_shadowProgramID);
}

void Shadow::DeleteShaders()
{
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
