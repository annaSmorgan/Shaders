#ifndef __Shadows_H_
#define __Shadows_H_

#include "Application.h"
#include <glm/glm.hpp>
#include "FBXFile.h"

// Derived application class that wraps up all globals neatly
class Shadows : public Application
{
public:

	Shadows();
	virtual ~Shadows();

protected:

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void DrawScene(unsigned int a_programID);
	virtual void Destroy();

	void SetupFrameBuffer(unsigned int& a_fbo, unsigned int a_fboWidth, unsigned int a_fboHeight, unsigned int& a_renderTexture, unsigned int& a_depthTexture);

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_shadowProjectionViewMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_modelMatrix;

	unsigned int	m_programID;
	unsigned int	m_shadowProgramID;
	
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	//frame buffer variables
	unsigned int m_FBO;
	unsigned int m_FBO_texture;
	unsigned int m_FBO_depth_texture;
	unsigned int m_FBO_Linear_Depth;


	//FBX Model File
	FBXFile* m_fbxModel;

	glm::vec4 m_lightPos;
};

#endif // __Application_H_