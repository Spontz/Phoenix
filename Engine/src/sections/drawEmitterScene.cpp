#include "main.h"

#include "core/drivers/mathdriver.h"
#include "core/renderer/ParticleSystem.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sDrawEmitterScene : public Section {
	public:
		sDrawEmitterScene();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Model* m_pModel = nullptr;

		// Particle engine variables
		unsigned int	m_uiNumMaxParticles = 0;
		unsigned int	m_uiNumEmitters = 0;
		float			m_fCurrentEmitter = 0;
		float			m_fEmissionTime = 0;
		float			m_fParticleLifeTime = 0;
		float			m_fParticleSpeed = 0;
		float			m_fEmitterRandomness = 0;
		ParticleSystem* m_pPartSystem = nullptr;

		// Particles positioning (for all the model)
		glm::vec3		m_vTranslation = { 0, 0, 0 };
		glm::vec3		m_vRotation = { 0, 0, 0 };
		glm::vec3		m_vScale = { 1, 1, 1 };

		glm::vec3		m_vVelocity = { 0, 0, 0 };
		glm::vec3		m_vForce = { 0, 0, 0 };
		glm::vec3		m_vColor = { 0, 0, 0 };
		MathDriver* m_pExprPosition = nullptr;	// A equation containing the calculations to position the object

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

	static float RandomFloat()
	{
		float Max = RAND_MAX;
		return ((float)rand() / Max);
	}

	bool sDrawEmitterScene::load()
	{
		// script validation
		if ((param.size() != 3) || (strings.size() < 9)) {
			Logger::error("Draw Emitter Scene [%s]: 3 param (emission time, Particle Life Time & Randomness) and 9 strings needed (shader path, model, 3 for positioning, part speed, velocity, force and color)", identifier.c_str());
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

		// Load Emitters and Particles config
		m_fEmissionTime = param[0];
		m_fParticleLifeTime = param[1];
		m_fEmitterRandomness = param[2];

		if (m_fEmissionTime <= 0) {
			Logger::error("Draw Emitter Scene [%s]: Emission time should be greater than 0", identifier.c_str());
			return false;
		}

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

		m_pExprPosition->SymbolTable.add_variable("partSpeed", m_fParticleSpeed);
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
			m_uiNumEmitters += static_cast<uint32_t>(mesh.unique_vertices_pos.size());
		}

		if (m_uiNumEmitters <= 0) {
			Logger::error("Draw Emitter Scene [%s]: No emitters found in the 3D model", identifier.c_str());
			return false;
		}
		m_pExprPosition->SymbolTable.add_constant("TnE", static_cast<float>(m_uiNumEmitters));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		m_uiNumMaxParticles = m_uiNumEmitters + static_cast<unsigned int>(static_cast<float>(m_uiNumEmitters) * m_fParticleLifeTime * (1.0f / m_fEmissionTime));
		Logger::info(LogLevel::low, "Draw Emitter Scene [%s]: Num max of particles will be: %d", identifier.c_str(), m_uiNumMaxParticles);

		std::vector<Particle> Emitter;
		Emitter.resize(m_uiNumEmitters);

		// Load the emitters, based in our model vertexes
		int numEmitter = 0;
		m_fCurrentEmitter = 0;
		for (int i = 0; i < m_pModel->meshes.size(); i++) {
			for (int j = 0; j < m_pModel->meshes[i].unique_vertices_pos.size(); j++) {
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Emitter[numEmitter].Type = ParticleType::Emitter;
				Emitter[numEmitter].Pos = m_pModel->meshes[i].unique_vertices_pos[j];
				Emitter[numEmitter].Vel = m_vVelocity + (m_fEmitterRandomness * glm::vec3(RandomFloat(), RandomFloat(), RandomFloat()));
				Emitter[numEmitter].Col = m_vColor;
				Emitter[numEmitter].lifeTime = 0.0f;
				numEmitter++;
				m_fCurrentEmitter = static_cast<float>(numEmitter);
			}
		}

		// Create the particle system
		m_pPartSystem = new ParticleSystem(pathShaders, m_uiNumMaxParticles, m_uiNumEmitters, m_fEmissionTime, m_fParticleLifeTime);
		if (!m_pPartSystem->InitParticleSystem(this, Emitter, uniform))
			return false;

		return true;
	}

	void sDrawEmitterScene::init()
	{
	}


	static float lastTime = 0;

	void sDrawEmitterScene::exec()
	{
		// Start evaluating blending
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		glDepthMask(GL_FALSE); // Disable depth buffer writting

		glm::mat4 projection = m_demo.m_pActiveCamera->getProjection();
		glm::mat4 view = m_demo.m_pActiveCamera->getView();

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_vTranslation);
		model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, m_vScale);

		glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix

		// Render particles
		float deltaTime = runTime - lastTime;
		deltaTime = deltaTime * m_fParticleSpeed;
		lastTime = runTime;
		if (deltaTime < 0) {
			deltaTime = -deltaTime;	// In case we rewind the demo
		}
		m_pPartSystem->force = m_vForce;

		m_pPartSystem->Render(deltaTime, vp, model, m_demo.m_pActiveCamera->getPosition());


		// End evaluating blending
		glDepthMask(GL_TRUE); // Enable depth buffer writting
		EvalBlendingEnd();

	}

	void sDrawEmitterScene::end()
	{
	}

	void sDrawEmitterScene::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Model used: " << m_pModel->filename << std::endl;
		ss << "Emitters: " << m_uiNumEmitters << std::endl;
		ss << "Max Particles: " << m_uiNumMaxParticles << std::endl;
		ss << "Emission Time: " << m_fEmissionTime << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Emitter Randomness: " << m_fEmitterRandomness << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawEmitterScene::debug() {
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}