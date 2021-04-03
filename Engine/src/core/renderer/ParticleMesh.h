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

		struct Particle
		{
			glm::vec3	Pos;	// Particle initial position
			glm::vec4	Col;	// Particle initial color
		};

		ParticleMesh(int numParticles);
		~ParticleMesh();

		bool startup(std::vector<Particle> const& Pos = {});
		void render(float currentTime);
		void shutdown();

		int				m_numParticles;

	private:
		Particle* m_particles;
		VertexArray* m_VertexArray;

		void initialize_particles(std::vector<Particle> Pos = { });
	};
}