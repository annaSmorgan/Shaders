#ifndef __PointLight_H_
#define __PointLight_H_

#include <glm/glm.hpp>


class PointLight
{
public:

	PointLight();
	virtual ~PointLight();

	//public variables and functions for use in sending light data to shader
	void Update();
	void Draw();

	glm::vec4 lightPos;
	float constant;
	float linear;
	float quadratic;

	glm::vec3 colour;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

private:
	float colourR = 1.0f;
	float colourG = 0.46f;
	float colourB = 0.18f; 

	float posX = 0.f;
	float posY = 0.f;
	float posZ = 0.f;

	bool onOff;

};

#endif