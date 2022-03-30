#ifndef __DirLight_H_
#define __DirLight_H_

#include <glm/glm.hpp>


class DirLight
{
public:

	DirLight();
	virtual ~DirLight();

	//public variables and functions for use in sending light data to shader
	void Update(float a_deltaTime);
	void Draw();

	glm::vec4 lightPos;
	glm::vec4 direction;

	glm::vec3 colour;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

private:
	float colourR = 0.5f;
	float colourG = 0.3f;
	float colourB = 0.2f;

	bool onOff;

};

#endif