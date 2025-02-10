// ParticleSystemEx.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleSystemEx.h"

namespace Phoenix {

	constexpr int RANDOM_TEXTURE_UNIT = 0;

	// TODO: QUITAR ESTO Y USAR LOS VB QUE TENEMOS EN EL ENGINE!!
	constexpr int LOC_TYPE = 0;
	constexpr int LOC_ID = 1;
	constexpr int LOC_INITPOSITION = 2;
	constexpr int LOC_POSITION = 3;
	constexpr int LOC_RANDOMNESS = 4;
	constexpr int LOC_ROTATION = 5;
	constexpr int LOC_INITCOLOR = 6;
	constexpr int LOC_COLOR = 7;
	constexpr int LOC_AGE = 8;
	constexpr int LOC_LIFE = 9;

	constexpr int BINDING_UPDATE = 0;
	constexpr int BINDING_BILLBOARD = 1;

	ParticleSystemEx::ParticleSystemEx(std::string particleSystemShader, std::string billboardShader, float particleLifeTime)
	{
		m_varsBillboard = nullptr;
		m_varsParticleSystem = nullptr;

		force = glm::vec3(0, 0, 0);
		color = glm::vec3(1, 1, 1);
		randomness = 0;

		m_currVB = 0;
		m_currTFB = 1;
		m_isFirst = true;

		m_queryPrimitives = 0;
		m_numGenParticles = 0;

		m_numParticles = 0;
		m_numParticlesPerEmitter = 0;

		m_memUsed = 0;

		m_pathBillboard = billboardShader;
		m_pathUpdate = particleSystemShader;

		m_particleLifeTime = particleLifeTime;
		
		m_transformFeedback = {};
		m_particleBuffer = {};
	}


	ParticleSystemEx::~ParticleSystemEx()
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


	bool ParticleSystemEx::Init(Section* sec, const std::vector<Particle> particles, unsigned int numParticlesPerEmitter, std::vector<std::string> shaderVarsUpdate, std::vector<std::string> shaderVarsBillboard)
	{
		if (particles.size() == 0)
			return false;
		m_numParticles = static_cast<unsigned int>(particles.size());
		m_numParticlesPerEmitter = numParticlesPerEmitter;

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
			glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * m_numParticles, particles.data(), GL_DYNAMIC_DRAW);	// Allocate mem, uploading all the particles
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
		}
	
		m_memUsed = (float)(2 * sizeof(Particle) * m_numParticles) / 1024.0f / 1024.0f;

		// Setup Vertex Attribute formats
		// Definitions for Update shader Binding
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));
		glVertexAttribBinding(LOC_TYPE, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_ID);
		glVertexAttribIFormat(LOC_ID, 1, GL_INT, offsetof(Particle, ID));
		glVertexAttribBinding(LOC_ID, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_INITPOSITION);
		glVertexAttribFormat(LOC_INITPOSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, InitPosition));
		glVertexAttribBinding(LOC_INITPOSITION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Position));
		glVertexAttribBinding(LOC_POSITION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_RANDOMNESS);
		glVertexAttribFormat(LOC_RANDOMNESS, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Randomness));
		glVertexAttribBinding(LOC_RANDOMNESS, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_ROTATION);
		glVertexAttribFormat(LOC_ROTATION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Rotation));
		glVertexAttribBinding(LOC_ROTATION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_INITCOLOR);
		glVertexAttribFormat(LOC_INITCOLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, InitColor));
		glVertexAttribBinding(LOC_INITCOLOR, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Color));
		glVertexAttribBinding(LOC_COLOR, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_AGE);
		glVertexAttribFormat(LOC_AGE, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, Age));
		glVertexAttribBinding(LOC_AGE, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_LIFE);
		glVertexAttribFormat(LOC_LIFE, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, Life));
		glVertexAttribBinding(LOC_LIFE, BINDING_UPDATE);

		// Definitions for Billboard shader Binding
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(Particle, Type));
		glVertexAttribBinding(LOC_TYPE, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Position));
		glVertexAttribBinding(LOC_POSITION, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Color));
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
		for (int i = 0; i < shaderVarsUpdate.size(); i++) {
			m_varsParticleSystem->ReadString(shaderVarsUpdate[i]);
		}
		// Validate and set particle system shader variables values
		m_varsParticleSystem->validateAndSetValues();

		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_varsBillboard = new ShaderVars(sec, m_billboardShader);
		// Read the shader variables
		for (int i = 0; i < shaderVarsBillboard.size(); i++) {
			m_varsBillboard->ReadString(shaderVarsBillboard[i]);
		}
		// Validate and set billboard shader variables values (texture, particle size, etc...)
		m_varsBillboard->validateAndSetValues();
		
		return !DEMO_checkGLError();
	}


	void ParticleSystemEx::Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		glBindVertexArray(m_VAO);
		UpdateParticles(time, model);
		RenderParticles(model, view, projection);
		glBindVertexArray(0);

		m_currVB = m_currTFB;
		m_currTFB = (m_currTFB + 1) & 0x1;
	}

	void ParticleSystemEx::UpdateParticles(float time, const glm::mat4& model)
	{
	
		m_particleSystemShader->use();
		m_particleSystemShader->setValue("u_m4Model", model);
		m_particleSystemShader->setValue("u_fTime", time);
		m_particleSystemShader->setValue("u_iRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
		m_particleSystemShader->setValue("u_uiNumMaxParticles", m_numParticles);
		m_particleSystemShader->setValue("u_uiNumParticlesPerEmitter", m_numParticlesPerEmitter);
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
			glDrawArrays(GL_POINTS, 0, m_numParticles);
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

	void ParticleSystemEx::RenderParticles(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
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
	}

	bool ParticleSystemEx::initShaderBillboard()
	{
		m_billboardShader = DEMO->m_shaderManager.addShader(m_pathBillboard);
		if (m_billboardShader)
			return true;
		return false;
	}

	bool ParticleSystemEx::initShaderParticleSystem()
	{
		// TODO: Poner los varyings como parte del init del shader, incluso en la seccion... o que lo detecte solo??
		m_particleSystemShader = DEMO->m_shaderManager.addShader(m_pathUpdate,
			{ "o_Type", "o_ID", "o_InitPosition", "o_Position", "o_Randomness", "o_Rotation", "o_InitColor", "o_Color", "o_Age", "o_Life" });

		if (m_particleSystemShader)
			return true;
		return false;
	}

	// TODO: Investigate if we should move this "Random texture 1D generator" to the TextureManager class
	bool ParticleSystemEx::initRandomTexture(uint32_t Size)
	{
		glm::vec3* pRandomData = new glm::vec3[Size];
		uint32_t randomSeed = 0;
		for (uint32_t i = 0; i < Size; i++) {
			randomSeed += i;
			pRandomData[i] = Utils::randomVec3_05(randomSeed);
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

	void ParticleSystemEx::bindRandomTexture(GLuint TexUnit)
	{
		glBindTextureUnit(TexUnit, m_textureRandID);
	}

}