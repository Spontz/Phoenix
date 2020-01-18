// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEMEVO_H
#define PARTICLESYSTEMEVO_H

#include "main.h"
#include "core/shadervars.h"

class ParticleSystemEvo
{
public:

	struct PARTICLE
	{
		glm::vec3   position;
		glm::vec3   velocity;
		int         ID;
	};

	ParticleSystemEvo(int numParticles, string shaderPath);
	~ParticleSystemEvo();

	bool startup();
	void render(float currentTime, const glm::mat4& PVM);
	void shutdown();

private:
	int         m_numParticles;
	string      m_shaderPath;
	GLuint      m_particleBuffer;
	PARTICLE*   m_mappedBuffer;
	PARTICLE*   m_particles[2];
	int         m_frameIndex;
	GLuint      m_vao;
	int         m_shader;

	void initialize_particles(void);
	void update_particles(float deltaTime);
};

#endif