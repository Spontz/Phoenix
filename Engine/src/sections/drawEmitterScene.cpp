#include "main.h"

#include "core/drivers/mathdriver.h"
#include "core/renderer/ParticleSystem.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawEmitterScene final : public Section {
	public:
		sDrawEmitterScene();
		~sDrawEmitterScene();

		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		SP_Model		m_pModel;

		// Particle engine variables
		float			m_lastTime = 0;
		unsigned int	m_uiNumEmitters = 0;
		float			m_fCurrentEmitter = 0;
		float			m_fEmissionTime = 0;
		float			m_fParticleLifeTime = 0;
		float			m_fParticleSpeed = 0;
		float			m_fParticleRandomness = 0;
		ParticleSystem* m_pPartSystem = nullptr;

		// Particles positioning (for all the model)
		glm::vec3		m_vTranslation = { 0, 0, 0 };
		glm::vec3		m_vRotation = { 0, 0, 0 };
		glm::vec3		m_vScale = { 1, 1, 1 };

		glm::vec3		m_vVelocity = { 0, 0, 0 };
		glm::vec3		m_vForce = { 0, 0, 0 };
		glm::vec3		m_vColor = { 0, 0, 0 };
		MathDriver*		m_pExprPosition = nullptr;	// An equation containing the calculations to position the object

	};

	// ******************************************************************

	Section* instance_drawEmitterScene()
	{
		return new sDrawEmitterScene();
	}

	sDrawEmitterScene::sDrawEmitterScene()
	{
		type = SectionType::DrawEmitterScene;
	}

	sDrawEmitterScene::~sDrawEmitterScene()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pPartSystem)
			delete m_pPartSystem;
	}

	static float RandomFloat()
	{
		float Max = RAND_MAX;
		return ((float)rand() / Max);
	}

	bool sDrawEmitterScene::load()
	{
		// script validation
		if ((param.size() != 2) || (strings.size() < 9)) {
			Logger::error("Draw Emitter Scene [{}]: 3 param (Emission time & Particle Life Time) and 9 strings needed (shader path, model, 3 for positioning, part speed, velocity, force and color)", identifier);
			return false;
		}

		// Load the shaders
		std::string pathShaders;
		pathShaders = m_demo.m_dataFolder + strings[0];

		// Load the model
		m_pModel = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);

		if (!m_pModel)
			return false;

		// Load unique vertices (it can take a while)
		m_pModel->loadUniqueVertices();

		// Render states
		render_disableDepthMask = true;

		// Load Emitters and Particles config
		m_fEmissionTime = param[0];
		m_fParticleLifeTime = param[1];

		if (m_fEmissionTime <= 0) {
			Logger::error("Draw Emitter Scene [{}]: Emission time should be greater than 0", identifier);
			return false;
		}

		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (size_t i = 2; i < strings.size(); i++)
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

		m_pExprPosition->SymbolTable.add_variable("partSpeed", m_fParticleSpeed);

		m_pExprPosition->SymbolTable.add_variable("partRandomness", m_fParticleRandomness);

		m_pExprPosition->SymbolTable.add_variable("velX", m_vVelocity.x);
		m_pExprPosition->SymbolTable.add_variable("velY", m_vVelocity.y);
		m_pExprPosition->SymbolTable.add_variable("velZ", m_vVelocity.z);

		m_pExprPosition->SymbolTable.add_variable("forceX", m_vForce.x);
		m_pExprPosition->SymbolTable.add_variable("forceY", m_vForce.y);
		m_pExprPosition->SymbolTable.add_variable("forceZ", m_vForce.z);

		m_pExprPosition->SymbolTable.add_variable("colorR", m_vColor.r);
		m_pExprPosition->SymbolTable.add_variable("colorG", m_vColor.g);
		m_pExprPosition->SymbolTable.add_variable("colorB", m_vColor.b);

		m_pExprPosition->SymbolTable.add_variable("nE", m_fCurrentEmitter);

		// Read the number of emmitters
		for (auto& mesh : m_pModel->meshes) {
			m_uiNumEmitters += static_cast<uint32_t>(mesh->m_uniqueVertices.size());
		}

		if (m_uiNumEmitters <= 0) {
			Logger::error("Draw Emitter Scene [{}]: No emitters found in the 3D model", identifier);
			return false;
		}
		m_pExprPosition->SymbolTable.add_constant("TnE", static_cast<float>(m_uiNumEmitters));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		
		std::vector<ParticleSystem::Particle> Emitter;
		Emitter.resize(m_uiNumEmitters);

		// Load the emitters, based in our model vertexes
		size_t numEmitter = 0;
		m_fCurrentEmitter = 0;
		for (auto& mesh : m_pModel->meshes) {
			for (auto& uniqueVertex : mesh->m_uniqueVertices) {
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Emitter[numEmitter].Type = ParticleSystem::ParticleType::Emitter;
				Emitter[numEmitter].Pos = uniqueVertex.Position;
				Emitter[numEmitter].Vel = uniqueVertex.Normal + (m_fParticleRandomness * glm::vec3(RandomFloat(), RandomFloat(), RandomFloat()));
				Emitter[numEmitter].Col = m_vColor;
				Emitter[numEmitter].lifeTime = 0.0f;
				numEmitter++;
				m_fCurrentEmitter = static_cast<float>(numEmitter);
			}
		}

		// Create the particle system
		m_pPartSystem = new ParticleSystem(pathShaders);
		if (!m_pPartSystem->Init(this, Emitter, m_fEmissionTime, m_fParticleLifeTime, uniform))
			return false;

		Emitter.clear(); // Delete emitters... it's worth it? or can be used for updating emitter values?

		return !DEMO_checkGLError();
	}

	void sDrawEmitterScene::init()
	{
	}

	void sDrawEmitterScene::exec()
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

		// Render particles
		float deltaTime = runTime - m_lastTime;
		deltaTime = deltaTime * m_fParticleSpeed;
		m_lastTime = runTime;
		if (deltaTime < 0) {
			deltaTime = -deltaTime;	// In case we rewind the demo
		}
		// Update Force and Color values
		m_pPartSystem->force = m_vForce;
		m_pPartSystem->color = m_vColor;
		m_pPartSystem->randomness = m_fParticleRandomness;

		m_pPartSystem->Render(deltaTime, model, view, projection);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawEmitterScene::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Model used: " << m_pModel->filename << std::endl;
		ss << "Emission Time: " << m_fEmissionTime << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Emitters: " << m_uiNumEmitters << std::endl;
		ss << "Particles per Emitter: " << m_pPartSystem->getNumParticlesPerEmitter() << std::endl;
		ss << "Max Particles: " << m_pPartSystem->getNumMaxParticles() << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pPartSystem->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawEmitterScene::debug() {
		std::stringstream ss;
		ss << debugStatic;
		ss << "Particle Randomness: " << std::format("{:.2f}", m_fParticleRandomness) << std::endl;
		ss << "Generated Particles: " << m_pPartSystem->getNumGenParticles() << std::endl;
		return ss.str();
	}
}