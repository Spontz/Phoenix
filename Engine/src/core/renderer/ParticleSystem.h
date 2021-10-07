// ParticleSystem.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/renderer/ShaderVars.h"

#include <array>

namespace Phoenix {

	class ParticleSystem
	{
	public:
		enum struct ParticleType : int32_t {
			None = 0,
			Emitter = 1,
			Shell = 2
		};

		struct Particle
		{
			glm::vec3 Pos;		// Position:	loc 0 (vec3)
			glm::vec3 Vel;		// Velocity:	loc 1 (vec3)
			glm::vec3 Col;		// Color:		loc 2 (vec3)
			float lifeTime;		// lifeTime:	loc 3 (float)
			ParticleType Type;	// type:		loc 4 (int32)
		};


		ParticleSystem(std::string shaderPath);
		~ParticleSystem();

		bool Init(Section* sec, const std::vector<Particle> emitters, float emissionTime, float particleLifeTime, std::vector<std::string> billboardShaderVars);
		void Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
		void UpdateEmitters(float deltaTime);

		int32_t getNumGenParticles() { return m_numGenParticles; };
		int32_t getNumMaxParticles() { return m_numMaxParticles; };
		float getMemUsedInMb() { return m_memUsed; };

		glm::vec3 force;	// Force to be applied globally
		glm::vec3 color;	// Color to be applied on each particle when it's emitted

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
		unsigned int			m_textureRandID;	// TODO: This will be removed once is included in the textureManager
		std::string				m_shaderPath;
		std::string				m_pathBillboard;
		ShaderVars*				m_varsBillboard;	// Billboard Shader variables

		std::string				m_pathUpdate;
		unsigned int			m_numMaxParticles;	// Number of maximum particles
		unsigned int			m_numEmitters;		// Number of emmiters
		float					m_emissionTime;
		float					m_particleLifeTime;

		bool					m_isFirst;
		unsigned int			m_currVB;			// Variable that indicates the current VB (The value is 0 or 1)
		unsigned int			m_currTFB;			// Variable that indicates the current TFB (The value is 0 or 1)
		GLuint					m_VAO;				// Vertex Array Object buffer
		std::array<GLuint, 2>	m_particleBuffer;	// Vertex Buffer: input
		std::array<GLuint, 2>	m_transformFeedback;// Transform feedback buffers: output
		Particle*				m_emitterData;		// Emitter info (for updating emitters positions)
		SP_Shader				m_particleSystemShader;
		SP_Shader				m_billboardShader;

		GLuint					m_queryPrimitives;	// Query to know how many primitives have been written
		GLuint					m_numGenParticles;	// Number of particles generated
		
		float					m_memUsed;			// Memory used (in Mb)
	};
}