#include "stdafx.h"
#include "ShaderLoader.h"

//handles the standard standards used within tank render
ShaderLoader::ShaderLoader()
{
}

ShaderLoader::~ShaderLoader()
{
}

void ShaderLoader::LoadShaders()
{
	//Load the shaders for this program
	vertexShader = Utility::loadShader("./resources/shaders/vertex.glsl", GL_VERTEX_SHADER);
	fragmentShader = Utility::loadShader("./resources/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
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

	DeleteShaders();
}

void ShaderLoader::Destroy()
{
	glDeleteProgram(m_programID);
}

void ShaderLoader::DeleteShaders()
{
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
