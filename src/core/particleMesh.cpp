// particleMesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleMesh.h"

using namespace std;

ParticleMesh::ParticleMesh(int numParticles)
{
    this->m_numParticles = numParticles;
}

ParticleMesh::~ParticleMesh()
{
    shutdown();
}

#define BINDING 0
#define LOC_ID 0

bool ParticleMesh::startup()
{
    // Application memory particle buffers (double buffered)
    m_particles = new PARTICLE[m_numParticles];

    // Load the ID's of the particles
    initialize_particles();

    // Create GPU buffer in read-only mode (faster)
    glGenBuffers(1, &m_particleBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
    glBufferStorage(GL_ARRAY_BUFFER, m_numParticles * sizeof(PARTICLE), m_particles, GL_MAP_READ_BIT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindVertexBuffer(BINDING, m_particleBuffer, 0, sizeof(PARTICLE));
    
    glEnableVertexAttribArray(LOC_ID); 
    glVertexAttribIFormat(LOC_ID, 1, GL_INT, offsetof(PARTICLE, ID));
    glVertexAttribBinding(LOC_ID, BINDING);

    return true;
}

void ParticleMesh::initialize_particles(void)
{
    // Load the ID on each particle
    for (int i = 0; i < m_numParticles; i++)
    {
        m_particles[i].ID = i;
    }
}


void ParticleMesh::render(float currentTime)
{
    // Bind our vertex arrays
    glBindVertexArray(m_vao);

    // Draw! (shader needs to be called in advance!)
    glDrawArrays(GL_POINTS, 0, m_numParticles);
	
	glBindVertexArray(0);
}

void ParticleMesh::shutdown()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1, &m_particleBuffer);

    delete[] m_particles;
}
