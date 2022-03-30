#include "stdafx.h"
#include "..\include\PointLight.h"

PointLight::PointLight()
{
	lightPos = glm::vec4(0.36f, 1.7f, 5.47f, 0.f);
	constant = 0.1f;
	linear = 0.1f;
	quadratic = 3.07f;

	colour = glm::vec3(colourR, colourG, colourB); //1 0.46 0.18
	ambient = glm::vec3(0.325f, 0.225f, 0.125f);
	diffuse = glm::vec3(1.f);
	specular = glm::vec3(0.7f);

}

PointLight::~PointLight()
{
}

void PointLight::Update()
{
	//imgui ui settings for point light

	ImGui::Begin("Light");
	ImGui::Text("Point Light");
	ImGui::SliderFloat("Point Colour r", &colourR, 0, 1.0f);
	ImGui::SliderFloat("Point Colour g", &colourG, 0, 1.0f);
	ImGui::SliderFloat("Point Colour b", &colourB, 0, 1.0f);
	float pos[] = { lightPos.x, lightPos.y, lightPos.z };
	ImGui::SliderFloat3("Point Position", pos, -10.0f, 10.0f);
	ImGui::SliderFloat("Point Light Strenght", &constant, 0.1f, 10.0f);
	ImGui::SliderFloat("Point Light Distance", &linear, 0.1f, 10.0f);
	ImGui::SliderFloat("Point Light Blend", &quadratic, 0.1f, 10.0f);
	onOff = ImGui::Button("Point off/on");
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

}

void PointLight::Draw()
{
	//draws a box where the light is
	Gizmos::addBox(lightPos.xyz, glm::vec3(0.2f, 0.2f, 0.2f), true, glm::vec4(colourR, colourG, colourB, 1.f));
}
