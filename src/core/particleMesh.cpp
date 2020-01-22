// particleMesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleMesh.h"

using namespace std;

ParticleMesh::ParticleMesh(int numParticles)
{
    m_frameIndex = 0;
    this->m_numParticles = numParticles;
}

ParticleMesh::~ParticleMesh()
{
    shutdown();
}

#define BINDING 0
#define LOC_POSITION 0
#define LOC_ID 1

bool ParticleMesh::startup()
{
    // Application memory particle buffers (double buffered)
    m_particles[0] = new PARTICLE[m_numParticles];
    m_particles[1] = new PARTICLE[m_numParticles];

    // Create GPU buffer
    glGenBuffers(1, &m_particleBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
	glBufferStorage(GL_ARRAY_BUFFER, m_numParticles * sizeof(PARTICLE), NULL,  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_mappedBuffer = (PARTICLE*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_numParticles * sizeof(PARTICLE), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    initialize_particles();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindVertexBuffer(BINDING, m_particleBuffer, 0, sizeof(PARTICLE));
    
    glEnableVertexAttribArray(LOC_POSITION);
    glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(PARTICLE, position));
    glVertexAttribBinding(LOC_POSITION, BINDING);

    glEnableVertexAttribArray(LOC_ID); 
    glVertexAttribIFormat(LOC_ID, 1, GL_INT, offsetof(PARTICLE, ID));
    glVertexAttribBinding(LOC_ID, BINDING);

    return true;
}

void ParticleMesh::initialize_particles(void)
{
    for (int i = 0; i < m_numParticles; i++)
    {
        m_particles[0][i].position = glm::vec3(0.0);
        m_particles[0][i].velocity = glm::vec3(0.0);
        m_particles[0][i].ID = i;

        m_mappedBuffer[i] = m_particles[0][i];
    }
}


static float m_time = 0;
void ParticleMesh::update_particles(float deltaTime)
{
    m_time += deltaTime;
    // Double buffer source and destination
    const PARTICLE* const __restrict src = m_particles[m_frameIndex & 1];
    PARTICLE* const __restrict dst = m_particles[(m_frameIndex + 1) & 1];

    /*
    // For each particle in the system
    for (unsigned int i = 0; i < m_numParticles; i++)
    {
        // Get my own data
        const PARTICLE& me = src[i];

        float sphere = 2 * 3.1415f * ((float)(i) / ((float)m_numParticles));
        dst[i].position = me.position;// glm::vec3(0.5f * sin(sphere), 1.0 * sin(m_time), 0.5f * cos(sphere));

        //dst[i].position = me.position+glm::vec3(10 * sin(deltaTime), 0, 0);// me.velocity;
        //dst[i].velocity = me.velocity;
        
        // Write to mapped buffer
        mapped_buffer[i].position = dst[i].position;
    }
    */

    // Count frames so we can double buffer next frame
    m_frameIndex++;
}

void ParticleMesh::render(float currentTime)
{
    // Update particle positions
    //update_particles(deltaTime);
    
    // Bind our vertex arrays
    glBindVertexArray(m_vao);

    // Let OpenGL know we've changed the contents of the buffer
    //glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, m_numParticles * sizeof(PARTICLE));

    // Draw! (shader needs to be called in advance!)
    glDrawArrays(GL_POINTS, 0, m_numParticles);
	
	glBindVertexArray(0);
}

void ParticleMesh::shutdown()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1, &m_particleBuffer);

    delete[] m_particles[1];
    delete[] m_particles[0];
}
