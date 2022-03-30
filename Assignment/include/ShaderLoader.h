#ifndef __ShaderLoader_H_
#define __ShaderLoader_H_

#include <glm/glm.hpp>

class ShaderLoader 
{
public:

	ShaderLoader();
	~ShaderLoader();

	//public functions
	void LoadShaders();
	void Destroy();

	//public variables accessable by other needed classes
	unsigned int	m_programID;

private:

	//private functions
	void DeleteShaders();

	//private variables
	unsigned int vertexShader;
	unsigned int fragmentShader;

};

#endif // __ShaderLoader_H_