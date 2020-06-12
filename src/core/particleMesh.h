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
	};

	ParticleMesh(int numParticles);
	~ParticleMesh();

	bool startup(std::vector<glm::vec3> Pos = { });
	void render(float currentTime);
	void shutdown();

	int         m_numParticles;

private:
	GLuint      m_particleBuffer;
	PARTICLE*   m_particles;
	GLuint      m_vao;

	void initialize_particles(std::vector<glm::vec3> Pos = { });
};

#endif