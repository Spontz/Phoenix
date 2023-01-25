#include "main.h"

#include "core/drivers/mathdriver.h"
#include "core/renderer/ParticleSystem.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawEmitterSpline final : public Section {
	public:
		sDrawEmitterSpline();
		~sDrawEmitterSpline();

		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:

		// Spline vector
		ChanVec	SplineCurrentPos;
		float	SplinePos = 0;
		std::vector<ParticleSystem::Particle> Emitter;

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

	Section* instance_drawEmitterSpline()
	{
		return new sDrawEmitterSpline();
	}

	sDrawEmitterSpline::sDrawEmitterSpline()
	{
		type = SectionType::DrawEmitterSpline;
	}

	sDrawEmitterSpline::~sDrawEmitterSpline()
	{
		// Delete Emitters
		Emitter.clear();

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

	bool sDrawEmitterSpline::load()
	{
		// script validation
		if ((param.size() != 2) || (spline.size() < 1) || (strings.size() < 9)) {
			Logger::error("Draw Emitter Spline [{}]: 2 param (Emission time & Particle Life Time) 1 spline and 9 strings needed (2 shaders, 3 for positioning, part speed, velocity, force and color)", identifier);
			return false;
		}

		// Load the shaders
		std::string pathParticleSystemShader;
		pathParticleSystemShader = m_demo.m_dataFolder + strings[0];

		std::string pathBillboardShader;
		pathBillboardShader = m_demo.m_dataFolder + strings[1];

		// Load the spline
		for (int i = 0; i < spline.size(); i++) {
			if (spline[i]->load() == false) {
				Logger::error("Draw Emitter Spline [{}]: Spline not loaded", identifier);
				return false;
			}
		}

		// Render states
		render_disableDepthMask = true;

		// Load Emitters and Particles config
		m_fEmissionTime = param[0];
		m_fParticleLifeTime = param[1];

		if (m_fEmissionTime <= 0) {
			Logger::error("Draw Emitter Spline [{}]: Emission time should be greater than 0", identifier);
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

		// One Emitter, by now :) TBC
		m_uiNumEmitters = 1;

		if (m_uiNumEmitters <= 0) {
			Logger::error("Draw Emitter Spline [{}]: No emitters defined", identifier);
			return false;
		}
		m_pExprPosition->SymbolTable.add_constant("TnE", static_cast<float>(m_uiNumEmitters));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		

		Emitter.resize(m_uiNumEmitters);

		// Load the emitters, based in our spline
		size_t numEmitter = 0;
		m_fCurrentEmitter = 0;
	
		m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
		spline[0]->MotionCalcStep(SplineCurrentPos, 0); // Evaluate position

		// Load emitters data

		Emitter[numEmitter].Type = ParticleSystem::ParticleType::Emitter;
		Emitter[numEmitter].Pos = glm::vec3(SplineCurrentPos[0], SplineCurrentPos[1], SplineCurrentPos[2]); 
		Emitter[numEmitter].Vel = glm::vec3(0);
		Emitter[numEmitter].Col = m_vColor;
		Emitter[numEmitter].lifeTime = 0.0f;
		numEmitter++;
		m_fCurrentEmitter = static_cast<float>(numEmitter);
		
		// Create the particle system
		m_pPartSystem = new ParticleSystem(pathParticleSystemShader, pathBillboardShader);
		if (!m_pPartSystem->Init(this, Emitter, m_fEmissionTime, m_fParticleLifeTime, uniform))
			return false;

		return !DEMO_checkGLError();
	}

	void sDrawEmitterSpline::init()
	{
		spline[0]->MotionCalcStep(SplineCurrentPos, 0);
		Emitter[0].Pos.x = SplineCurrentPos[0];
		Emitter[0].Pos.y = SplineCurrentPos[1];
		Emitter[0].Pos.z = SplineCurrentPos[2];

		m_pPartSystem->UpdateEmittersPosition(Emitter);
	}

	void sDrawEmitterSpline::exec()
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

		// Update Emitters position
		SplinePos = this->runTime/this->duration;
		spline[0]->MotionCalcStep(SplineCurrentPos, runTime);
		Emitter[0].Pos.x = SplineCurrentPos[0];
		Emitter[0].Pos.y = SplineCurrentPos[1];
		Emitter[0].Pos.z = SplineCurrentPos[2];

		m_pPartSystem->UpdateEmittersPosition(Emitter);

		// Update Force and Color values
		m_pPartSystem->force = m_vForce;
		m_pPartSystem->color = m_vColor;
		m_pPartSystem->randomness = m_fParticleRandomness;

		m_pPartSystem->Render(deltaTime, model, view, projection);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawEmitterSpline::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Spline used: " << spline[0]->filename << std::endl;
		ss << "Emission Time: " << m_fEmissionTime << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Emitters: " << m_uiNumEmitters << std::endl;
		ss << "Particles per Emitter: " << m_pPartSystem->getNumParticlesPerEmitter() << std::endl;
		ss << "Max Particles: " << m_pPartSystem->getNumMaxParticles() << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pPartSystem->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawEmitterSpline::debug() {
		std::stringstream ss;
		ss << debugStatic;
		ss << "System pos: " << std::format("{:.2f},{:.2f},{:.2f}", m_vTranslation.x, m_vTranslation.y, m_vTranslation.z) << std::endl;
		ss << "Spline pos: " << std::format("{:.2f}", SplinePos) << std::endl;
		ss << "Emitter pos: " << std::format("{:.2f},{:.2f},{:.2f}", Emitter[0].Pos.x, Emitter[0].Pos.y, Emitter[0].Pos.z) << std::endl;
		ss << "Particle Randomness: " << std::format("{:.2f}", m_fParticleRandomness) << std::endl;
		ss << "Generated Particles: " << m_pPartSystem->getNumGenParticles() << std::endl;
		return ss.str();
	}
}