// particleSystemEvo.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystemEvo.h"
#include <omp.h>
using namespace std;

ParticleSystemEvo::ParticleSystemEvo(int numParticles, string shaderPath)
{
    frame_index = 0;
    use_omp = false;
    this->m_numParticles = numParticles;
    this->m_shaderPath = shaderPath;
}

ParticleSystemEvo::~ParticleSystemEvo()
{
    shutdown();
}

#define BINDING 0
#define LOC_POSITION 0
#define LOC_ID 1

bool ParticleSystemEvo::startup()
{
    // Application memory particle buffers (double buffered)
    particles[0] = new PARTICLE[m_numParticles];
    particles[1] = new PARTICLE[m_numParticles];

    // Create GPU buffer
    glGenBuffers(1, &particle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
    //glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(PARTICLE), nullptr, )
    glBufferStorage(GL_ARRAY_BUFFER, m_numParticles * sizeof(PARTICLE), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    mapped_buffer = (PARTICLE*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_numParticles * sizeof(PARTICLE), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    initialize_particles();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    glBindVertexBuffer(BINDING, particle_buffer, 0, sizeof(PARTICLE));
    
    glEnableVertexAttribArray(LOC_POSITION);
    glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(PARTICLE, position));
    glVertexAttribBinding(LOC_POSITION, BINDING);

    glEnableVertexAttribArray(LOC_ID); 
    glVertexAttribIFormat(LOC_ID, 1, GL_INT, offsetof(PARTICLE, ID));
    glVertexAttribBinding(LOC_ID, BINDING);

    // Upload shader
    m_shader = DEMO->shaderManager.addShader(m_shaderPath + "/particleSystem.vert", m_shaderPath + "/particleSystem.frag");
    if (m_shader < 0)
        return false;

    int maxThreads = omp_get_max_threads();
    omp_set_num_threads(maxThreads);

    return true;
}

// Random number generator
static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

void ParticleSystemEvo::initialize_particles(void)
{
    int i;

    for (i = 0; i < m_numParticles; i++)
    {
        particles[0][i].position.x = random_float() * 1.0f - 0.5f;
        particles[0][i].position.y = random_float() * 1.0f - 0.5f;
        particles[0][i].position.z = random_float() * 1.0f - 0.5f;
        particles[0][i].velocity = particles[0][i].position * 0.001f;
        particles[0][i].ID = i;

        mapped_buffer[i] = particles[0][i];
    }
}


static float m_time = 0;
void ParticleSystemEvo::update_particles(float deltaTime)
{
    m_time += deltaTime;
    // Double buffer source and destination
    const PARTICLE* const __restrict src = particles[frame_index & 1];
    PARTICLE* const __restrict dst = particles[(frame_index + 1) & 1];

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
    frame_index++;
}

void ParticleSystemEvo::update_particles_omp(float deltaTime)
{
    // Double buffer source and destination
    const PARTICLE* const __restrict src = particles[frame_index & 1];
    PARTICLE* const __restrict dst = particles[(frame_index + 1) & 1];

    // For each particle in the system
#pragma omp parallel for schedule (dynamic, 16)
    for (int i = 0; i < m_numParticles; i++)
    {
        // Get my own data
        const PARTICLE& me = src[i];
        glm::vec3 delta_v(0.0f);

        // For all the other particles
        for (int j = 0; j < m_numParticles; j++)
        {
            if (i != j) // ... not me!
            {
                //  Get the vector to the other particle
                glm::vec3 delta_pos = src[j].position - me.position;
                float distance = glm::length(delta_pos);
                // Normalize
                glm::vec3 delta_dir = delta_pos / distance;
                // This clamp stops the system from blowing up if particles get
                // too close...
                distance = distance < 0.005f ? 0.005f : distance;
                // Update velocity
                delta_v += (delta_dir / (distance * distance));
            }
        }
        // Add my current velocity to my position.
        dst[i].position = me.position + me.velocity;
        // Produce new velocity from my current velocity plus the calculated delta
        dst[i].velocity = me.velocity + delta_v * deltaTime * 0.01f;
        // Write to mapped buffer
        mapped_buffer[i].position = dst[i].position;
    }

    // Count frames so we can double buffer next frame
    frame_index++;
}

void ParticleSystemEvo::render(float currentTime, const glm::mat4& PVM)
{
    static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static float previousTime = 0.0;

    // Calculate delta time
    float deltaTime = (currentTime - previousTime);
    previousTime = currentTime;

    // Update particle positions using OpenMP... or not.
    if (use_omp)
    {
        update_particles_omp(deltaTime * 0.001f);
    }
    else
    {
        //update_particles(deltaTime);
    }

    // Bind our vertex arrays
    glBindVertexArray(vao);

    // Let OpenGL know we've changed the contents of the buffer
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, m_numParticles * sizeof(PARTICLE));

    glPointSize(3.0f);

    // Draw!
    Shader* particleSystem_shader = DEMO->shaderManager.shader[m_shader];
    particleSystem_shader->use();
    particleSystem_shader->setValue("gTime", currentTime);	// Send the Time
    particleSystem_shader->setValue("gPVM", PVM);	// Set (Projection x View x Model) matrix
    particleSystem_shader->setValue("gNumParticles", (float)m_numParticles);   // Set the total number of particles

    glDrawArrays(GL_POINTS, 0, m_numParticles);
}


void ParticleSystemEvo::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
        case 'M':
            use_omp = !use_omp;
            break;
        }
    }
}

void ParticleSystemEvo::shutdown()
{
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1, &particle_buffer);

    delete[] particles[1];
    delete[] particles[0];
}

