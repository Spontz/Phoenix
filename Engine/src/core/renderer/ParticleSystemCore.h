// ParticleSystemCore.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/renderer/ShaderVars.h"

#include <array>

namespace Phoenix {

	class ParticleSystemCore
	{
	public:
		struct Particle
		{
			glm::vec3	Pos;		// loc 0: Position
			glm::vec3	Vel;		// loc 1: Velocity
			float		StartTime;	// loc 2: StartTime
			glm::vec3	InitVel;	// loc 3: Initial Veolicity
			glm::vec3	Col;		// loc 4: Color
			float		Life;		// loc 5: Life
			Particle()
				:Pos(0), Vel(0), StartTime(0), InitVel(0), Col(0), Life(0) {}
		};


		ParticleSystemCore(std::string particleSystemShader, std::string billboardShader);
		~ParticleSystemCore();

		bool Init(Section* sec, float emissionTime, float particleLifeTime, std::vector<std::string> shaderVars);
		void RestartParticles();
		void Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
		void UpdateEmitter(ParticleSystemCore::Particle emitter);

		int32_t getNumGenParticles() { return m_numGenParticles; };
		int32_t getNumMaxParticles() { return m_numMaxParticles; };
		float getMemUsedInMb() { return m_memUsed; };

		Particle	m_emitter;	// Emitter of all the particles
		glm::vec3	force;		// Force to be applied globally
		float		randomness;	// Global randomness to apply to particles

	private:

		void UpdateParticles(float deltaTime, const glm::mat4& model);
		void RenderParticles(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

		bool initShaderBillboard();		// For drawing the quads using geometry shaders
		bool initShaderParticleSystem();// For drawing the particles using geometry shaders

		bool initRandomTexture(unsigned int Size); //TODO: Add it into the textureManager
		void bindRandomTexture(GLuint TexUnit = 0);//TODO: this should be removed once is included in the textureManager

		void debugLogBufferData();

	private:

		float					m_time;
		unsigned int			m_textureRandID;			// TODO: This will be removed once is included in the textureManager

		std::string				m_pathBillboard;
		ShaderVars*				m_varsBillboard;			// Billboard Shader variables

		std::string				m_pathUpdate;
		ShaderVars*				m_varsParticleSystem;		// ParticleSystem Shader variables

		unsigned int			m_numMaxParticles;			// Number of maximum particles
		
		float					m_emissionTime;
		float					m_particleLifeTime;

		bool					m_isFirst;
		unsigned int			m_currVB;					// Variable that indicates the current VB (The value is 0 or 1)
		unsigned int			m_currTFB;					// Variable that indicates the current TFB (The value is 0 or 1)
		GLuint					m_VAO;						// Vertex Array Object buffer
		std::array<GLuint, 2>	m_particleBuffer;			// Vertex Buffer: input
		std::array<GLuint, 2>	m_transformFeedback;		// Transform feedback buffers: output
		Particle*				m_emitterData;				// Emitter info (for updating emitters positions)
		SP_Shader				m_particleSystemShader;
		SP_Shader				m_billboardShader;

		GLuint					m_queryPrimitives;			// Query to know how many primitives have been written
		GLuint					m_numGenParticles;			// Number of particles generated
		
		float					m_memUsed;					// Memory used (in Mb)
	};
}