#include "main.h"

#include "core/drivers/mathdriver.h"
#include "core/renderer/ParticleSystemEx.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawEmitterSceneEx final : public Section {
	public:
		sDrawEmitterSceneEx();
		~sDrawEmitterSceneEx();

		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		SP_Model		m_pModel;

		// Particle engine variables
		float			m_lastTime = 0;
		unsigned int	m_uiNumMaxParticles = 0;
		unsigned int	m_uiNumEmitters = 0;
		float			m_fCurrentEmitter = 0;
		int				m_iParticlesPerEmitter = 0;
		float			m_fParticleLifeTime = 0;
		float			m_fParticleSpeed = 0;
		float			m_fParticleRandomness = 0;
		ParticleSystemEx* m_pPartSystem = nullptr;

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

	Section* instance_drawEmitterSceneEx()
	{
		return new sDrawEmitterSceneEx();
	}

	sDrawEmitterSceneEx::sDrawEmitterSceneEx()
	{
		type = SectionType::DrawEmitterScene;
	}

	sDrawEmitterSceneEx::~sDrawEmitterSceneEx()
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

	bool sDrawEmitterSceneEx::load()
	{
		// script validation
		if ((param.size() != 2) || (strings.size() < 9)) {
			Logger::error("Draw Emitter Scene EX [{}]: 2 param (Particles per Emitter & Particle Life Time) and 9 strings needed (shader path, scene, 3 for positioning, partSpeed, partRandomness, force and color)", identifier);
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
		render_disableDepthTest = true;

		// Load Emitters and Particles config
		m_iParticlesPerEmitter = static_cast<int>(param[0]);
		m_fParticleLifeTime = param[1];

		if (m_iParticlesPerEmitter < 0) {
			Logger::error("Draw Emitter Scene Ex [{}]: Particles per Emitter should be 0 or greater", identifier);
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

		m_pExprPosition->SymbolTable.add_variable("forceX", m_vForce.x);
		m_pExprPosition->SymbolTable.add_variable("forceY", m_vForce.y);
		m_pExprPosition->SymbolTable.add_variable("forceZ", m_vForce.z);

		m_pExprPosition->SymbolTable.add_variable("colorR", m_vColor.r);
		m_pExprPosition->SymbolTable.add_variable("colorG", m_vColor.g);
		m_pExprPosition->SymbolTable.add_variable("colorB", m_vColor.b);

		m_pExprPosition->SymbolTable.add_variable("nE", m_fCurrentEmitter);

		// Read the number of emmitters
		for (auto& mesh : m_pModel->meshes) {
			m_uiNumEmitters += static_cast<uint32_t>(mesh->unique_vertices_pos.size());
		}

		if (m_uiNumEmitters <= 0) {
			Logger::error("Draw Emitter Scene EX [{}]: No emitters found in the 3D model", identifier);
			return false;
		}
		m_pExprPosition->SymbolTable.add_constant("TnE", static_cast<float>(m_uiNumEmitters));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		m_uiNumMaxParticles = m_uiNumEmitters + static_cast<unsigned int>(m_uiNumEmitters * m_iParticlesPerEmitter);
		Logger::info(LogLevel::low, "Draw Emitter Scene EX [{}]: Num max of particles will be: {}", identifier, m_uiNumMaxParticles);

		std::vector<ParticleEx> Particles;
		Particles.resize(m_uiNumMaxParticles);

		// Load the emitters and particle valies, based in our model vertexes
		size_t numEmitter = 0;	// Number of Emitter
		size_t emitterID = 0;	// Emitter number (inside the array)
		size_t numParticle = 0;

		m_fCurrentEmitter = 0;
		for (size_t i = 0; i < m_pModel->meshes.size(); i++) {
			for (size_t j = 0; j < m_pModel->meshes[i]->unique_vertices_pos.size(); j++) {
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Particles[numParticle].Type = ParticleType::Emitter;
				Particles[numParticle].RandomID = (int32_t)numParticle;
				Particles[numParticle].InitPosition = m_pModel->meshes[i]->unique_vertices_pos[j];
				Particles[numParticle].Position = glm::vec3(0, 0, 0);
				Particles[numParticle].Randomness = glm::vec3(0, 0, 0);
				Particles[numParticle].Rotation = glm::vec3(0, 0, 0);
				Particles[numParticle].Color = glm::vec3(0, 0, 0);
				Particles[numParticle].Age = 0;
				Particles[numParticle].Life = 0;
				emitterID = numParticle;
				numParticle++;
				// Fill the particles per emitter
				for (size_t k = 0; k < m_iParticlesPerEmitter; k++) {
					m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
					Particles[numParticle].Type = ParticleType::Shell;
					Particles[numParticle].RandomID = (int32_t)numParticle;
					Particles[numParticle].InitPosition = Particles[emitterID].InitPosition;	// Load the position of the emitter as Initial position
					Particles[numParticle].Position = Particles[emitterID].InitPosition;	// Load the position of the emitter as Initial position
					Particles[numParticle].Randomness = m_fParticleRandomness * glm::vec3(RandomFloat(), RandomFloat(), RandomFloat());
					Particles[numParticle].Rotation = glm::vec3(0,0,0);
					Particles[numParticle].Color = Particles[emitterID].Color;	// Inherit the color of the emitter
					Particles[numParticle].Age = ((float)k/ (float)(m_iParticlesPerEmitter-1)) * (m_fParticleLifeTime);
					Particles[numParticle].Life = m_fParticleLifeTime;
					numParticle++;
				}
				numEmitter++;
				m_fCurrentEmitter = static_cast<float>(numEmitter);
			}
		}
		
		// Create the particle system
		m_pPartSystem = new ParticleSystemEx(pathShaders, m_uiNumMaxParticles, m_uiNumEmitters, m_fParticleLifeTime, m_fParticleRandomness);
		if (!m_pPartSystem->InitParticleSystem(this, Particles, uniform))
			return false;

		return !GLDRV_checkError();
	}

	void sDrawEmitterSceneEx::init()
	{
	}

	void sDrawEmitterSceneEx::exec()
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

		// Render particles
		float deltaTime = runTime - m_lastTime;
		deltaTime = deltaTime * m_fParticleSpeed;
		m_lastTime = runTime;
		if (deltaTime < 0) {
			deltaTime = -deltaTime;	// In case we rewind the demo
		}
		// Update particle system, public values
		m_pPartSystem->force = m_vForce;
		m_pPartSystem->color = m_vColor;
		m_pPartSystem->randomness = m_fParticleRandomness;

		m_pPartSystem->Render(deltaTime, model, view, projection);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawEmitterSceneEx::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Model used: " << m_pModel->filename << std::endl;
		ss << "Emitters: " << m_uiNumEmitters << std::endl;
		ss << "Max Particles: " << m_uiNumMaxParticles << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pPartSystem->getMemUsedInMb()) << " Mb" << std::endl;
		ss << "Particles per Emitter: " << m_iParticlesPerEmitter << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawEmitterSceneEx::debug() {
		std::stringstream ss;
		ss << debugStatic;
		ss << "Particle Randomness: " << std::format("{:.2f}", m_fParticleRandomness) << std::endl;
		ss << "Generated Particles: " << m_pPartSystem->getNumParticles() << std::endl;
		return ss.str();
	}
}