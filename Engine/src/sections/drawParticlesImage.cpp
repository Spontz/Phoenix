#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawParticlesImage final : public Section {
	public:
		sDrawParticlesImage();
		~sDrawParticlesImage();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// Image
		SP_Texture m_pTexture;

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

	Section* instance_drawParticlesImage()
	{
		return new sDrawParticlesImage();
	}

	sDrawParticlesImage::sDrawParticlesImage()
	{
		type = SectionType::DrawParticlesImage;
	}

	sDrawParticlesImage::~sDrawParticlesImage()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
		if (m_pParticleMesh)
			delete m_pParticleMesh;
	}

	bool sDrawParticlesImage::load()
	{
		// script validation
		if ((param.size() != 2) || (strings.size() != 5)) {
			Logger::error("Draw Particles Image [{}]: 2 param (Particles per Emitter & Particle Life Time) + 5 strings needed (shader file, image, 3 for positioning)", identifier);
			return false;
		}

		// Set States
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

		// Load the image
		m_pTexture = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[1]);
		if (m_pTexture == nullptr)
			return false;
		if (!m_pTexture->keepData()) {
			Logger::error("Draw Particles Image [{}]: Could not allocate data image", identifier);
			return false;
		}


		// Calculate particles number
		m_iNumEmitters = m_pTexture->m_width * m_pTexture->m_height;
		m_iNumParticles = m_iNumEmitters + m_iNumEmitters * m_iParticlesPerEmitter;

		if (m_iNumParticles == 0) {
			Logger::error("Draw Particles Image [{}]: Image size is zero, no particles to draw", identifier);
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

		// Add constants
		m_pExprPosition->SymbolTable.add_constant("imageWidth", static_cast<float>(m_pTexture->m_width));
		m_pExprPosition->SymbolTable.add_constant("imageHeight", static_cast<float>(m_pTexture->m_height));
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

		for (int i = 0; i < m_pTexture->m_width; i++) {
			for (int j = 0; j < m_pTexture->m_height; j++) {
				m_pExprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
				Particles[numParticle].Type = ParticleMesh::ParticleType::Emitter;
				Particles[numParticle].ID = (int32_t)numEmitter;
				Particles[numParticle].InitPosition = glm::vec3(i, j, 0);
				Particles[numParticle].Randomness = Utils::randomVec3_05(randomSeed);
				Particles[numParticle].InitColor = m_pTexture->getColor(i, j);
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

		// Set shader variables
		m_pVars->setValues();

		return !DEMO_checkGLError();
	}

	void sDrawParticlesImage::init()
	{
	}

	void sDrawParticlesImage::warmExec()
	{
		exec();
	}

	void sDrawParticlesImage::exec()
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

		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render();

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticlesImage::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Image: " << m_pTexture->m_filename << std::endl;
		ss << "Width: " << m_pTexture->m_width << std::endl;
		ss << "Height: " << m_pTexture->m_height << std::endl;
		ss << "Emitters: " << m_iNumEmitters << std::endl;
		ss << "Particles per Emitter: " << m_iParticlesPerEmitter << std::endl;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Particle Life Time: " << m_fParticleLifeTime << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pParticleMesh->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticlesImage::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}