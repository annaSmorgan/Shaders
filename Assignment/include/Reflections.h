#ifndef __Reflections_H_
#define __Reflections_H_

#include "Application.h"
#include <glm/glm.hpp>
#include "FBXFile.h"

class Reflections  //doesnt work with two models
{
public:

	Reflections();
	virtual ~Reflections();

	//public functions used within tank render
	bool onCreate(unsigned int m_windowWidth, unsigned int m_windowHeight);
	void Update(float a_deltaTime, unsigned int m_windowWidth, unsigned int m_windowHeight);
	void Draw(unsigned int m_windowWidth, unsigned int m_windowHeight);
	void Destroy();

private:
	//variables used within reflections building of model
	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_modelMatrix;

	unsigned int	m_programID;
	unsigned int	m_vertexShader;
	unsigned int	m_fragmentShader;

	unsigned int m_vao[2];
	unsigned int m_vbo[2];
	unsigned int m_ibo[2];

	//FBO related variables
	unsigned int m_fbo;
	unsigned int m_fbo_texture;
	unsigned int m_fbo_depth_texture;
	unsigned int m_fbo_linear_depth;

	unsigned int	m_fbo_program;
	unsigned int	m_fbo_vertexShader;
	unsigned int	m_fbo_fragmentShader;

	struct SVertex
	{
		enum Offsets
		{
			PositionOffset = 0,
			TexCoord1Offset = PositionOffset + sizeof(glm::vec4)
		};

		glm::vec4 pos;
		glm::vec2 uv;

		SVertex() {}
		SVertex(glm::vec4 a_pos, glm::vec2 a_uv) : pos(a_pos), uv(a_uv) {}
	};

	//FBX Model File
	FBXFile* m_fbxModel;

	glm::vec4 m_lightPos;
};

#endif // __Application_H_ons_H_