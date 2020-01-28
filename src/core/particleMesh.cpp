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

#define BINDING			0
#define LOC_POSITION	0

bool ParticleMesh::startup(vector<glm::vec3> Pos)
{
    // Application memory particle buffers (double buffered)
    m_particles = new PARTICLE[m_numParticles];

    // Load the ID's of the particles
    initialize_particles(Pos);

    // Create GPU buffer in read-only mode (faster)
    glGenBuffers(1, &m_particleBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer);
    glBufferStorage(GL_ARRAY_BUFFER, m_numParticles * sizeof(PARTICLE), m_particles, GL_MAP_READ_BIT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindVertexBuffer(BINDING, m_particleBuffer, 0, sizeof(PARTICLE));
    
	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(PARTICLE, Pos));
	glVertexAttribBinding(LOC_POSITION, BINDING);

    return true;
}

void ParticleMesh::initialize_particles(vector<glm::vec3> Pos)
{
	if (Pos.empty()) {
		for (int i = 0; i < m_numParticles; i++)
		{
			m_particles[i].Pos = glm::vec3(0.0f);
		}

	}
	else {
		if (Pos.size() != m_numParticles)
			LOG->Error("ParticleMesh: The number of positions does not match the number of particles!");
		else {
			for (int i = 0; i < m_numParticles; i++)
			{
				m_particles[i].Pos = Pos[i];
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

