#include "stdafx.h"
#include "..\include\SpotLight.h"

SpotLight::SpotLight()
{
	lightPos = glm::vec4(0.1f, 2.5f, 0, 0);
	constant = 1.0f;
	linear = 0.3f;
	quadratic = 0.1f;

	colour = glm::vec3(colourR, colourG, colourB);
	ambient = glm::vec3(0.325f, 0.225f, 0.125f);
	diffuse = glm::vec3(1.f);
	specular = glm::vec3(0.7f);

	angle = 0.78f;
	direction = glm::vec4(0, -1.0f, 0, 0);
}

SpotLight::~SpotLight()
{
}

void SpotLight::Update()
{


	ImGui::Begin("Light");
	ImGui::Text("Spot Light");
	ImGui::SliderFloat("Spot Colour r", &colourR, 0, 1.0f);
	ImGui::SliderFloat("Spot Colour g", &colourG, 0, 1.0f);
	ImGui::SliderFloat("Spot Colour b", &colourB, 0, 1.0f);
	float pos[] = { lightPos.x, lightPos.y, lightPos.z };
	ImGui::SliderFloat3("Spot Position", pos, 0.f, 10.0f);

	ImGui::SliderFloat("Spot Light Strenght", &constant, 0.f, 1.0f);
	ImGui::SliderFloat("Spot Light Linear", &linear, 0.f, 1.0f);
	ImGui::SliderFloat("Spot Light Blend", &quadratic, 0.1f, 1.0f);
	ImGui::SliderFloat("Spot Light Angle", &angle, -1.f, 1.0f);
	onOff = ImGui::Button("Spot off/on");
	ImGui::Separator();
	ImGui::End();

	if (onOff == true)
	{
		colourR = 0;
		colourG = 0;
		colourB = 0;
	}

	colour = glm::vec3(colourR, colourG, colourB);
	lightPos.x = pos[0];
	lightPos.y = pos[1];
	lightPos.z = pos[2];
	//lightPos = glm::vec4(posX, posY, posZ, 1.0f);
}

void SpotLight::Draw()
{
	Gizmos::addBox(lightPos.xyz, glm::vec3(0.2f, 0.2f, 0.2f), true, glm::vec4(colourR, colourG, colourB, 1.f));
}
