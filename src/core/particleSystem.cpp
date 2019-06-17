// particleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystem.h"

using namespace std;

#define PARTICLE_TYPE_LAUNCHER 0
#define PARTICLE_TYPE_SHELL 1

#define RANDOM_TEXTURE_UNIT 3

struct Particle
{
	glm::vec3 Pos;	// Position: loc 0 (vec3)
	glm::vec3 Vel;	// Velocity: loc 1 (vec3)
	glm::vec3 Col;	// Color: loc 2 (vec3)
	float lifeTime;	// lifeTime: loc 3 (float)
	float Size;		// size: loc 4 (float)
	int Type;		// type: loc 5 (int)
};


ParticleSystem::ParticleSystem()
{
	m_currVB = 0;
	m_currTFB = 1;
	m_isFirst = true;
	m_time = 0;
	m_pTexture = NULL;

	numMaxParticles = 10000; // Should be at least greather than: numEmitters + numEmitters*gShellLifetime*(1/gLauncherLifetime)
	numEmitters = 5;

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
	this->initPosition = Pos;
	//Particle* Particles = (Particle*)malloc(sizeof(Particle) * numMaxParticles);	 // This is only need if we want to upload all the particles to the GPU, which is a waste of resources
	Particle* Particles = (Particle*)malloc(sizeof(Particle) * numEmitters);
	ZERO_MEM(Particles);

	// Init the particle 0, the initial emitter
	for (unsigned int i = 0; i < numEmitters; i++) {
		Particles[i].Type = PARTICLE_TYPE_LAUNCHER;
		float sphere = 2*3.1415f* ( (float)(i+1) / ((float)numEmitters));
		Particles[i].Pos = initPosition + glm::vec3(sin(sphere), 0, cos(sphere));
		Particles[i].Vel = glm::vec3(0.0f, 1.0f, 0.0f);
		Particles[i].Col = glm::vec3(1.0f, 1.0f, 1.0f);
		Particles[i].Size = 1.0;
		Particles[i].lifeTime = 0.0f;
	}

	// Gen the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Gen buffers
	glGenTransformFeedbacks(2, m_transformFeedback);
	glGenBuffers(2, m_particleBuffer);


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*numMaxParticles, Particles, GL_DYNAMIC_DRAW); // Upload the entire buffer (requires that "Particles" should have the size of all the particles [numMaxParticles])
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*numMaxParticles, NULL, GL_DYNAMIC_DRAW);	// Allocate mem, uploading an empty buffer for all the particles
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle)*numEmitters, Particles);			// Upload only the emitters to the Buffer
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}
	free(Particles);

	if (!initShaderParticleSystem()) {
		return false;
	}

	// Send the variables to the Particle System shader
	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
	particleSystem_shader->setValue("gLauncherLifetime", 1.0f); // Time between emissions
	particleSystem_shader->setValue("gShellLifetime", 20.0f);

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
	my_shader->setValue("gColorMap", 0);			// Set color map to 0
	my_shader->setValue("gBillboardSize", 1.0f);	// Set billboard size
	
	m_pTextureNum = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/part_redglow.jpg"); //TODO: Use any other texture, configure in the section
	m_pTexture = DEMO->textureManager.texture[m_pTextureNum];

	// Make sure the VAO is not changed from the outside
	glBindVertexArray(0);

	//return GLCheckError();
	return true; // TODO: check errors, etc etc...
}


void ParticleSystem::Render(float deltaTime, const glm::mat4 &VP, const glm::vec3 &CameraPos)
{
	m_time += deltaTime;

	glBindVertexArray(m_VAO);

	UpdateParticles(deltaTime);

	RenderParticles(VP, CameraPos);

	glBindVertexArray(0);

	m_currVB = m_currTFB;
	m_currTFB = (m_currTFB + 1) & 0x1;
}

// TODO
void ParticleSystem::resetParticleSystem(const glm::vec3 &Pos)
{
	/*Particle Particles[MAX_PARTICLES];

	ZERO_MEM(Particles);

	// Init the particle 0, the initial emitter
	Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
	Particles[0].Pos = Pos;
	Particles[0].Vel = glm::vec3(0.0f, 0.01f, 0.0f);
	Particles[0].lifeTime = 0.0f;


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}
	*/
}

float each_half = 0.0f;
void ParticleSystem::UpdateEmitters(float deltaTime)
{
	// Test: move the emitter
	each_half += deltaTime;
	if (each_half > 1.0f) {
		each_half = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
		// TODO: Investigate this flags:  | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
		Particle *data = (Particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle)*numEmitters, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);// | GL_MAP_UNSYNCHRONIZED_BIT);
		// Change data and move some random positions
		for (unsigned int i = 0; i < numEmitters; i++) {
			data[i].Type = PARTICLE_TYPE_LAUNCHER;
			float sphere = 2 * 3.1415f* ((float)(i + 1) / ((float)numEmitters));
			data[i].Pos = initPosition + glm::vec3(sin(sphere), -0.1*m_time, cos(sphere));
			data[i].Vel = glm::vec3(1.0f, 0.01f, 0.0f);
			data[i].lifeTime = 10.0f; // TODO: investigate why only emmits if this is greater than 0...
			data[i].Size = 1.0f;
			data[i].Col = glm::vec3(1, 1, 1);
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
}


void ParticleSystem::UpdateParticles(float deltaTime)
{
	//UpdateEmitters(deltaTime);

	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gTime", this->m_time);
	particleSystem_shader->setValue("gDeltaTime", deltaTime);

	bindRandomTexture(RANDOM_TEXTURE_UNIT);

	glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen

	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);	// Position (12 bytes)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12);	// Velocity (12 bytes)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24);	// Color (12 bytes)
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36);	// Lifetime (4 bytes)
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)40);	// Size (4 bytes)
	glVertexAttribPointer(5, 1, GL_INT, GL_FALSE, sizeof(Particle), (const GLvoid*)44);		// Type (4 bytes)

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
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);
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
		// TODO: En principio el "glVertexAttribPointer" solo es necesario en el setup del principio,
		// aqui solo hace falta hacer un "glEnable/DisableVertexAttribArray" de los canales que queremos enviar al pintar
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);	// Position
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position
		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
	glDisableVertexAttribArray(0);
}

bool ParticleSystem::initShaderBillboard()
{
	billboardShader = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.vert",
													DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.frag",
													DEMO->dataFolder + "/resources/shaders/particleSystem/billboard.geom");
	if (billboardShader < 0)
		return false;
	return true;
}

bool ParticleSystem::initShaderParticleSystem()
{
	particleSystemShader = DEMO->shaderManager.addShader(	DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.vert",
															DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.frag",
															DEMO->dataFolder + "/resources/shaders/particleSystem/ps_update.geom",
															{"Position1", "Velocity1", "Color1", "Age1", "Size1", "Type1"});
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
