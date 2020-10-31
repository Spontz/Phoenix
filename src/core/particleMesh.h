// particleMesh.h
// Spontz Demogroup

#ifndef PARTICLEMESH_H
#define PARTICLEMESH_H

#include "main.h"
#include "core/shadervars.h"

class ParticleMesh
{
public:

	struct PARTICLE
	{
		glm::vec3	Pos;	// Particle initial position
		glm::vec4	Col;	// Particle initial color
	};

	ParticleMesh(int numParticles);
	~ParticleMesh();

	bool startup(std::vector<PARTICLE> Pos = { });
	void render(float currentTime);
	void shutdown();

	int         m_numParticles;

private:
	GLuint      m_particleBuffer;
	PARTICLE*   m_particles;
	GLuint      m_vao;

	void initialize_particles(std::vector<PARTICLE> Pos = { });
};

#endif