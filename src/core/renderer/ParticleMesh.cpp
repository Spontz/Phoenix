// ParticleMesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleMesh.h"


ParticleMesh::ParticleMesh(int numParticles):
    m_numParticles(numParticles),
    m_particles (nullptr),
    m_particleBuffer (0),
    m_vao(0)
{
    
}

ParticleMesh::~ParticleMesh()
{
    shutdown();
}

#define BINDING			0
#define LOC_POSITION	0
#define LOC_COLOR   	1

bool ParticleMesh::startup(std::vector<PARTICLE> Part)
{
    // Application memory particle buffers (double buffered)
    m_particles = new PARTICLE[m_numParticles];

    // Load the ID's of the particles
    initialize_particles(Part);

    // Create GPU buffer in read-only mode (faster)
    glCreateBuffers(1, &m_particleBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
    glBufferStorage(GL_ARRAY_BUFFER, m_numParticles * sizeof(PARTICLE), m_particles, GL_MAP_READ_BIT);

    glCreateVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindVertexBuffer(BINDING, m_particleBuffer, 0, sizeof(PARTICLE));

    // Position location
	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(PARTICLE, Pos));
	glVertexAttribBinding(LOC_POSITION, BINDING);

    // Color location
    glEnableVertexAttribArray(LOC_COLOR);
    glVertexAttribFormat(LOC_COLOR, 4, GL_FLOAT, GL_FALSE, offsetof(PARTICLE, Col));
    glVertexAttribBinding(LOC_COLOR, BINDING);

    glBindVertexArray(0);
    return true;
}

void ParticleMesh::initialize_particles(std::vector<PARTICLE> Part)
{
	if (Part.empty()) {
		for (int i = 0; i < m_numParticles; i++)
		{
			m_particles[i].Pos = glm::vec3(0.0f);
            m_particles[i].Col = glm::vec4(0.0f);
		}
	}
	else {
		if (Part.size() != m_numParticles)
			Logger::error("ParticleMesh: The number of positions does not match the number of particles!");
		else {
			for (int i = 0; i < m_numParticles; i++)
			{
                m_particles[i] = Part[i];
			}
		}
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

