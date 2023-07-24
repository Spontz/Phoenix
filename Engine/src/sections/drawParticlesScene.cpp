#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawParticlesScene final : public Section {
	public:
		sDrawParticlesScene();
		~sDrawParticlesScene();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// 3D Scene
		SP_Model m_pModel;

		// Particle engine variables
		int				m_iNumParticles = 0;
		int				m_iNumEmitters = 0;
		float			m_fCurrentEmitter = 0;
		int				m_iParticlesPerEmitter = 0;
		float			m_fParticleLifeTime = 0;
		ParticleMesh*	m_pParticleMesh = nullptr;
		SP_Shader		m_pShader = nullptr;

		// Particle Matrix positioning (for all the model)
		glm::vec3		m_vTranslation = { 0, 0, 0 };
		glm::vec3		m_vRotation = { 0, 0, 0 };
		glm::vec3		m_vScale = { 1, 1, 1 };

		MathDriver*		m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*		m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawParticlesScene()
	{
		return new sDrawParticlesScene();
	}

	sDrawParticlesScene::sDrawParticlesScene()
	{
		type = SectionType::DrawParticlesScene;
	}

	sDrawParticlesScene::~sDrawParticlesScene()
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

	bool sDrawParticlesScene::load()
	{
		// script validation
		if ((param.size() != 2) || (strings.size() != 5)) {
			Logger::error("Draw Particles Scene [{}]: 2 param (Particles per Emitter & Particle Life Time) + 5 strings needed (shader file, scene, 3 for positioning)", identifier);
			return false;
		}

		// Set render states
		render_disableDepthMask = true;

		// Load Emitters and Particles config
		m_iParticlesPerEmitter = static_cast<int>(param[0]);
		m_fParticleLifeTime = param[1];

		// Section checks
		if (m_iParticlesPerEmitter < 0) {
			Logger::error("Draw Particles Scene [{}]: Particles per emitter should be greater than 0", identifier);
			return false;
		}

		// Load the shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_pShader)
			return false;

		// Load the model model
		m_pModel = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);

		if (m_pModel == nullptr)
			return false;

		// Load unique vertices (it can take a while)
		m_pModel->loadUniqueVertices();

		// Calculate emitters and total particles number
		m_iNumEmitters = 0;
		for (auto& mesh : m_pModel->meshes) {
			m_iNumEmitters += static_cast<uint32_t>(mesh->m_uniqueVertices.size());
		}
		m_iNumParticles = m_iNumEmitters + (m_iNumEmitters * m_iParticlesPerEmitter);

		if (m_iNumParticles == 0) {
			Logger::error("Draw Particles Scene [{}]: No vertex found in the model", identifier);
			return false;
		}

		// Load particle positioning
		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 2; i < strings.size(); i++)
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

		m_pExprPosition->SymbolTable.add_variable("nE", m_fCurrentEmitter);

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Load the emitters and particle values, based in our model vertexes
		std::vector<ParticleMesh::Particle> Particles;
		Particles.resize(m_iNumParticles);

		size_t numEmitter = 0;	// Number of Emitter
		size_t emitterID = 0;	// Emitter number (inside the array)
		size_t numParticle = 0;

		//size_t cnt = 0;
		// Set the seed
		srand(static_cast<unsigned int>(time(0)));

		for (size_t i = 0; i < m_pModel->meshes.size(); i++) {
			for (size_t j = 0; j < m_pModel->meshes[i]->m_uniqueVertices.size(); j++) {
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Particles[numParticle].Type = ParticleMesh::ParticleType::Emitter;
				Particles[numParticle].ID = (int32_t)numEmitter;
				Particles[numParticle].InitPosition = m_pModel->meshes[i]->m_uniqueVertices[j].Position;
				Particles[numParticle].Randomness = glm::vec3(0, 0, 0);
				Particles[numParticle].InitColor = glm::vec4(1, 0, 0, 1); //TODO: Change to use formulas
				Particles[numParticle].Life = 0;
				emitterID = numParticle;
				numParticle++;
				// Fill the particles per emitter
				for (size_t k = 0; k < m_iParticlesPerEmitter; k++) {
					m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
					Particles[numParticle].Type = ParticleMesh::ParticleType::Shell;
					Particles[numParticle].ID = (int32_t)k;
					Particles[numParticle].InitPosition = Particles[emitterID].InitPosition;	// Load the position of the emitter as Initial position
					Particles[numParticle].Randomness = RandomVec3();
					Particles[numParticle].InitColor = Particles[emitterID].InitColor;// Inherit the color of the emitter
					Particles[numParticle].Life = m_fParticleLifeTime;
					numParticle++;
				}
				numEmitter++;
				m_fCurrentEmitter = static_cast<float>(numEmitter);
			}
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

		// Set shader variables
		m_pVars->setValues();
		
		return !DEMO_checkGLError();
	}

	void sDrawParticlesScene::init()
	{
	}

	void sDrawParticlesScene::warmExec()
	{
		exec();
	}

	void sDrawParticlesScene::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = m_demo.m_cameraManager.getActiveView();

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

		m_pShader->setValue("iNumParticlesPerEmitter", m_iParticlesPerEmitter);
		
		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render();

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticlesScene::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Model used: " << m_pModel->filename << std::endl;
		ss << "Emitters: " << m_iNumEmitters << std::endl;
		ss << "Particles per Emitter: " << m_iParticlesPerEmitter << std::endl; 
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pParticleMesh->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticlesScene::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}
