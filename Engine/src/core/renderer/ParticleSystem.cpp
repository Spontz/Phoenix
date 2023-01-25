// ParticleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleSystem.h"
#include <math.h>

namespace Phoenix {

	constexpr int RANDOM_TEXTURE_UNIT = 0;

	// TODO: QUITAR ESTO Y USAR LOS VB QUE TENEMOS EN EL ENGINE!!
	constexpr int LOC_TYPE = 0;
	constexpr int LOC_POSITION = 1;
	constexpr int LOC_VELOCITY = 2;
	constexpr int LOC_COLOR = 3;
	constexpr int LOC_LIFETIME = 4;
	
	constexpr int BINDING_UPDATE = 0;
	constexpr int BINDING_BILLBOARD = 1;

	ParticleSystem::ParticleSystem(std::string particleSystemShader, std::string billboardShader)
	{
		m_varsParticleSystem = nullptr; 
		m_varsBillboard = nullptr;
		
		force = glm::vec3(0, 0, 0);
		color = glm::vec3(1, 1, 1);
		randomness = 0;

		m_currVB = 0;
		m_currTFB = 1;
		m_isFirst = true;
		m_time = 0;

		m_queryPrimitives = 0;
		m_numGenParticles = 0;

		m_memUsed = 0;

		m_pathBillboard = billboardShader;
		m_pathUpdate = particleSystemShader;

		m_numEmitters = 0;
		m_numMaxParticles = 0;
		m_numParticlesPerEmitter = 0;
		
		m_emissionTime = 0;
		m_particleLifeTime = 0;
		
		m_transformFeedback = {};
		m_particleBuffer = {};
	}


	ParticleSystem::~ParticleSystem()
	{
		if (m_transformFeedback[0] != 0)
			glDeleteTransformFeedbacks(2, m_transformFeedback.data());

		if (m_particleBuffer[0] != 0)
			glDeleteBuffers(2, m_particleBuffer.data());

		if (m_queryPrimitives != 0)
			glDeleteQueries(1, &m_queryPrimitives);

		if (m_textureRandID != 0)
			glDeleteTextures(1, &m_textureRandID);

		if (m_varsBillboard)
			delete m_varsBillboard;

		if (m_varsParticleSystem)
			delete m_varsParticleSystem;
	}


