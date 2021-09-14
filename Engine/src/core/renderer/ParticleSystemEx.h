// ParticleSystemEx.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/renderer/ShaderVars.h"

#include <array>

namespace Phoenix {


enum struct ParticleType : int32_t {
	None = 0,
	Emitter = 1,
	Shell = 2
};

	struct ParticleEx
	{
		ParticleType Type;			// Particle Type:		loc 0
		int32_t		RandomID;		// Particle ID:			loc 1
		glm::vec3	InitPosition;	// Initial position:	loc 2
		glm::vec3	Position;		// Position:			loc 3
		glm::vec3	Rotation;		// Rotation:			loc 4
		glm::vec3	Color;			// Color:				loc 5
		float		Age;			// Age (0 to Life):		loc 6
		float		Life;			// Life:				loc 7
	};

	class ParticleSystemEx
	{
	public:
		ParticleSystemEx(std::string shaderPath, unsigned int	numMaxParticles, unsigned int numEmitters, float particleLifeTime);
		~ParticleSystemEx();

		bool InitParticleSystem(Section* sec, const std::vector<ParticleEx> emitter, std::vector<std::string>	billboardShaderVars);
		void Render(float deltaTime, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

		int32_t getNumParticles() { return m_numParticles; };
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

	private:

		float					m_time;
		unsigned int			m_textureRandID;	// TODO: This will be removed once is included in the textureManager
		std::string				m_shaderPath;
		std::string				m_pathBillboard;
		ShaderVars*				m_varsBillboard;	// Billboard Shader variables

		std::string				m_pathUpdate;
		unsigned int			m_numMaxParticles;	// Number of maximum particles
		unsigned int			m_numEmitters;		// Number of emmiters
		float					m_particleLifeTime;

		bool					m_isFirst;
		unsigned int			m_currVB;			// Variable that indicates the current VB (The value is 0 or 1)
		unsigned int			m_currTFB;			// Variable that indicates the current TFB (The value is 0 or 1)
		GLuint					m_VAO;				// Vertex Array Object buffer
		std::array<GLuint, 2>	m_particleBuffer;	// Vertex Buffer: input
		std::array<GLuint, 2>	m_transformFeedback;// Transform feedback buffers: output
		ParticleEx*				m_emitterData;		// Emitter info (for updating emitters positions)
		SP_Shader				m_particleSystemShader;
		SP_Shader				m_billboardShader;

		GLuint					m_queryPrimitives;	// Query to know how many primitives have been written
		GLuint					m_numParticles;		// Number of particles generated
		
		float					m_memUsed;			// Memory used (in Mb)
	};
}