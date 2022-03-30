#ifndef __Model_H_
#define __Model_H_

#include <glm/glm.hpp>
#include "FBXFile.h"


class Model //loads and renders both models
{
public:

	Model();
	virtual ~Model();

	//pulbic functions used within tank render
	void Create();
	void Update(float a_deltaTime);
	void Destroy();
	void Draw(unsigned int a_programID, float modelPosX, float modelPosY, float modelPosZ, float modelScale);


private:

	//FBX Model File
	FBXFile* m_fbxModel;
	FBXFile* m_fbxModel2;
	FBXFile* m_fbxModel3;

	//vertex buffer / array
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	//model matricies
	glm::mat4	m_modelMatrix;
	glm::mat4	m_modelMatrix2;
	glm::mat4	m_modelMatrix3;

};

#endif // _Model_H_