	bool ParticleSystem::Init(Section* sec, const std::vector<Particle> emitters, float emissionTime,float particleLifeTime, std::vector<std::string> shaderVars)
	{
		if (emitters.size() == 0)
			return false;

		if (emissionTime <= 0)
			return false;

		m_numEmitters = static_cast<unsigned int>(emitters.size());
		m_emissionTime = emissionTime; 
		m_particleLifeTime = particleLifeTime;
		
		m_numParticlesPerEmitter = static_cast<unsigned int>(ceil(m_particleLifeTime * (1.0f / m_emissionTime)));
		m_numMaxParticles = m_numEmitters + m_numEmitters * m_numParticlesPerEmitter;

		// Gen the Query
		glGenQueries(1, &m_queryPrimitives);

		// Gen the VAO
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		// Gen buffers
		glGenTransformFeedbacks(2, m_transformFeedback.data());	// Transform Feedback object
		glGenBuffers(2, m_particleBuffer.data());				// Transform Feedback buffer

		// Info for Opengl4.5 tranform feedback buffer creation: https://cpp.hotexamples.com/examples/-/-/glNamedBufferStorage/cpp-glnamedbufferstorage-function-examples.html

		for (unsigned int i = 0; i < 2; i++) {
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
			glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * m_numMaxParticles, NULL, GL_DYNAMIC_DRAW);	// Allocate mem, uploading an empty buffer for all the particles
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * m_numEmitters, emitters.data());		// Upload only the emitters to the Buffer
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
		}
	
		m_memUsed = (float)(2 * sizeof(Particle) * m_numMaxParticles) / 1024.0f / 1024.0f;

		// Setup Vertex Attribute formats
		// Definitions for Update shader Binding
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));	// Type (4 bytes)
		glVertexAttribBinding(LOC_TYPE, BINDING_UPDATE);

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

		// Definitions for Billboard shader Binding
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));	// Type (4 bytes)
		glVertexAttribBinding(LOC_TYPE, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Pos));	// Position (12 bytes)
		glVertexAttribBinding(LOC_POSITION, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Col));	// Color (12 bytes)
		glVertexAttribBinding(LOC_COLOR, BINDING_BILLBOARD);

		// Make sure the VAO is not changed from the outside
		glBindVertexArray(0);

		// UPDATE shader
		if (!initShaderParticleSystem()) {
			return false;
		}

		// Send the variables to the Particle System shader
		m_particleSystemShader->use();
		m_particleSystemShader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT);
		m_particleSystemShader->setValue("fEmissionTime", m_emissionTime); // Time between emissions
		m_particleSystemShader->setValue("fParticleLifetime", m_particleLifeTime);

		if (!initRandomTexture(m_numParticlesPerEmitter)) {
			return false;
		}

		bindRandomTexture(RANDOM_TEXTURE_UNIT);

		// BILLBOARD shader
		if (!initShaderBillboard()) {
			return false;
		}


		//Use the particleSystem shader and send variables
		m_particleSystemShader->use();
		m_varsParticleSystem = new ShaderVars(sec, m_particleSystemShader);
		// Read the shader variables
		for (int i = 0; i < shaderVars.size(); i++) {
			m_varsParticleSystem->ReadString(shaderVars[i]);
		}
		// Set billboard shader variables values (texture, particle size, etc...)
		m_varsParticleSystem->setValues();

		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_varsBillboard = new ShaderVars(sec, m_billboardShader);
		// Read the shader variables
		for (int i = 0; i < shaderVars.size(); i++) {
			m_varsBillboard->ReadString(shaderVars[i]);
		}
		// Set billboard shader variables values (texture, particle size, etc...)
		m_varsBillboard->setValues();
		
		return !DEMO_checkGLError();
	}


	void ParticleSystem::Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		m_time += deltaTime;

		glBindVertexArray(m_VAO);
		UpdateParticles(deltaTime, model);
		RenderParticles(model, view, projection);
		glBindVertexArray(0);

		m_currVB = m_currTFB;
		m_currTFB = (m_currTFB + 1) & 0x1;
	}

	void ParticleSystem::UpdateEmitters(float deltaTime)
	{
		// NOTE: This method is never used, only useful for debugging and modifying the emitters by hardcode
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
		//m_numMaxParticles
		unsigned int nParts = m_numMaxParticles;// m_numEmitters;
		m_emitterData = (Particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle) * nParts, GL_MAP_WRITE_BIT);

		// Change data and move some random positions
		for (unsigned int i = 0; i < nParts; i++) {
			m_emitterData[i].Type = ParticleType::Emitter;
			float sphere = 4 * 3.1415f * ((float)(i) / ((float)nParts));
			m_emitterData[i].Pos = glm::vec3(glm::sin(sphere), 3.0 * glm::sin(m_time / 2.0), glm::cos(sphere));
			m_emitterData[i].Vel = glm::vec3(0.0f, 10.0f, 0.0f);
			m_emitterData[i].Col = glm::vec3(1, 1, 1);
			m_emitterData[i].lifeTime = 0.0f;
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	void ParticleSystem::UpdateEmittersPosition(const std::vector<Particle> emitters)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
		uint32_t nParts = m_numMaxParticles;

		m_emitterData = (Particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle) * nParts, GL_MAP_WRITE_BIT);

		// Update Emitter positions
		for (uint32_t i = 0; i < nParts; i++) {
			if (m_emitterData[i].Type == ParticleType::Emitter) {
				m_emitterData[i].Pos = emitters[0].Pos;
			}
			Logger::info(LogLevel::low, "Particle {}, Type {}, Pos {},{},{}, Age {}", i, (int32_t)(m_emitterData[i].Type), m_emitterData[i].Pos.x, m_emitterData[i].Pos.y, m_emitterData[i].Pos.z, m_emitterData[i].lifeTime);
				
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	void ParticleSystem::UpdateParticles(float deltaTime, const glm::mat4& model)
	{
		//UpdateEmitters(deltaTime); // For debugging only: Overwrittes the emitter position
		m_particleSystemShader->use();
		m_particleSystemShader->setValue("u_m4Model", model);
		m_particleSystemShader->setValue("u_fTime", m_time);
		m_particleSystemShader->setValue("u_fDeltaTime", deltaTime);
		m_particleSystemShader->setValue("u_iRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
		m_particleSystemShader->setValue("u_fEmissionTime", m_emissionTime);
		m_particleSystemShader->setValue("u_fParticleLifetime", m_particleLifeTime);
		m_particleSystemShader->setValue("u_fRamndomness", randomness);
		m_particleSystemShader->setValue("u_v3Force", force);
		m_particleSystemShader->setValue("u_v3Color", color);
		m_varsParticleSystem->setValues();

		bindRandomTexture(RANDOM_TEXTURE_UNIT);

		glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

		// Use Binding for particles (all attributes)
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[m_currVB], 0, sizeof(Particle));

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_queryPrimitives);

		glBeginTransformFeedback(GL_POINTS);

		if (m_isFirst) {
			glDrawArrays(GL_POINTS, 0, m_numEmitters);
			m_isFirst = false;
		}
		else {
			glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
		}

		glEndTransformFeedback();

		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glGetQueryObjectuiv(m_queryPrimitives, GL_QUERY_RESULT, &m_numGenParticles);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	}

	void ParticleSystem::RenderParticles(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_billboardShader->setValue("model", model);						// Set Model Matrix
		m_billboardShader->setValue("view", view);							// Set View Matrix
		m_billboardShader->setValue("projection", projection);				// Set Projection Matrix
		m_varsBillboard->setValues();

		glDisable(GL_RASTERIZER_DISCARD);	// Start drawing on the screen
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

		// Use binding for billboard (only Position and color attributes)
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[m_currTFB], 0, sizeof(Particle));

		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

		//debugLogBufferData(); // For debugging only: Outputs to the log the content of the buffers
	}

	bool ParticleSystem::initShaderBillboard()
	{
		m_billboardShader = DEMO->m_shaderManager.addShader(m_pathBillboard);
		if (m_billboardShader)
			return true;
		return false;
	}

	bool ParticleSystem::initShaderParticleSystem()
	{
		m_particleSystemShader = DEMO->m_shaderManager.addShader(m_pathUpdate,
			{ "o_Type", "o_Position", "o_Velocity", "o_Color", "o_Age"});

		if (m_particleSystemShader)
			return true;
		return false;
	}

	// TODO: Investigate if we should move this "Random texture 1D generator" to the TextureManager class
	static glm::vec3 RandomVec3() // Return a float between -0.5 and 0.5
	{
		float Max = RAND_MAX;
		glm::vec3 randNum((float)rand(), (float)rand(), (float)rand());
		randNum /= Max;	// Values between 0 and 1
		randNum -= 0.5f;	// Values between 0.5 and -0.5

		return randNum;
	}


	bool ParticleSystem::initRandomTexture(unsigned int Size)
	{
		glm::vec3* pRandomData = new glm::vec3[Size];
		for (unsigned int i = 0; i < Size; i++) {
			pRandomData[i] = RandomVec3();
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

	void ParticleSystem::debugLogBufferData()
	{
		/// DEBUG ONLY
		/// Output Buffer values
		LogLevel l = LogLevel::low;
		Logger::info(l, "Buffers Current VB:{}, current TFB:{}", m_currVB, m_currTFB);
		for (unsigned int i = 0; i < 2; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
			Particle* p = (Particle*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			Logger::info(l, "SETUP Buffer {}", i);
			for (unsigned int i = 0; i < m_numMaxParticles; i++)
			{
				Logger::info(l, "Part {}: Type: {}, L:{:.2f}, P:({:.2f},{:.2f},{:.2f}), C:({:.2f},{:.2f},{:.2f})", i, (int32_t)p[i].Type, p[i].lifeTime, p[i].Pos.x, p[i].Pos.y, p[i].Pos.z, p[i].Col.x, p[i].Col.y, p[i].Col.z);
			}
			Logger::info(l, "");
			glUnmapBuffer(GL_ARRAY_BUFFER);

		}
	}
}