#ifndef __week_001VAO_H_
#define __week_001VAO_H_

#include "Application.h"
#include <glm/glm.hpp>

// Derived application class that wraps up all globals neatly
class week_001VAO : public Application
{
public:

	week_001VAO();
	virtual ~week_001VAO();

protected:

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();

	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	};

	Vertex* m_vertices;

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int m_programID;
	unsigned int m_vertexShader;
	unsigned int m_fragmentShader;

	unsigned int m_maxVerts;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

};

#endif // __week_001VAO_H_