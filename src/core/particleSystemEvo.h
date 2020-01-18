// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEMEVO_H
#define PARTICLESYSTEMEVO_H

#include "main.h"
#include "core/shadervars.h"

//#define PARTICLE_COUNT 2048

/*
struct PARTICLE
{
	glm::vec3 position;
	glm::vec3 velocity;
};*/

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
    GLuint      particle_buffer;
    PARTICLE*   mapped_buffer;
    PARTICLE*   particles[2];
    int         frame_index;
    GLuint      vao;
    int         m_shader;
    bool        use_omp;

    void initialize_particles(void);
    void update_particles(float deltaTime);
    void update_particles_omp(float deltaTime);
    void onKey(int key, int action);
	
};

#endif