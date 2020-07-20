// particleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystem.h"

#define RANDOM_TEXTURE_UNIT 0

#define LOC_POSITION 0
#define LOC_VELOCITY 1
#define LOC_COLOR 2
#define LOC_LIFETIME 3
#define LOC_TYPE 4

#define BINDING_UPDATE	0
#define BINDING_BILLBOARD	1

ParticleSystem::ParticleSystem(std::string shaderPath, unsigned int	numMaxParticles, unsigned int numEmitters, float emissionTime, float particleLifeTime)
{
	force = glm::vec3(0, 0, 0);

	m_currVB = 0;
	m_currTFB = 1;
	m_isFirst = true;
	m_time = 0;

	this->shaderPath = shaderPath;
	this->pathBillboard = shaderPath + "/billboard.glsl";
	this->pathUpdate = shaderPath + "/update.glsl";

	this->numMaxParticles = numMaxParticles; // Should be at least greather than: numEmitters + numEmitters*gShellLifetime*(1/gLauncherLifetime)
	this->numEmitters = numEmitters;
	this->emissionTime = emissionTime;
	this->particleLifeTime = particleLifeTime;

	ZERO_MEM(m_transformFeedback);
	ZERO_MEM(m_particleBuffer);
}


ParticleSystem::~ParticleSystem()
{
	if (m_transformFeedback[0] != 0) {
		glDeleteTransformFeedbacks(2, m_transformFeedback);
	}

	if (m_particleBuffer[0] != 0) {
		glDeleteBuffers(2, m_particleBuffer);
	}
}


bool ParticleSystem::InitParticleSystem(Section* sec, const std::vector<Particle> emitter, std::vector<std::string> billboardShaderVars)
{
	if (numEmitters == 0)
		return false;

	Particle* Particles = new Particle[numEmitters];

	// Init the particle emitters
	for (unsigned int i = 0; i < numEmitters; i++) {
		Particles[i].Type = PARTICLE_TYPE_EMITTER;
		Particles[i].Pos = emitter[i].Pos;
		Particles[i].Vel = emitter[i].Vel;
		Particles[i].Col = emitter[i].Col;
		Particles[i].lifeTime = emitter[i].lifeTime;
	}

	// Gen the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Gen buffers
	glGenTransformFeedbacks(2, m_transformFeedback);	// Transform Feedback object
	glGenBuffers(2, m_particleBuffer);					// Transform Feedback buffer


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*numMaxParticles, NULL, GL_DYNAMIC_DRAW);	// Allocate mem, uploading an empty buffer for all the particles
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle)*numEmitters, Particles);			// Upload only the emitters to the Buffer
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
	}

	// Setup Vertex Attribute formats
	// Definitions for Update shader Binding
	glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[0], 0, sizeof(Particle));

	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Pos));	// Position (12 bytes)
	glVertexAttribBinding(LOC_POSITION, BINDING_UPDATE);

	glEnableVertexAttribArray(LOC_VELOCITY);
	glVertexAttribFormat(LOC_VELOCITY, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Vel));	// Velocity (12 bytes)
	glVertexAttribBinding(LOC_VELOCITY, BINDING_UPDATE);

	glEnableVertexAttribArray(LOC_COLOR);
	glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Col));	// Color (12 bytes)
	glVertexAttribBinding(LOC_COLOR, BINDING_UPDATE);

	glEnableVertexAttribArray(LOC_LIFETIME);
	glVertexAttribFormat(LOC_LIFETIME, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, lifeTime));	// Lifetime (4 bytes)
	glVertexAttribBinding(LOC_LIFETIME, BINDING_UPDATE);

	glEnableVertexAttribArray(LOC_TYPE);
	glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));	// Type (4 bytes)
	glVertexAttribBinding(LOC_TYPE, BINDING_UPDATE);

	// Definitions for Billboard shader Binding
	glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[0], 0, sizeof(Particle));

	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Pos));	// Position (12 bytes)
	glVertexAttribBinding(LOC_POSITION, BINDING_BILLBOARD);

	glEnableVertexAttribArray(LOC_COLOR);
	glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Col));	// Color (12 bytes)
	glVertexAttribBinding(LOC_COLOR, BINDING_BILLBOARD);

	glEnableVertexAttribArray(LOC_TYPE);
	glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));	// Type (4 bytes)
	glVertexAttribBinding(LOC_TYPE, BINDING_UPDATE);

	SAFE_DELETE_ARRAY(Particles);
	// Make sure the VAO is not changed from the outside
	glBindVertexArray(0);

	// UPDATE shader
	if (!initShaderParticleSystem()) {
		return false;
	}

	// Send the variables to the Particle System shader
	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT);
	particleSystem_shader->setValue("fEmissionTime", this->emissionTime); // Time between emissions
	particleSystem_shader->setValue("fParticleLifetime", this->particleLifeTime);

	if (!initRandomTexture(1000)) {
		return false;
	}

	bindRandomTexture(RANDOM_TEXTURE_UNIT);

	// BILLBOARD shader
	if (!initShaderBillboard()) {
		return false;
	}

	//Use the billboard shader and send variables
	Shader *billboard_shader;
	billboard_shader = DEMO->shaderManager.shader[billboardShader];
	billboard_shader->use();
	this->varsBillboard = new ShaderVars(sec, billboard_shader);
	// Read the shader variables
	for (int i = 0; i < billboardShaderVars.size(); i++) {
		this->varsBillboard->ReadString(billboardShaderVars[i].c_str());
	}
	// Set billboard shader variables values (texture, particle size, etc...)
	this->varsBillboard->setValues();
	
	//return GLCheckError();
	return true; // TODO: check errors, etc etc...
}


