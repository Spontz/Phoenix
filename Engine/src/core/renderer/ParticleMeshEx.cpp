// ParticleMeshEx.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ParticleMeshEx.h"

namespace Phoenix {

	ParticleMeshEx::ParticleMeshEx() :
		m_numParticles(0),
		m_memUsed(0),
		m_particles(nullptr),
		m_VertexArray(nullptr)
	{

	}

	ParticleMeshEx::~ParticleMeshEx()
	{
		shutdown();
	}

	bool ParticleMeshEx::init(std::vector<Particle> const& Part)
	{
		m_numParticles = static_cast<uint32_t>(Part.size());

		if (m_numParticles == 0)
			return false;
		
		m_memUsed = (float)(sizeof(Particle) * m_numParticles) / 1024.0f / 1024.0f;

		// Allocte mem for particles
		m_particles = new Particle[m_numParticles];
		for (uint32_t i = 0; i < m_numParticles; i++)
			m_particles[i] = Part[i];


		// Allocate Vertex Array
		m_VertexArray = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto pVB = std::make_shared<VertexBuffer>(&m_particles[0], m_numParticles * static_cast<uint32_t>(sizeof(Particle)));
		pVB->SetLayout({
			{ ShaderDataType::Int,		"Type"},
			{ ShaderDataType::Int,		"ID"},
			{ ShaderDataType::Float3,	"InitPosition"},
			{ ShaderDataType::Float3,	"Randomness"},
			{ ShaderDataType::Float4,	"InitColor"},
			{ ShaderDataType::Float,	"Life"}
			});

		m_VertexArray->AddVertexBuffer(pVB);
		m_VertexArray->unbind();

		return true;
	}

	void ParticleMeshEx::render()
	{
		// Bind our vertex arrays
		m_VertexArray->bind();
		// Draw! (shader needs to be called in advance!)
		glDrawArrays(GL_POINTS, 0, m_numParticles);
		m_VertexArray->unbind();
	}

	void ParticleMeshEx::shutdown()
	{
		m_numParticles = 0;
		m_memUsed = 0;

		if (m_particles)
			delete[] m_particles;

		m_particles = nullptr;
		m_VertexArray = nullptr;
	}
}