// ParticleMesh.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/renderer/ShaderVars.h"
#include "core/renderer/VertexArray.h"

namespace Phoenix {

	class ParticleMesh
	{
	public:

		enum struct ParticleType : int32_t {
			None = 0,
			Emitter = 1,
			Shell = 2
		};

		struct Particle
		{
			ParticleType	Type;			// Particle Type:		loc 0
			int32_t			ID;				// Particle ID:			loc 1
			glm::vec3		InitPosition;	// Initial position:	loc 2
			glm::vec3		Randomness;		// Ramdomness:			loc 3
			glm::vec4		InitColor;		// Initial color:		loc 4
			float			Life;			// Life:				loc 5
		};

		ParticleMesh();
		~ParticleMesh();

		bool init(std::vector<Particle> const& Part);
		void render();
		void shutdown();
		float getMemUsedInMb() { return m_memUsed; };

	private:

		uint32_t		m_numParticles;
		Particle* m_particles = nullptr;
		SP_VertexArray	m_VertexArray;

		float			m_memUsed;					// Memory used (in Mb)
	};
}