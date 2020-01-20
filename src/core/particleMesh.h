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
		glm::vec3   position;
		glm::vec3   velocity;
		int         ID;
	};

	ParticleMesh(int numParticles);
	~ParticleMesh();

	bool startup();
	void render(float currentTime, const glm::mat4& PVM);
	void shutdown();

	int         m_numParticles;

private:
	GLuint      m_particleBuffer;
	PARTICLE*   m_mappedBuffer;
	PARTICLE*   m_particles[2];
	int         m_frameIndex;
	GLuint      m_vao;

	void initialize_particles(void);
	void update_particles(float deltaTime);
};

#endif