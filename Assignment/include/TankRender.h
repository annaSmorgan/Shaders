#ifndef __Application_H_
#define __Application_H_

#include "Application.h"
#include "ShaderLoader.h"
#include "FireParticles.h"
#include "DeferredRender.h"
#include "Model.h"
#include "Shadow.h"
#include "WaterTess.h"
#include <glm/glm.hpp>
#include "FBXFile.h"

#include "PointLight.h"
#include "DirLight.h"
#include "SpotLight.h"


// Derived application class that wraps up all globals neatly
class TankRender : public Application
{
public:

	TankRender();
	virtual ~TankRender();

protected:
	//main functions
	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void DrawScene(unsigned int a_programID);
	virtual void Destroy();

	void SetupFrameBuffer(unsigned int& a_fbo, unsigned int a_fboWidth, unsigned int a_fboHeight, unsigned int& a_renderTexture, unsigned int& a_depthTexture);//sets up frame buffer
	bool LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID);//loads images using stbi
	void DrawUI();//draws the ui using imgui

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int	m_programID; //used for shaders

	//frame buffer variables
	unsigned int m_FBO;
	unsigned int m_FBO_texture;
	unsigned int m_FBO_depth_texture;
	unsigned int m_FBO_Linear_Depth;

	//Texture Handle for OpenGL
	unsigned int m_textureID;
	unsigned int m_randomTexture;
	unsigned int m_textureDRID[2];
	unsigned int m_DiffuseTex;
	unsigned int m_NormalTex;
	unsigned int m_SpecularTex;
	unsigned int m_OcclusionTex;
	unsigned int m_DisplacementTex;

	//imgui values
	float modelPosX = 0.0f;
	float modelPosY = 10.0f;
	float modelPosZ = -4.0f;
	float modelScale = 10.0f;

	float colourR = 0.325f;
	float colourG = 0.225f;
	float colourB = 0.125f;

	float backgroundR = 0.4f;
	float backgroundG = 0.4f;
	float backgroundB = 0.6f;


	//pointers
	Shadow* shadow;
	ShaderLoader* shaders;
	FireParticles* particle;
	DeferredRender* illumination;
	Model* models;
	WaterTess* water;
	PointLight* pointLight;
	DirLight* dirLight;
	SpotLight* spotLight;

};

#endif // __TankRender_H_