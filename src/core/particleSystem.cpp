﻿// particleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystem.h"

using namespace std;

#define MAX_PARTICLES 10000
#define PARTICLE_LIFETIME 1000.0f

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

	numEmitters = 10;

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

	// Init the particle 0, the initial emitter
	for (unsigned int i = 0; i < numEmitters; i++) {
		Particles[i].Type = PARTICLE_TYPE_LAUNCHER;
		float sphere = 2*3.1415f* ( (float)i / ((float)numEmitters - 1));
		Particles[i].Pos = Pos + glm::vec3(0.1f*sin(sphere), 0, 0.1f*cos(sphere));
		Particles[i].Vel = glm::vec3(0.0f, 0.01f, 0.0f);
		Particles[i].LifetimeMillis = 0.0f;
	}

	// Init the particle 0, the initial emitter
	Particles[1].Type = PARTICLE_TYPE_LAUNCHER;
	Particles[1].Pos = glm::vec3(0.01f, 0, 2.8f);
	Particles[1].Vel = glm::vec3(0.0f, 0.01f, 0.0f);
	Particles[1].LifetimeMillis = 0.0f;

	// Gen the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Gen buffers
	glGenTransformFeedbacks(2, m_transformFeedback);
	glGenBuffers(2, m_particleBuffer);


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}

	if (!initShaderParticleSystem()) {
		return false;
	}

	// Send the variables to the Particle System shader
	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
	particleSystem_shader->setValue("gLauncherLifetime", 100.0f); //Time to emit all particles
	particleSystem_shader->setValue("gShellLifetime", 10000.0f);
	particleSystem_shader->setValue("gSecondaryShellLifetime", 2500.0f);

	if (!initRandomTexture(1000)) {
		return false;
	}

	bindRandomTexture(RANDOM_TEXTURE_UNIT);

	if (!initShaderBillboard()) {
		return false;
	}

	//Use the billboard shader and send variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[billboardShader];
	my_shader->use();
	my_shader->setValue("gColorMap", 0); // Set color map to 0
	my_shader->setValue("gBillboardSize", 0.01f);	// Set billboard size
	
	m_pTextureNum = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/part_redglow.jpg"); //TODO: Use any other texture, configure in the section
	m_pTexture = DEMO->textureManager.texture[m_pTextureNum];

	// Make sure the VAO is not changed from the outside
	glBindVertexArray(0);

	//return GLCheckError();
	return true; // TODO: check errors, etc etc...
}


void ParticleSystem::Render(int DeltaTimeMillis, const glm::mat4 &VP, const glm::vec3 &CameraPos)
{
	m_time += DeltaTimeMillis;

	glBindVertexArray(m_VAO);

	UpdateParticles(DeltaTimeMillis);

	RenderParticles(VP, CameraPos);

	glBindVertexArray(0);

	m_currVB = m_currTFB;
	m_currTFB = (m_currTFB + 1) & 0x1;
}

void ParticleSystem::resetParticleSystem(const glm::vec3 &Pos)
{
	/*Particle Particles[MAX_PARTICLES];

	ZERO_MEM(Particles);

	// Init the particle 0, the initial emitter
	Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
	Particles[0].Pos = Pos;
	Particles[0].Vel = glm::vec3(0.0f, 0.01f, 0.0f);
	Particles[0].LifetimeMillis = 0.0f;


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}
	*/
}


void ParticleSystem::UpdateParticles(int DeltaTimeMillis)
{

	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gTime", (float)this->m_time); // TODO: Esto se ha de ajustar... la variable m_time ha de ser un float sin milisegundos ni mierdas
	particleSystem_shader->setValue("gDeltaTimeMillis", (float)DeltaTimeMillis); // TODO: Esto se ha de ajustar... la variable DeltaTimeMillis ha de ser un float sin milisegundos ni mierdas

	bindRandomTexture(RANDOM_TEXTURE_UNIT);

	glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen

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
		glDrawArrays(GL_POINTS, 0, numEmitters);
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

	glDisable(GL_RASTERIZER_DISCARD);	// Start drawing on the screen
	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);
	glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position
		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
	glDisableVertexAttribArray(0);
}

bool ParticleSystem::initShaderBillboard()
{
	billboardShader = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.vs",
													DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.fs",
													DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.gs");
	if (billboardShader < 0)
		return false;
	return true;
}

bool ParticleSystem::initShaderParticleSystem()
{
	particleSystemShader = DEMO->shaderManager.addShader(	DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.vs",
															DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.fs",
															DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.gs",
															{ "Type1", "Position1", "Velocity1", "Age1" });
	if (particleSystemShader < 0)
		return false;

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
