// ParticleSystemEx.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleSystemEx.h"

namespace Phoenix {

#define RANDOM_TEXTURE_UNIT 0

// TODO: QUITAR ESTO Y USAR LOS VB QUE TENEMOS EN EL ENGINE!!
#define LOC_TYPE		 0
#define LOC_ID			 1
#define LOC_INITPOSITION 2
#define LOC_POSITION 3
#define LOC_ROTATION 4
#define LOC_COLOR 5
#define LOC_AGE 6
#define LOC_LIFE 7

#define BINDING_UPDATE	0
#define BINDING_BILLBOARD	1

	ParticleSystemEx::ParticleSystemEx(std::string shaderPath, unsigned int	numMaxParticles, unsigned int numEmitters, float particleLifeTime)
	{
		m_varsBillboard = nullptr;

		force = glm::vec3(0, 0, 0);
		color = glm::vec3(1, 1, 1);

		m_currVB = 0;
		m_currTFB = 1;
		m_isFirst = true;
		m_time = 0;

		m_queryPrimitives = 0;
		m_numParticles = 0;

		m_memUsed = 0;

		m_shaderPath = shaderPath;
		m_pathBillboard = shaderPath + "/billboard.glsl";
		m_pathUpdate = shaderPath + "/update.glsl";

		m_numMaxParticles = numMaxParticles; // Should be at least greather than: numEmitters + numEmitters*gShellLifetime*(1/gLauncherLifetime)
		m_numEmitters = numEmitters;
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
	}


	bool ParticleSystemEx::InitParticleSystem(Section* sec, const std::vector<ParticleEx> particles, std::vector<std::string> billboardShaderVars)
	{
		if (m_numEmitters == 0)
			return false;

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
			glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleEx) * m_numMaxParticles, particles.data(), GL_DYNAMIC_DRAW);	// Allocate mem, uploading all the particles
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
		}
	
		m_memUsed = (float)(2 * sizeof(ParticleEx) * m_numMaxParticles) / 1024.0f / 1024.0f;

		// Setup Vertex Attribute formats
		// Definitions for Update shader Binding
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[0], 0, sizeof(ParticleEx));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(ParticleEx, Type));
		glVertexAttribBinding(LOC_TYPE, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_ID);
		glVertexAttribIFormat(LOC_ID, 1, GL_INT, offsetof(ParticleEx, RandomID));
		glVertexAttribBinding(LOC_ID, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_INITPOSITION);
		glVertexAttribFormat(LOC_INITPOSITION, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, InitPosition));
		glVertexAttribBinding(LOC_INITPOSITION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Position));
		glVertexAttribBinding(LOC_POSITION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_ROTATION);
		glVertexAttribFormat(LOC_ROTATION, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Rotation));
		glVertexAttribBinding(LOC_ROTATION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Color));
		glVertexAttribBinding(LOC_COLOR, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_AGE);
		glVertexAttribFormat(LOC_AGE, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Age));
		glVertexAttribBinding(LOC_AGE, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_LIFE);
		glVertexAttribFormat(LOC_LIFE, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Life));
		glVertexAttribBinding(LOC_LIFE, BINDING_UPDATE);

		// Definitions for Billboard shader Binding
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[0], 0, sizeof(ParticleEx));

		glEnableVertexAttribArray(LOC_TYPE);
		glVertexAttribIFormat(LOC_TYPE, 1, GL_INT, offsetof(ParticleEx, Type));
		glVertexAttribBinding(LOC_TYPE, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Position));
		glVertexAttribBinding(LOC_POSITION, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleEx, Color));
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

		if (!initRandomTexture(1000)) {
			return false;
		}

		bindRandomTexture(RANDOM_TEXTURE_UNIT);

		// BILLBOARD shader
		if (!initShaderBillboard()) {
			return false;
		}

		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_varsBillboard = new ShaderVars(sec, m_billboardShader);
		// Read the shader variables
		for (int i = 0; i < billboardShaderVars.size(); i++) {
			m_varsBillboard->ReadString(billboardShaderVars[i].c_str());
		}
		// Set billboard shader variables values (texture, particle size, etc...)
		m_varsBillboard->setValues();
		
		return !GLDRV_checkError();
	}


	void ParticleSystemEx::Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		m_time += deltaTime;

		glBindVertexArray(m_VAO);
		UpdateParticles(deltaTime, model);
		RenderParticles(model, view, projection);
		glBindVertexArray(0);

		m_currVB = m_currTFB;
		m_currTFB = (m_currTFB + 1) & 0x1;
	}

	void ParticleSystemEx::UpdateParticles(float deltaTime, const glm::mat4& model)
	{
	
		m_particleSystemShader->use();
		m_particleSystemShader->setValue("model", model);
		m_particleSystemShader->setValue("gTime", m_time);
		m_particleSystemShader->setValue("gDeltaTime", deltaTime);
		m_particleSystemShader->setValue("gRandomTexture", RANDOM_TEXTURE_UNIT); // TODO: fix... where to store the random texture unit?
		m_particleSystemShader->setValue("uiNumMaxParticles", m_numMaxParticles);
		m_particleSystemShader->setValue("fParticleLifetime", m_particleLifeTime);
		m_particleSystemShader->setValue("gForce", force);
		m_particleSystemShader->setValue("gColor", color);


		bindRandomTexture(RANDOM_TEXTURE_UNIT);

		glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

		// Use Binding for particles (all attributes)
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[m_currVB], 0, sizeof(ParticleEx));

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_queryPrimitives);

		glBeginTransformFeedback(GL_POINTS);

		if (m_isFirst) {
			glDrawArrays(GL_POINTS, 0, m_numMaxParticles);
			m_isFirst = false;
		}
		else {
			glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
		}

		glEndTransformFeedback();

		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glGetQueryObjectuiv(m_queryPrimitives, GL_QUERY_RESULT, &m_numParticles);

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
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[m_currTFB], 0, sizeof(ParticleEx));

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
		m_particleSystemShader = DEMO->m_shaderManager.addShader(m_pathUpdate,
			{ "o_Type", "o_ID", "o_InitPosition", "o_Position", "o_Rotation", "o_Color", "o_Age", "o_Life" });

		if (m_particleSystemShader)
			return true;
		return false;
	}

	// TODO: Investigate if we should move this "Random texture 1D generator" to the TextureManager class
	static float RandomFloat()
	{
		float Max = RAND_MAX;
		return ((float)rand() / Max);
	}

	bool ParticleSystemEx::initRandomTexture(unsigned int Size)
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

	void ParticleSystemEx::bindRandomTexture(GLuint TexUnit)
	{
		glBindTextureUnit(TexUnit, m_textureRandID);
	}

}