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
		int		ID;	// Particle ID, for being used in the shaders
	};

	ParticleMesh(int numParticles);
	~ParticleMesh();

	bool startup();
	void render(float currentTime);
	void shutdown();

	int         m_numParticles;

private:
	GLuint      m_particleBuffer;
	PARTICLE*   m_particles;
	GLuint      m_vao;

	void initialize_particles(void);
};

#endif