void ParticleSystem::Render(float deltaTime, const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos)
{
	m_time += deltaTime;

	glBindVertexArray(m_VAO);
		UpdateParticles(deltaTime);
		RenderParticles(VP, model, CameraPos);
	glBindVertexArray(0);

	m_currVB = m_currTFB;
	m_currTFB = (m_currTFB + 1) & 0x1;
}

static float m_time = 0.0f;
void ParticleSystem::UpdateEmitters(float deltaTime)
{
	m_time += deltaTime;
	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
	//numMaxParticles
	unsigned int nParts = numMaxParticles;// numEmitters;
	m_emitterData = (Particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle) * nParts, GL_MAP_WRITE_BIT);
	
	// Change data and move some random positions
	for (unsigned int i = 0; i < nParts; i++) {
		m_emitterData[i].Type = PARTICLE_TYPE_EMITTER;
		float sphere = 4* 3.1415f * ((float)(i) / ((float)nParts));
		m_emitterData[i].Pos = glm::vec3(sin(sphere), 3.0 * sin(m_time / 2.0), cos(sphere));
		m_emitterData[i].Vel = glm::vec3(0.0f, 10.0f, 0.0f);
		m_emitterData[i].Col = glm::vec3(1, 1, 1);
		m_emitterData[i].lifeTime = 1.0f; // TODO: investigate why only emmits if this is greater than 0...
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}


void ParticleSystem::UpdateParticles(float deltaTime)
{
	//UpdateEmitters(deltaTime);

	Shader *particleSystem_shader = DEMO->shaderManager.shader[particleSystemShader];
	particleSystem_shader->use();
	particleSystem_shader->setValue("gTime", this->m_time);
	particleSystem_shader->setValue("gDeltaTime", deltaTime);
	particleSystem_shader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
	particleSystem_shader->setValue("fEmissionTime", this->emissionTime);
	particleSystem_shader->setValue("fParticleLifetime", this->particleLifeTime);
	particleSystem_shader->setValue("force", this->force);


	bindRandomTexture(RANDOM_TEXTURE_UNIT);

	glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen

	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

	// Use Binding for particles (all attributes)
	glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[m_currVB], 0, sizeof(Particle));
	
	glBeginTransformFeedback(GL_POINTS);

	if (m_isFirst) {
		glDrawArrays(GL_POINTS, 0, numEmitters);
		m_isFirst = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
	}

	glEndTransformFeedback();

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}

void ParticleSystem::RenderParticles(const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos)
{
	//Use the billboard shader and send variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[billboardShader];
	my_shader->use();
	my_shader->setValue("gCameraPos", CameraPos);				// Set camera position
	my_shader->setValue("gVP", VP);								// Set ViewProjection Matrix
	my_shader->setValue("model", model);						// Set Model Matrix
	this->varsBillboard->setValues();

	glDisable(GL_RASTERIZER_DISCARD);	// Start drawing on the screen
	glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);
	
	// Use binding for billboard (only Position and color attributes)
	glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[m_currTFB], 0, sizeof(Particle));

	glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
}

bool ParticleSystem::initShaderBillboard()
{
	billboardShader = DEMO->shaderManager.addShader(this->pathBillboard);
	if (billboardShader < 0)
		return false;
	return true;
}

bool ParticleSystem::initShaderParticleSystem()
{
	particleSystemShader = DEMO->shaderManager.addShader( this->pathUpdate,
															{ "Position1", "Velocity1", "Color1", "Age1", "Type1" });

	if (particleSystemShader < 0)
		return false;
	return true;
}

// TODO: Investigate if we should move this "Random texture 1D generator" to the TextureManager class
static float RandomFloat()
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

void ParticleSystem::bindRandomTexture(GLuint TexUnit)
{
	glBindTextureUnit(TexUnit, m_textureRandID);
}
