#ifndef __FireParticles_H_
#define __FireParticles_H_

#include <glm/glm.hpp>


class FireParticles //particles made with geometry
{
public:

	FireParticles();
	virtual ~FireParticles();

	//public functions called in tank renderer
	bool onCreate(unsigned int m_windowWidth, unsigned int m_windowHeight, glm::vec4 laucherPos);
	void Update(float a_deltaTime);
	void Destroy();
	void GeoDraw(unsigned int g_randomTexture, unsigned int g_textureID, glm::mat4 g_cameraMatrix, glm::mat4 g_projectionMatrix);

private:

	struct SVertex 
	{
		glm::vec4 pos;
		glm::vec4 colour;
	};
	//program and shader variables
	unsigned int	g_programID;
	unsigned int	g_vertexShader;
	unsigned int	g_geometryShader;
	unsigned int	g_fragmentShader;

	//buffer variables
	unsigned int g_maxIndices;
	unsigned int g_vao;
	unsigned int g_vbo;
	unsigned int g_ibo;
	//transform feedback 
	unsigned int g_tfProgram;
	unsigned int g_tfVertexShader;
	unsigned int g_tfGeometryShader;

	unsigned int g_tf[2];
	unsigned int g_vb[2];
	unsigned int g_tf_curr;
	unsigned int g_tf_prev;
	unsigned int g_tfVAO[2];

	////use atomic int to track particles
	unsigned int g_atomicBuffer;
	unsigned int g_acReadBuffer;
	unsigned int g_particleCount;
	SVertex* g_vertices;

	int		g_currentState;

};

#endif // _FireParticles