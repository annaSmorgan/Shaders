#include "stdafx.h"
#include "FireParticles.h"

//particles made with geometry

FireParticles::FireParticles()
{
}

FireParticles::~FireParticles()
{
}

bool FireParticles::onCreate(unsigned int m_windowWidth, unsigned int m_windowHeight, glm::vec4 laucherPos)
{
	g_currentState = 0; //state 0 is draw a triangle state 1 is draw a box
	// initialise the Gizmos helper class
	Gizmos::create();
	//limit the maximum vertices we can have
	g_maxIndices = 4;
	
#pragma region TRANSFORM FEEDBACK
	g_tf_curr = 0;
	g_tf_prev = 1;
	//load in geo shaders
	g_tfVertexShader = Utility::loadShader("./resources/shaders/tf_vertex.glsl", GL_VERTEX_SHADER);
	g_tfGeometryShader = Utility::loadShader("./resources/shaders/tf_geom.glsl", GL_GEOMETRY_SHADER);
	const char* tfInputs[] = { "particleType", "particlePos", "particleVelocity", "particleAge" };
	const char* tfOutputs[] = { "Type", "Position", "Velocity", "Age" };
	g_tfProgram = Utility::createTransformFeedbackProgram(g_tfVertexShader, 0,
		0, g_tfGeometryShader, 4, tfInputs, 4, tfOutputs);

	struct Particle //struct for details about the particle emitter
	{
		float Type;
		glm::vec4 Pos;
		glm::vec4 Vel;
		float Age;
	};
	Particle launcher;
	launcher.Type = 0.f;
	launcher.Pos = laucherPos; //glm::vec4(-0.5f, -9.f, 5.f, 1.f);
	launcher.Vel = glm::vec4(0.f, 0.f, 0.f, 0.f);
	launcher.Age = 0.f;

	//tx feedback buffers
	glGenTransformFeedbacks(2, g_tf);
	glGenVertexArrays(2, g_tfVAO);
	glGenBuffers(2, g_vb);
	srand(time(nullptr));
	for (int i = 0; i < 2; ++i)
	{
		glBindVertexArray(g_tfVAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, g_vb[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * 500, nullptr, GL_DYNAMIC_DRAW);
		for (int j = 0; j < 10; ++j)
		{
			glm::vec4 rotVel = glm::vec4(glm::rotate(glm::vec3(0.f, 0.5f, 0.f), j * 1.f, glm::vec3(1.f, 0.f, 0.f)), 0.f);
			launcher.Vel = rotVel;
			glBufferSubData(GL_ARRAY_BUFFER, j * sizeof(Particle), sizeof(Particle), &launcher);
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)20);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, g_tf[i]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, g_vb[i]);

	}

	g_particleCount = 0;
	//atomic counter to track particles
	glGenBuffers(1, &g_atomicBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, g_atomicBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &g_particleCount, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	glGenBuffers(1, &g_acReadBuffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, g_acReadBuffer);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_READ);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

	glBindVertexArray(0);
#pragma endregion
	//Load the shaders for this program
	g_vertexShader = Utility::loadShader("./resources/shaders/vertexGeo.glsl", GL_VERTEX_SHADER);
	g_fragmentShader = Utility::loadShader("./resources/shaders/fragmentGeo.glsl", GL_FRAGMENT_SHADER);
	//here we have an additional shader to load and bind the geometry shader
	//This is the only change to our loading code for this example as the geometry shader
	//feeds into the fragment shader the outputs remain the same.
	g_geometryShader = Utility::loadShader("./resources/shaders/geometry.glsl", GL_GEOMETRY_SHADER);
	//Define the input and output varialbes in the shaders
	//Note: these names are taken from the glsl files -- added in inputs for UV coordinates
	const char* szInputs[] = { "particleType", "particlePos", "particleVelocity", "particleAge" };
	const char* szOutputs[] = { "FragColor" };
	//bind the shaders to create our shader program
	g_programID = Utility::createProgram(
		g_vertexShader,
		0,
		0,
		g_geometryShader,
		g_fragmentShader,
		4, szInputs, 1, szOutputs);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void FireParticles::Update(float a_deltaTime)
{

	if (ImGui::Begin("Particle Details"))
	{
		ImGui::Text("Particle Count: %u", g_particleCount);
	}
	ImGui::End();

	static bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
	Application_Log* log = Application_Log::Get();
	if (log != nullptr && show_demo_window)
	{
		log->showLog(&show_demo_window);
	}

}

