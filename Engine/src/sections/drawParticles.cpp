#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawParticles final : public Section {
	public:
		sDrawParticles();
		~sDrawParticles();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// Particle engine variables
		int				m_iNumParticles = 0;
		float			m_fCurrentParticle = 0;
		ParticleMesh*	m_pParticleMesh = nullptr;
		SP_Shader		m_pShader;

		// Particle Matrix positioning (for all the model)
		glm::vec3		m_vTranslation = { 0, 0, 0 };
		glm::vec3		m_vRotation = { 0, 0, 0 };
		glm::vec3		m_vScale = { 1, 1, 1 };

		// Particles initial data
		glm::vec3		m_vInitialPosition = { 0, 0, 0 };
		glm::vec4		m_vInitialColor = { 0, 0, 0, 1 };
		float			m_fParticleLife = 0;

		MathDriver*		m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*		m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawParticles()
	{
		return new sDrawParticles();
	}

	sDrawParticles::sDrawParticles()
	{
		type = SectionType::DrawParticles;
	}

	sDrawParticles::~sDrawParticles()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
		if (m_pParticleMesh)
			delete m_pParticleMesh;
			
	}

	static glm::vec3 RandomVec3() // Return a float between -0.5 and 0.5
	{
		float Max = RAND_MAX;
		glm::vec3 randNum((float)rand(), (float)rand(), (float)rand());
		randNum /= Max;	// Values between 0 and 1
		randNum -= 0.5f;	// Values between 0.5 and -0.5

		return randNum;
	}

	bool sDrawParticles::load()
	{
		// script validation
		if ((param.size() != 1) || (strings.size() < 7)) {
			Logger::error("Draw Particles [{}]: 1 param (Particles number) and 7 strings needed (shader file, 3 for positioning, 3 for particles data)", identifier);
			return false;
		}


		// Load the shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);

		if (!m_pShader)
			return false;

		// Set States
		render_disableDepthMask = true;

		// Particles number
		m_iNumParticles = static_cast<int>(param[0]);

		if (m_iNumParticles <= 0) {
			Logger::error("Draw Particles [{}]: Particles are zero, no particles to draw", identifier);
			return false;
		}

		// Load particle positioning
		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 1; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("tx", m_vTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("ty", m_vTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("tz", m_vTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("rx", m_vRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vScale.z);

		// Add particles data
		m_pExprPosition->SymbolTable.add_constant("nPart", static_cast<float>(m_iNumParticles));
		m_pExprPosition->SymbolTable.add_variable("nP", m_fCurrentParticle);
		
		m_pExprPosition->SymbolTable.add_variable("px", m_vInitialPosition.x);
		m_pExprPosition->SymbolTable.add_variable("py", m_vInitialPosition.y);
		m_pExprPosition->SymbolTable.add_variable("pz", m_vInitialPosition.z);

		m_pExprPosition->SymbolTable.add_variable("cr", m_vInitialColor.r);
		m_pExprPosition->SymbolTable.add_variable("cg", m_vInitialColor.g);
		m_pExprPosition->SymbolTable.add_variable("cb", m_vInitialColor.b);
		m_pExprPosition->SymbolTable.add_variable("ca", m_vInitialColor.a);

		m_pExprPosition->SymbolTable.add_variable("pLife", m_fParticleLife);

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Load the emitters and particle values, based in our source image
		std::vector<ParticleMesh::Particle> Particles;
		Particles.resize(m_iNumParticles);
		m_fCurrentParticle = 0;

		for (int32_t i = 0; i < m_iNumParticles; i++) {
			m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
			Particles[i].Type = ParticleMesh::ParticleType::Emitter;
			Particles[i].ID = i;
			Particles[i].InitPosition = m_vInitialPosition;
			Particles[i].Randomness = RandomVec3();
			Particles[i].InitColor = m_vInitialColor;
			Particles[i].Life = m_fParticleLife;
			m_fCurrentParticle = static_cast<float>(i);
		}

		// Create the particle system
		m_pParticleMesh = new ParticleMesh();
		if (!m_pParticleMesh->init(Particles))
			return false;
		// Delete all temporarly elements
		Particles.clear();

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}

		// Set shader variables values
		m_pVars->setValues();

		return !GLDRV_checkError();
	}

	void sDrawParticles::init()
	{
	}


	void sDrawParticles::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		glm::mat4 projection = m_demo.m_pActiveCamera->getProjection();
		glm::mat4 view = m_demo.m_pActiveCamera->getView();

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_vTranslation);
		model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, m_vScale);


		// Get the shader
		m_pShader->use();
		m_pShader->setValue("m4ViewModel", view * model);	// Set View x Model matrix
		m_pShader->setValue("m4Projection", projection);

		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render();

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticles::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pParticleMesh->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticles::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}