// particleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystem.h"

using namespace std;

#define MAX_PARTICLES 10000
#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

#define RANDOM_TEXTURE_UNIT 3

struct Particle
{
	float Type;
	glm::vec3 Pos;
	glm::vec3 Vel;
	float LifetimeMillis;
};


ParticleSystem::ParticleSystem()
{
	m_currVB = 0;
	m_currTFB = 1;
	m_isFirst = true;
	m_time = 0;
	m_pTexture = NULL;

	ZERO_MEM(m_transformFeedback);
	ZERO_MEM(m_particleBuffer);
}


ParticleSystem::~ParticleSystem()
{
	SAFE_DELETE(m_pTexture);

	if (m_transformFeedback[0] != 0) {
		glDeleteTransformFeedbacks(2, m_transformFeedback);
	}

	if (m_particleBuffer[0] != 0) {
		glDeleteBuffers(2, m_particleBuffer);
	}
}


bool ParticleSystem::InitParticleSystem(const glm::vec3 &Pos)
{
	Particle Particles[MAX_PARTICLES];
	ZERO_MEM(Particles);

	Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
	Particles[0].Pos = Pos;
	Particles[0].Vel = glm::vec3(0.0f, 0.01f, 0.0f);
	Particles[0].LifetimeMillis = 0.0f;

	// Gen buffers
	glGenTransformFeedbacks(2, m_transformFeedback);
	glGenBuffers(2, m_particleBuffer);


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}

	if (!initParticleSystem()) {
		return false;
	}

	// Send the variables to the Particle System shader
	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
	particleSystem_shader->setValue("gLauncherLifetime", 100.0f);
	particleSystem_shader->setValue("gShellLifetime", 10000.0f);
	particleSystem_shader->setValue("gSecondaryShellLifetime", 2500.0f);

	if (!initRandomTexture(1000)) {
		return false;
	}

	bindRandomTexture(RANDOM_TEXTURE_UNIT);
	//m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

	if (!initBillboard()) {
		return false;
	}

	//Use the billboard shader and send variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[billboardShader];
	my_shader->use();
	my_shader->setValue("gColorMap", 0); // Set color map to 0
	my_shader->setValue("gBillboardSize", 0.01f);	// Set billboard size
	
	string s_dir = DEMO->demoDir;
	m_pTextureNum = DEMO->textureManager.addTexture(s_dir + "/resources/textures/part_redfireworks.jpg"); //TODO: Use any other texture, configure in the section
	m_pTexture = DEMO->textureManager.texture[m_pTextureNum];

	//return GLCheckError();
	return true; // TODO: check errors, etc etc...
}


void ParticleSystem::Render(int DeltaTimeMillis, const glm::mat4 &VP, const glm::vec3 &CameraPos)
{
	m_time += DeltaTimeMillis;

	UpdateParticles(DeltaTimeMillis);

	RenderParticles(VP, CameraPos);

	m_currVB = m_currTFB;
	m_currTFB = (m_currTFB + 1) & 0x1;
}


void ParticleSystem::UpdateParticles(int DeltaTimeMillis)
{
	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gTime", (float)this->m_time); // TODO: Esto se ha de ajustar... la variable m_time ha de ser un float sin milisegundos ni mierdas
	particleSystem_shader->setValue("gDeltaTimeMillis", (float)DeltaTimeMillis); // TODO: Esto se ha de ajustar... la variable DeltaTimeMillis ha de ser un float sin milisegundos ni mierdas
	//m_updateTechnique.Enable();
	//m_updateTechnique.SetTime(m_time);
	//m_updateTechnique.SetDeltaTimeMillis(DeltaTimeMillis);

	bindRandomTexture(RANDOM_TEXTURE_UNIT);
	//m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

	glEnable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);						// type
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);		// position
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);		// velocity
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);		// lifetime

	glBeginTransformFeedback(GL_POINTS);

	if (m_isFirst) {
		glDrawArrays(GL_POINTS, 0, 1);

		m_isFirst = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
	}

	glEndTransformFeedback();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}


void ParticleSystem::RenderParticles(const glm::mat4 &VP, const glm::vec3 &CameraPos)
{
	//Use the billboard shader and send variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[billboardShader];
	my_shader->use();
	my_shader->setValue("gCameraPos", CameraPos); // Set camera position
	my_shader->setValue("gVP", VP);	// Set billboard size

	// Activate texture
	m_pTexture->active(0);
	m_pTexture->bind();

	glDisable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position

	glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

	glDisableVertexAttribArray(0);
}

bool ParticleSystem::initBillboard()
{
	string demoDir = DEMO->demoDir;
	billboardShader = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/particleSystem/billboard.vs",
													demoDir + "/resources/shaders/particleSystem/billboard.fs",
													demoDir + "/resources/shaders/particleSystem/billboard.gs");
	if (billboardShader < 0)
		return false;
	return true;
}

bool ParticleSystem::initParticleSystem()
{
	string demoDir = DEMO->demoDir;
	particleSystemShader = DEMO->shaderManager.addShader(	demoDir + "/resources/shaders/particleSystem/ps_update.vs",
															demoDir + "/resources/shaders/particleSystem/ps_update.fs",
															demoDir + "/resources/shaders/particleSystem/ps_update.gs");
	if (particleSystemShader < 0)
		return false;

	// TODO: Implement this "glTransformFeedbackVaryings" into the shader class

	Shader *my_shader = DEMO->shaderManager.shader[particleSystemShader];
	const GLchar* Varyings[4];
	Varyings[0] = "Type1";
	Varyings[1] = "Position1";
	Varyings[2] = "Velocity1";
	Varyings[3] = "Age1";

	glTransformFeedbackVaryings(my_shader->ID, 4, Varyings, GL_INTERLEAVED_ATTRIBS);

	return true;
}

// TODO: Fix this guarrada
float RandomFloat()
{
	float Max = RAND_MAX;
	return ((float)rand() / Max);
}

bool ParticleSystem::initRandomTexture(unsigned int Size)
{
	glm::vec3* pRandomData = new glm::vec3[Size];
	for (unsigned int i = 0; i < Size; i++) {
		pRandomData[i].x = RandomFloat();
		pRandomData[i].y = RandomFloat();
		pRandomData[i].z = RandomFloat();
	}

	glGenTextures(1, &m_textureRandID);
	glBindTexture(GL_TEXTURE_1D, m_textureRandID);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Size, 0, GL_RGB, GL_FLOAT, pRandomData);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	delete[] pRandomData;

	return true;
}

void ParticleSystem::bindRandomTexture(unsigned int TextureUnit)
{
	glActiveTexture(GL_TEXTURE0 + TextureUnit);
	glBindTexture(GL_TEXTURE_1D, m_textureRandID);
}
