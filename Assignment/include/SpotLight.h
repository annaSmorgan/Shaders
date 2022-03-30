#ifndef __SpotLight_H_
#define __SpotLight_H_

#include <glm/glm.hpp>


class SpotLight
{
public:

	SpotLight();
	virtual ~SpotLight();

	//public variables and functions for use in sending light data to shader
	void Update();
	void Draw();

	glm::vec4 lightPos;
	glm::vec4 direction;
	float constant;
	float linear;
	float quadratic;
	float angle;

	glm::vec3 colour;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	
private:
	float colourR = 1.0f;
	float colourG = 0.71f;
	float colourB = 0.28f;

	float posX = 0.f;
	float posY = 0.f;
	float posZ = 0.f;

	bool onOff;
};

#endif