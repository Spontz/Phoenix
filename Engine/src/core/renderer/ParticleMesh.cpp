﻿// ParticleMesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleMesh.h"

namespace Phoenix {

	ParticleMesh::ParticleMesh(int numParticles) :
		m_numParticles(numParticles),
		m_particles(nullptr),
		m_VertexArray(nullptr)
	{

	}

	ParticleMesh::~ParticleMesh()
	{
		shutdown();
	}

	bool ParticleMesh::startup(std::vector<Particle> const& Part)
	{
		// Application memory particle buffers (double buffered)
		m_particles = new Particle[m_numParticles];

		// Load the ID's of the particles
		initialize_particles(Part);

		// Allocate Vertex Array
		m_VertexArray = new VertexArray();

		// Create & Load the Vertex Buffer
		VertexBuffer* vertexBuffer = new VertexBuffer(&m_particles[0], m_numParticles * sizeof(Particle));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3,	"aPos"},
			{ ShaderDataType::Float4,	"aColor"}
			});

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->Unbind();

		return true;
	}

	void ParticleMesh::initialize_particles(std::vector<Particle> Part)
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
		m_VertexArray->Bind();
		// Draw! (shader needs to be called in advance!)
		glDrawArrays(GL_POINTS, 0, m_numParticles);
		m_VertexArray->Unbind();
	}

	void ParticleMesh::shutdown()
	{
		delete[] m_particles;
		delete[] m_VertexArray;

		m_particles = nullptr;
		m_VertexArray = nullptr;
	}

}