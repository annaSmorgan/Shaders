#include "stdafx.h"
#include "..\include\Model.h"

//loads and renders both models

Model::Model()
{
}

Model::~Model()
{
}

void Model::Create()
{
	//creates instances of fbx class
	m_fbxModel = new FBXFile();
	m_fbxModel2 = new FBXFile();
	m_fbxModel3 = new FBXFile(); //third model was attempted but effect frame rate too much
	//loads the models from file
	m_fbxModel->load("./resources/models/island/andrewsisland_Unity_1.obj", FBXFile::UNITS_DECIMETER);
	m_fbxModel2->load("./resources/models/cat/model.obj", FBXFile::UNITS_FOOT);
	//m_fbxModel3->load("./resources/models/dirLight/parking_pole_ornamental_ma2.obj", FBXFile::UNITS_CENTIMETER);

	//Generate our OpenGL Vertex and Index Buffers for rendering our FBX Model Data
	// OPENGL: genorate the VBO, IBO and VAO
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);
	glGenVertexArrays(1, &m_vao);

	// OPENGL: Bind  VAO, and then bind the VBO and IBO to the VAO
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	//There is no need to populate the vbo & ibo buffers with any data at this stage
	//this can be done when rendering each mesh component of the FBX model

	// enable the attribute locations that will be used on our shaders
	glEnableVertexAttribArray(0); //Pos
	glEnableVertexAttribArray(1); //Col
	glEnableVertexAttribArray(2); //Norm
	glEnableVertexAttribArray(3); //Tex1


	// tell our shaders where the information within our buffers lie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), ((char*)0) + FBXVertex::PositionOffset);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::ColourOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::TexCoord1Offset);

	// finally, where done describing our mesh to the shader
	// we can describe the next mesh
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_modelMatrix = glm::mat4();
	m_modelMatrix2 = glm::mat4();
	m_modelMatrix3 = glm::mat4();
}


void Model::Update(float a_deltaTime)
{
	//Update our FBX model
	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode* mesh = m_fbxModel->getMeshByIndex(i);
		// if you move an object around within your scene
		// children nodes are not updated until this function is called.
		mesh->updateGlobalTransform();
	}

	for (unsigned int i = 0; i < m_fbxModel2->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode* mesh = m_fbxModel2->getMeshByIndex(i);
		// if you move an object around within your scene
		// children nodes are not updated until this function is called.
		mesh->updateGlobalTransform();
	}

	//for (unsigned int i = 0; i < m_fbxModel3->getMeshCount(); ++i)
	//{
	//	// get the current mesh
	//	FBXMeshNode* mesh = m_fbxModel3->getMeshByIndex(i);
	//	// if you move an object around within your scene
	//	// children nodes are not updated until this function is called.
	//	mesh->updateGlobalTransform();
	//}
}

void Model::Destroy()
{
	m_fbxModel->unload();
	m_fbxModel2->unload();
	m_fbxModel3->unload();

	delete m_fbxModel;
	delete m_fbxModel2;
	delete m_fbxModel3;

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
}

void Model::Draw(unsigned int a_programID, float modelPosX, float modelPosY, float modelPosZ, float modelScale)
{
	//bind our vertex array object
	glBindVertexArray(m_vao);

	//Draw our FBX Model
	// for each mesh in the model...
	for (unsigned int i = 0; i < m_fbxModel->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode* pMesh = m_fbxModel->getMeshByIndex(i);

		// send the Model
		glm::mat4 m4Model = pMesh->m_globalTransform;// *m_modelMatrix;
		unsigned int modelUniform = glGetUniformLocation(a_programID, "Model");
		glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(m4Model));
		//send the normal matrix
		//this is only beneficial if a model has a non-uniform scale or non-orthoganal model matrix
		glm::mat4 m4Normal = glm::transpose(glm::inverse(pMesh->m_globalTransform));// *m_modelMatrix;
		unsigned int normalMatrixUniform = glGetUniformLocation(a_programID, "NormalMatrix");
		glUniformMatrix4fv(normalMatrixUniform, 1, false, glm::value_ptr(m4Normal));

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
		unsigned int texUniformID = glGetUniformLocation(a_programID, "DiffuseTexture");
		glUniform1i(texUniformID, 0);
		//set our active texture, and bind our loaded texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::DiffuseTexture]);

		// Send the vertex data to the VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	for (unsigned int i = 0; i < m_fbxModel2->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode* pMesh = m_fbxModel2->getMeshByIndex(i);

		// send the Model
		glm::mat4 m4Model = pMesh->m_globalTransform;
		glm::mat4 catPos = glm::translate(m4Model, glm::vec3(modelPosX, modelPosY, modelPosZ));
		glm::mat4 catFinal = glm::scale(catPos, glm::vec3(modelScale));
		unsigned int modelUniform = glGetUniformLocation(a_programID, "Model");
		glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(catFinal));
		//send the normal matrix
		//this is only beneficial if a model has a non-uniform scale or non-orthoganal model matrix
		glm::mat4 m4Normal = glm::transpose(glm::inverse(m_modelMatrix2));// *m_modelMatrix;
		unsigned int normalMatrixUniform = glGetUniformLocation(a_programID, "NormalMatrix");
		glUniformMatrix4fv(normalMatrixUniform, 1, false, glm::value_ptr(m4Normal));

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
		unsigned int texUniformID = glGetUniformLocation(a_programID, "DiffuseTexture");
		glUniform1i(texUniformID, 0);
		//set our active texture, and bind our loaded texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::DiffuseTexture]);

		// Send the vertex data to the VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	//for (unsigned int i = 0; i < m_fbxModel3->getMeshCount(); ++i)
	//{
	//	// get the current mesh
	//	FBXMeshNode* pMesh = m_fbxModel3->getMeshByIndex(i);

	//	// send the Model
	//	glm::mat4 m4Model = pMesh->m_globalTransform;
	//	glm::mat4 lamp = glm::translate(m4Model, glm::vec3(0, 0, 0));
	//	unsigned int modelUniform = glGetUniformLocation(a_programID, "Model");
	//	glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(lamp));
	//	//send the normal matrix
	//	//this is only beneficial if a model has a non-uniform scale or non-orthoganal model matrix
	//	glm::mat4 m4Normal = glm::transpose(glm::inverse(m_modelMatrix2));// *m_modelMatrix;
	//	unsigned int normalMatrixUniform = glGetUniformLocation(a_programID, "NormalMatrix");
	//	glUniformMatrix4fv(normalMatrixUniform, 1, false, glm::value_ptr(m4Normal));

	//	// Bind the texture to one of the ActiveTextures
	//	// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
	//	//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
	//	unsigned int texUniformID = glGetUniformLocation(a_programID, "DiffuseTexture");
	//	glUniform1i(texUniformID, 0);
	//	//set our active texture, and bind our loaded texture
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, pMesh->m_material->textureIDs[FBXMaterial::DiffuseTexture]);

	//	// Send the vertex data to the VBO
	//	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	//	glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

	//	// send the index data to the IBO
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

	//	glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	//}
}
