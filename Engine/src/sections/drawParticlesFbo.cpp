#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawParticlesFbo final : public Section {
	public:
		sDrawParticlesFbo();
		~sDrawParticlesFbo();

	public:
		void		freeMem();
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// Fbo to draw
		Fbo*			m_pFbo = nullptr;
		uint32_t		m_uFboNum = 0;
		uint32_t		m_uFboAttachment = 0;
		int32_t			m_iFboTexUnitID = 0;

		// Fbo drawing properties
		bool			m_bFitToContent = false;	// Fit to content: true:respect image aspect ratio, false:stretch to viewport/quad

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

	Section* instance_drawParticlesFbo()
	{
		return new sDrawParticlesFbo();
	}

	sDrawParticlesFbo::sDrawParticlesFbo()
	{
		type = SectionType::DrawParticlesFbo;
	}

	sDrawParticlesFbo::~sDrawParticlesFbo()
	{
		freeMem();
	}

	void sDrawParticlesFbo::freeMem()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
		if (m_pParticleMesh)
			delete m_pParticleMesh;
	}

	bool sDrawParticlesFbo::load()
	{
		// We free memory, because in some cases we will need to reload the section
		freeMem();

		// script validation
		if ((param.size() != 5) || (strings.size() != 4)) {
			Logger::error("Draw Particles Fbo [{}]: 5 param (Fbo number and attachment, Fit to content, particles per Emitter & particle Life Time) + 4 strings needed (shader file and 3 for positioning)", identifier);
			return false;
		}

		// Set States
		render_disableDepthMask = true;

		// Load Fbo number
		m_uFboNum = static_cast<uint32_t>(param[0]);
		m_uFboAttachment = static_cast<uint32_t>(param[1]);

		// Load image properties
		m_bFitToContent = static_cast<bool>(param[2]);

		// Load Emitters and Particles config
		m_iParticlesPerEmitter = static_cast<int>(param[3]);
		m_fParticleLifeTime = param[4];

		// Section checks
		if (m_iParticlesPerEmitter < 0) {
			Logger::error("Draw Particles Fbo [{}]: Particles per emitter should be greater than 0", identifier);
			return false;
		}

		// Load the shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_pShader)
			return false;

		// Check for the right parameter values
		if (m_uFboNum >= FBO_BUFFERS) {
			Logger::error("Draw Particles Fbo [{}]: Invalid fbo number: {}", identifier, m_uFboNum);
			return false;
		}

		m_pFbo = m_demo.m_fboManager.fbo[m_uFboNum];

		if (m_uFboAttachment >= static_cast<uint32_t>(m_pFbo->numAttachments)) {
			Logger::error("Draw Particles Fbo [{}]: Invalid fbo attachment: {}", identifier, m_uFboAttachment);
			return false;
		}

		// Calculate particles number
		m_iNumEmitters = m_pFbo->width * m_pFbo->height;
		m_iNumParticles = m_iNumEmitters + m_iNumEmitters * m_iParticlesPerEmitter;

		if (m_iNumParticles == 0) {
			Logger::error("Draw Particles Fbo [{}]: Image size is zero, no particles to draw", identifier);
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

		m_pExprPosition->SymbolTable.add_variable("nE", m_fCurrentEmitter);

		// Add constants
		m_pExprPosition->SymbolTable.add_constant("fboWidth", static_cast<float>(m_pFbo->width));
		m_pExprPosition->SymbolTable.add_constant("fboHeight", static_cast<float>(m_pFbo->height));
		m_pExprPosition->SymbolTable.add_constant("particlesNumber", static_cast<float>(m_iNumParticles));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Load the emitters and particle values, based in our source image
		std::vector<ParticleMesh::Particle> Particles;
		Particles.resize(m_iNumParticles);

		size_t numEmitter = 0;	// Number of Emitter
		size_t emitterID = 0;	// Emitter number (inside the array)
		size_t numParticle = 0;
		uint32_t randomSeed = 0;

		// Calculate particle position increment, based on the Image size
		float texAspectRatio = static_cast<float>(m_pFbo->width) / static_cast<float>(m_pFbo->height);

		// Calculate Scales
		float fXScale = 1;
		float fYScale = 1;
		if (m_bFitToContent) {
			if (texAspectRatio > 1.0)
				fYScale = 1 / texAspectRatio;
			else
				fXScale = texAspectRatio;
		}
		// Calculate increments
		float partPosIncrementX = 2.0f / static_cast<float>(m_pFbo->width) * fXScale;
		float partPosIncrementY = 2.0f / static_cast<float>(m_pFbo->height) * fYScale;

		// Calculate offsets
		float fXOffset = 0;
		float fYOffset = 0;
		if (m_bFitToContent) {
			if (texAspectRatio > 1.0)
				fYOffset = (2.0f - partPosIncrementY * static_cast<float>(m_pFbo->height)) / 2.0f;
			else
				fXOffset = (2.0f - partPosIncrementX * static_cast<float>(m_pFbo->width)) / 2.0f;
		}

		for (int i = 0; i < m_pFbo->width; i++) {
			for (int j = 0; j < m_pFbo->height; j++) {
				// Calculate particle position, should go from -1 to 1
				glm::vec3 ParticlePos = glm::vec3(-1 + fXOffset +  partPosIncrementX * i, -1 + fYOffset + partPosIncrementY * j, 0);
				
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Particles[numParticle].Type = ParticleMesh::ParticleType::Emitter;
				Particles[numParticle].ID = (int32_t)numEmitter;
				Particles[numParticle].InitPosition = ParticlePos;
				Particles[numParticle].Randomness = Utils::randomVec3_05(randomSeed);
				Particles[numParticle].InitColor.x = static_cast<float>(i) / static_cast<float>(m_pFbo->width); //Instead of color, we keep the fbo coordinates
				Particles[numParticle].InitColor.y = static_cast<float>(j) / static_cast<float>(m_pFbo->height);
				Particles[numParticle].InitColor.z = 0.0f;
				Particles[numParticle].InitColor.w = 0.0f;
				Particles[numParticle].Life = 0;
				emitterID = numParticle;
				numParticle++;
				// Fill the particles per emitter
				for (size_t k = 0; k < m_iParticlesPerEmitter; k++) {
					//m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
					Particles[numParticle].Type = ParticleMesh::ParticleType::Shell;
					Particles[numParticle].ID = (int32_t)k;
					Particles[numParticle].InitPosition = Particles[emitterID].InitPosition;	// Load the position of the emitter as Initial position
					Particles[numParticle].Randomness = Utils::randomVec3_05(randomSeed);
					Particles[numParticle].InitColor = Particles[emitterID].InitColor;// Inherit the color of the emitter
					Particles[numParticle].Life = m_fParticleLifeTime;
					numParticle++;
					randomSeed += static_cast<uint32_t>(numParticle) + static_cast<uint32_t>(numEmitter); // modify te seed
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

		// Validate and set shader variables
		m_pVars->validateAndSetValues();

		// Set FBO Texture unit ID, which will be the last of all the sampler2D that we have in all the shader variables
		m_iFboTexUnitID = static_cast<int32_t>(m_pVars->sampler2D.size());

		return !DEMO_checkGLError();
	}

	void sDrawParticlesFbo::init()
	{
	}

	void sDrawParticlesFbo::warmExec()
	{
		exec();
	}

	void sDrawParticlesFbo::exec()
	{
		// We add this to prevent an error if the screen is resized during execution time
		if (m_pFbo != m_demo.m_fboManager.fbo[m_uFboNum]) {
			this->load(); // Reload the section, and recalculate the particles position, color, etc... based on the new FBO created after FBO resize
			this->loadDebugStatic();
		}
		
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
		m_pShader->setValue("screenTexture", m_iFboTexUnitID);	// Send the fbo to the shader, so we can read the color value inside the shader
		m_pFbo->bind_tex(m_iFboTexUnitID, m_uFboAttachment);

		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render();

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticlesFbo::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Fbo num: " << m_uFboNum << std::endl;
		ss << "Fbo attachment: " << m_uFboAttachment << std::endl;
		ss << "Width: " << m_pFbo->width << std::endl;
		ss << "Height: " << m_pFbo->height << std::endl;
		ss << "Emitters: " << m_iNumEmitters << std::endl;
		ss << "Particles per Emitter: " << m_iParticlesPerEmitter << std::endl;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pParticleMesh->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticlesFbo::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}