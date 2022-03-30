#ifndef __week_02Fullscreen_fragment_shader_H_
#define __week_02Fullscreen_fragment_shader_H_

#include "Application.h"
#include <glm/glm.hpp>

// Derived application class that wraps up all globals neatly
class week_02Fullscreen_fragment_shader : public Application
{
public:

	week_02Fullscreen_fragment_shader();
	virtual ~week_02Fullscreen_fragment_shader();

protected:

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();

	
	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int m_programID;
	unsigned int m_vertexShader;
	unsigned int m_fragmentShader;

	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	};
};

#endif // __week_02Fullscreen_fragment_shader_H_