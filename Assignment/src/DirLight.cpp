#include "stdafx.h"
#include "..\include\DirLight.h"

DirLight::DirLight()
{
	lightPos = glm::vec4(50.f, 35.f, 0.f, 1.f);

	ambient = glm::vec3(0.325f, 0.225f, 0.125f);
	diffuse = glm::vec3(1.f);
	specular = glm::vec3(0.7f);

}

DirLight::~DirLight()
{
}

void DirLight::Update(float a_deltaTime)
{
	//rotates the directinoal light around the centre as well as using imgui to set up the ui 
	// rotate point
	float s = sinf(a_deltaTime * 0.2f);
	float c = cosf(a_deltaTime * 0.2f);
	glm::vec4 temp = lightPos;
	lightPos.x = temp.x * c - temp.z * s;
	lightPos.z = temp.x * s + temp.z * c;
	lightPos.w = 0;
	lightPos = glm::normalize(lightPos) * 25.f;

	ImGui::Begin("Light");
	ImGui::Text("Directional Light");
	ImGui::SliderFloat("Dir Colour r", &colourR, 0, 1.0f);
	ImGui::SliderFloat("Dir Colour g", &colourG, 0, 1.0f);
	ImGui::SliderFloat("Dir Colour b", &colourB, 0, 1.0f);
	onOff = ImGui::Button("Dir off/on");
	ImGui::Separator();
	ImGui::End();

	if (onOff == true)
	{
		colourR = 0;
		colourG = 0;
		colourB = 0;
	}

	colour = glm::vec3(colourR, colourG, colourB);
	direction = -lightPos;
	direction.w = 0.f;
	direction = glm::normalize(direction);
	
}

void DirLight::Draw()
{
	//draws a box where the light is
	Gizmos::addBox(lightPos.xyz, glm::vec3(0.2f, 0.2f, 0.2f), true, glm::vec4(colourR, colourG, colourB, 1.f));
}
