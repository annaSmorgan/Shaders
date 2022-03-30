#ifndef __TessellationShaderApplication_H_
#define __TessellationShaderApplication_H_

#include "Application.h"
#include <glm/glm.hpp>
#include <string>

//A simple vertex structure containing position, normal and texture coordinate data
class SimpleVertex
{
public:
	enum VertexAttributeFlags
	{
		POSITION = (1 << 0), //The Position of the Vertex
		NORMAL = (1 << 1), //The Normal for the Vertex
		TANGENT = (1<<2),
		TEXCOORD = (1 << 3), //The UV Coordinates for the mesh
	};

	enum Offsets
	{
		PositionOffset = 0,
		NormalOffset = PositionOffset + sizeof(glm::vec4),
		TangentOffset = NormalOffset + sizeof(glm::vec4),
		TexCoordOffset = TangentOffset + sizeof(glm::vec4),
	};

	SimpleVertex();
	~SimpleVertex();

	glm::vec4 position;
	glm::vec4 normal;
	glm::vec4 tangent;
	glm::vec2 texCoord;
};

//SimpleVertex inline constructor and destructor
inline SimpleVertex::SimpleVertex() : position(0, 0, 0, 1),
normal(0, 0, 0, 0), tangent(0,0,0,0), texCoord(0, 0) {}

inline SimpleVertex::~SimpleVertex() {}

// Derived application class that wraps up all globals neatly
class TessellationShaderApplication : public Application
{
public:

	TessellationShaderApplication();
	virtual ~TessellationShaderApplication();

protected:

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();

	//function to load in a texture
	bool LoadImageFromFile(std::string a_filePath, unsigned int& a_textureID);
		
	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	glm::vec4 m_lightPos;

	unsigned int	m_programID;
	unsigned int	m_vertexShader;
	unsigned int	m_geometryShader;
	unsigned int	m_tessControlShader;
	unsigned int	m_tessEvaluationShader;
	unsigned int	m_fragmentShader;

	unsigned int m_maxIndices;
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;

	//Texture Handles for OpenGL
	unsigned int m_DiffuseTex;
	unsigned int m_NormalTex;
	unsigned int m_SpecularTex;
	unsigned int m_OcclusionTex;
	unsigned int m_DisplacementTex;

	SimpleVertex* m_vertices;

	bool m_bRenderWireFrame;
	int m_innerTessEdge;
	int m_outerTessEdge;
	float m_displacementScale;
};

#endif // __TessellationShaderApplication_H_