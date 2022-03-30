#ifndef __Application_H_
#define __Application_H_

#include "Application.h"
#include <glm/glm.hpp>
#include <string>

// Derived application class that wraps up all globals neatly
class GeometryShdrTUT : public Application
{
public:

	GeometryShdrTUT();
	virtual ~GeometryShdrTUT();

protected:

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();

	bool LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID);
	
	struct SVertex
	{
		glm::vec4 pos;
		glm::vec4 colour;
	};

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int	m_programID;
	unsigned int	m_vertexShader;
	unsigned int	m_geometryShader;
	unsigned int	m_fragmentShader;

	unsigned int m_maxIndices;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	//Texture Handle for OpenGL
	unsigned int m_textureID;

	SVertex* m_vertices;

	int		m_currentState;
};

#endif // __Application_H_