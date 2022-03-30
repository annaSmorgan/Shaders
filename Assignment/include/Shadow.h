#ifndef __Shadow_H_
#define __Shadow_H_

#include <glm/glm.hpp>

class Shadow
{
public:

	Shadow();
	~Shadow();

	//public functions
	void LoadShadowShaders();
	void ShadowProjectionMatrix(glm::vec4 a_lightPos);
	void DrawShadow(unsigned int a_programID);
	void DestroyShadow();

	//public variables accessable by other needed classes
	unsigned int	m_shadowProgramID;

private:

	//private functions
	void DeleteShaders();

	//private variables
	unsigned int vertexShader;
	unsigned int fragmentShader;

	glm::mat4	m_shadowProjectionViewMatrix;

};

#endif // __Shadow_H_