// ParticleSystemCore.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleSystemCore.h"
#include <math.h>

namespace Phoenix {

	// TODO: QUITAR ESTO Y USAR LOS VB QUE TENEMOS EN EL ENGINE!!
	constexpr int LOC_POSITION = 0;
	constexpr int LOC_VELOCITY = 1;
	constexpr int LOC_STARTTIME = 2;
	constexpr int LOC_INITVEL = 3;
	constexpr int LOC_COLOR = 4;
	
	constexpr int BINDING_UPDATE = 0;
	constexpr int BINDING_BILLBOARD = 1;

	ParticleSystemCore::ParticleSystemCore(std::string particleSystemShader, std::string billboardShader)
	{
		m_varsParticleSystem = nullptr; 
		m_varsBillboard = nullptr;
		
		force = glm::vec3(0, 0, 0);
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

		m_amountParticles = 0;
		
		m_emissionTime = 0;
		m_particleLifeTime = 0;
		
		m_transformFeedback = {};
		m_particleBuffer = {};
	}


	ParticleSystemCore::~ParticleSystemCore()
	{
		if (m_transformFeedback[0] != 0)
			glDeleteTransformFeedbacks(2, m_transformFeedback.data());

		if (m_particleBuffer[0] != 0)
			glDeleteBuffers(2, m_particleBuffer.data());

		if (m_queryPrimitives != 0)
			glDeleteQueries(1, &m_queryPrimitives);

		if (m_varsBillboard)
			delete m_varsBillboard;

		if (m_varsParticleSystem)
			delete m_varsParticleSystem;
	}


