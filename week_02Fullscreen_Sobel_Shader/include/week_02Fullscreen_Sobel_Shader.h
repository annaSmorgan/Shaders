#ifndef __week_02Fullscreen_Sobel_Shader_H_
#define __week_02Fullscreen_Sobel_Shader_H_

#include "Application.h"
#include <glm/glm.hpp>
#include <string>

// Derived application class that wraps up all globals neatly
class week_02Fullscreen_Sobel_Shader : public Application
{
public:

	week_02Fullscreen_Sobel_Shader();
	virtual ~week_02Fullscreen_Sobel_Shader();

protected:

	bool LoadImageFromFile(std::string a_filepath, unsigned int& a_textureID, int& a_width, int& a_height);
	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();


	unsigned int m_programID;
	unsigned int m_vertexShader;
	unsigned int m_fragmentShader;

	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
		glm::vec2 uvs;
	};
};

#endif // __week_02Fullscreen_Sobel_Shader_H_