void FireParticles::GeoDraw(unsigned int g_randomTexture, unsigned int g_textureID, glm::mat4 g_cameraMatrix, glm::mat4 g_projectionMatrix)
{
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(g_cameraMatrix);

	glUseProgram(g_tfProgram);
	//no frag shader so disable rasterisation
	glEnable(GL_RASTERIZER_DISCARD);
	unsigned int deltaT = glGetUniformLocation(g_tfProgram, "deltaTime");
	glUniform1f(deltaT, Utility::getDeltaTime());
	unsigned int totalT = glGetUniformLocation(g_tfProgram, "totalTime");
	glUniform1f(totalT, Utility::getTotalTime());
	unsigned int randTexUniformID = glGetUniformLocation(g_tfProgram, "randomTexture");
	glUniform1i(randTexUniformID, 0);
	//set active texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_randomTexture);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, g_atomicBuffer);
	glBindVertexArray(g_tfVAO[g_tf_curr]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, g_tf[g_tf_prev]);

	glBeginTransformFeedback(GL_POINTS);

	static bool firstDraw = true;
	if (firstDraw)
	{
		glDrawArrays(GL_POINTS, 0, 10);
		firstDraw = false;
	}
	else
	{
		glDrawTransformFeedback(GL_POINTS, g_tf[g_tf_curr]);
	}
	glEndTransformFeedback();
	glBindVertexArray(0);
	//turn rasterisation back on
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, g_atomicBuffer);

	//read counter through temporary buyffer and reset counter
	glBindBuffer(GL_COPY_WRITE_BUFFER, g_acReadBuffer);
	glCopyBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLuint));
	unsigned int* counterValue = (unsigned int*)glMapBufferRange(GL_COPY_WRITE_BUFFER, 0,
		sizeof(GLuint), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	g_particleCount = counterValue[0];
	counterValue[0] = 0;
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_ATOMIC_COUNTER_BUFFER, 0, 0, sizeof(GLuint));
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glUseProgram(0);

	//bing our shader program
	glUseProgram(g_programID);


	//get our shaders uniform location for our projectionViewMatrix and then use glUniformMatrix4fv to fill it with the correct data
	unsigned int projectionViewUniform = glGetUniformLocation(g_programID, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(g_projectionMatrix * viewMatrix));

	unsigned int cameraPosUniform = glGetUniformLocation(g_programID, "cameraPosition");
	glUniform4fv(cameraPosUniform, 1, glm::value_ptr(g_cameraMatrix[3]));

	//bind our textureLocation variable from the shaders and set it's value to 0 as the active texture is texture 0
	unsigned int texUniformID = glGetUniformLocation(g_programID, "textureLoc");
	glUniform1i(texUniformID, 0);

	//set our active texture, and bind our loaded texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textureID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//bind our vertex array object
	glBindVertexArray(g_tfVAO[g_tf_prev]);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);
	glBlendEquation(GL_MAX);

	//depending on the state call draw with glDrawElements to draw our buffer
	//glDrawElements uses the index array in our index buffer to draw the vertices in our vertex buffer
	glDrawTransformFeedback(GL_POINTS, g_tf[g_tf_prev]);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glBindVertexArray(0);
	glUseProgram(0);

	g_tf_curr = g_tf_prev;
	g_tf_prev = (g_tf_prev + 1) & 0x1;
}

void FireParticles::Destroy()
{
	Gizmos::destroy();
	//unload the buffers and shaders
	glDeleteBuffers(1, &g_vbo);
	glDeleteBuffers(1, &g_ibo);
	glDeleteVertexArrays(1, &g_vao);
	glDeleteProgram(g_programID);
	glDeleteShader(g_fragmentShader);
	glDeleteShader(g_geometryShader);
	glDeleteShader(g_vertexShader);
}