	bool ParticleSystemCore::Init(Section* sec, float emissionTime,float particleLifeTime, std::vector<std::string> shaderVars)
	{
		if (emissionTime <= 0)
			return false;

		m_emissionTime = emissionTime; 
		m_particleLifeTime = particleLifeTime;
		
		m_amountParticles = static_cast<unsigned int>(ceil(m_particleLifeTime * (1.0f / m_emissionTime)));

		if (m_amountParticles <= 0)
			return false;
		
		// Init particles data
		Particle* particles = new Particle[m_amountParticles];
		for (uint32_t i=0; i < m_amountParticles; i++) {
			particles[i].InitVel = glm::vec3(0, 0, 0);
			particles[i].Vel = particles[i].InitVel;
			particles[i].Col = glm::vec3(1,1,1);
			particles[i].StartTime = i * (m_particleLifeTime / (float)m_amountParticles);
		}

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
			glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * m_amountParticles, NULL, GL_DYNAMIC_DRAW);	// Allocate mem, uploading an empty buffer for all the particles
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * m_amountParticles, particles);		// Upload only the emitters to the Buffer
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
		}
	
		m_memUsed = (float)(2 * sizeof(Particle) * m_amountParticles) / 1024.0f / 1024.0f;

		// Setup Vertex Attribute formats
		// Definitions for Update shader Binding
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Pos));	// Position (12 bytes)
		glVertexAttribBinding(LOC_POSITION, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_VELOCITY);
		glVertexAttribFormat(LOC_VELOCITY, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Vel));	// Velocity (12 bytes)
		glVertexAttribBinding(LOC_VELOCITY, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_STARTTIME);
		glVertexAttribFormat(LOC_STARTTIME, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, StartTime));	// Start Time (4 bytes)
		glVertexAttribBinding(LOC_STARTTIME, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_INITVEL);
		glVertexAttribFormat(LOC_INITVEL, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, InitVel));	// Initial Velocity (12 bytes)
		glVertexAttribBinding(LOC_INITVEL, BINDING_UPDATE);

		glEnableVertexAttribArray(LOC_COLOR);
		glVertexAttribFormat(LOC_COLOR, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Col));	// Color (12 bytes)
		glVertexAttribBinding(LOC_COLOR, BINDING_UPDATE);

		// Definitions for Billboard shader Binding
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[0], 0, sizeof(Particle));

		glEnableVertexAttribArray(LOC_POSITION);
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, Pos));	// Position (12 bytes)
		glVertexAttribBinding(LOC_POSITION, BINDING_BILLBOARD);

		glEnableVertexAttribArray(LOC_STARTTIME);
		glVertexAttribFormat(LOC_STARTTIME, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, StartTime));	// Start Time (4 bytes)
		glVertexAttribBinding(LOC_STARTTIME, BINDING_BILLBOARD);

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
		m_particleSystemShader->setValue("fEmissionTime", m_emissionTime); // Time between emissions
		m_particleSystemShader->setValue("fParticleLifetime", m_particleLifeTime);

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
		// Validate and set particleSystem shader variables values
		m_varsParticleSystem->validateAndSetValues();

		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_varsBillboard = new ShaderVars(sec, m_billboardShader);
		// Read the shader variables
		for (int i = 0; i < shaderVars.size(); i++) {
			m_varsBillboard->ReadString(shaderVars[i]);
		}
		// Validate and set billboard shader variables values (texture, particle size, etc...)
		m_varsBillboard->validateAndSetValues();
		
		// Delete the temporary particles data, all is now in the GPU
		delete[] particles;

		return !DEMO_checkGLError();
	}

	void ParticleSystemCore::RestartParticles(float runTime)
	{
		//Logger::info(LogLevel::low,"Restarted particle System!");
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
		Particle* particleData = (Particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle) * m_amountParticles, GL_MAP_WRITE_BIT);

		for (uint32_t i = 0; i < m_amountParticles; i++) {
			particleData[i].Pos = m_emitter.Pos;
			particleData[i].InitVel = glm::vec3(0, 0, 0);// RandomVec3();
			particleData[i].Vel = particleData[i].InitVel;
			particleData[i].Col = glm::vec3(1, 1, 1);
			particleData[i].StartTime = runTime + ((float)i * (m_particleLifeTime / (float)m_amountParticles));
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		m_time = runTime;
		m_isFirst = true;
		//this->debugLogBufferData();
	}


	void ParticleSystemCore::Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		m_time += deltaTime;

		glBindVertexArray(m_VAO);
		UpdateParticles(deltaTime, model);
		RenderParticles(model, view, projection);
		glBindVertexArray(0);

		m_currVB = m_currTFB;
		m_currTFB = (m_currTFB + 1) & 0x1;
	}

	void ParticleSystemCore::UpdateEmitter(ParticleSystemCore::Particle emitter)
	{
		m_emitter = emitter;
	}


	void ParticleSystemCore::UpdateParticles(float deltaTime, const glm::mat4& model)
	{
		//UpdateEmitters(deltaTime); // For debugging only: Overwrittes the emitter position
		m_particleSystemShader->use();
		m_particleSystemShader->setValue("u_v3Position", m_emitter.Pos);
		m_particleSystemShader->setValue("u_v3Color", m_emitter.Col);
		m_particleSystemShader->setValue("u_m4Model", model);
		m_particleSystemShader->setValue("u_fTime", m_time);
		m_particleSystemShader->setValue("u_fDeltaTime", deltaTime);
		m_particleSystemShader->setValue("u_fEmissionTime", m_emissionTime);
		m_particleSystemShader->setValue("u_fParticleLifetime", m_particleLifeTime);
		m_particleSystemShader->setValue("u_fRamndomness", randomness);
		m_particleSystemShader->setValue("u_v3Force", force);
		m_varsParticleSystem->setValues();

		glEnable(GL_RASTERIZER_DISCARD);	// Stop drawing on the screen
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

		// Use Binding for particles (all attributes)
		glBindVertexBuffer(BINDING_UPDATE, m_particleBuffer[m_currVB], 0, sizeof(Particle));

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_queryPrimitives);

		glBeginTransformFeedback(GL_POINTS);

		if (m_isFirst) {
			glDrawArrays(GL_POINTS, 0, m_amountParticles);
			m_isFirst = false;
		}
		else {
			glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]); // TODO: Get rid of this?
		}

		glEndTransformFeedback();

		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glGetQueryObjectuiv(m_queryPrimitives, GL_QUERY_RESULT, &m_numGenParticles);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	}

	void ParticleSystemCore::RenderParticles(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
	{
		//Use the billboard shader and send variables
		m_billboardShader->use();
		m_billboardShader->setValue("model", model);						// Set Model Matrix
		m_billboardShader->setValue("view", view);							// Set View Matrix
		m_billboardShader->setValue("projection", projection);				// Set Projection Matrix
		m_billboardShader->setValue("u_fTime", m_time);
		m_billboardShader->setValue("u_fParticleLifetime", m_particleLifeTime);
		m_varsBillboard->setValues();

		glDisable(GL_RASTERIZER_DISCARD);	// Start drawing on the screen
		glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

		// Use binding for billboard (only Position and color attributes)
		glBindVertexBuffer(BINDING_BILLBOARD, m_particleBuffer[m_currTFB], 0, sizeof(Particle));

		glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
	}

	bool ParticleSystemCore::initShaderBillboard()
	{
		m_billboardShader = DEMO->m_shaderManager.addShader(m_pathBillboard);
		if (m_billboardShader)
			return true;
		return false;
	}

	bool ParticleSystemCore::initShaderParticleSystem()
	{
		m_particleSystemShader = DEMO->m_shaderManager.addShader(m_pathUpdate,
			{ "o_Position", "o_Velocity", "o_StartTime", "o_InitVelocity", "o_Color"});

		if (m_particleSystemShader)
			return true;
		return false;
	}

	void ParticleSystemCore::debugLogBufferData()
	{
		/// DEBUG ONLY
		/// Output Buffer values
		constexpr LogLevel logLevel = LogLevel::low;
		Logger::info(logLevel, "Buffers Current VB:{}, current TFB:{}", m_currVB, m_currTFB);
		for (unsigned int i = 0; i < 2; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
			Particle* p = (Particle*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			Logger::info(logLevel, "SETUP Buffer {}", i);
			for (unsigned int j = 0; j < m_amountParticles; j++)
			{
				const auto& part = p[j];
				Logger::info(
					logLevel,
					"Part {}: ST:{:.2f}, P:({:.2f},{:.2f},{:.2f}), C:({:.2f},{:.2f},{:.2f})", 
					j,
					part.StartTime,
					part.Pos.x,
					part.Pos.y,
					part.Pos.z,
					part.Col.x,
					part.Col.y,
					part.Col.z
				);
			}
			Logger::info(logLevel, "");
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
	}
}
