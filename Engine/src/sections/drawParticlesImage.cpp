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
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// Image
		SP_Texture m_pTexture;

		// Particle engine variables
		int				m_iNumParticles = 0;
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
		if (strings.size() != 5) {
			Logger::error("Draw Particles Image [{}]: 5 strings needed (1 for shader file, 1 for image, 3 for positioning)", identifier);
			return false;
		}

		// Set States
		render_disableDepthMask = true;

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
		m_iNumParticles = m_pTexture->width * m_pTexture->height;

		if (m_iNumParticles == 0) {
			Logger::error("Draw Particles Image [{}]: Image size is zero, no particles to draw", identifier);
			return false;
		}
		// Load the particles position and color
		std::vector<ParticleMesh::Particle> Part;
		Part.resize(m_iNumParticles);
		int cnt = 0;
		for (int i = 0; i < m_pTexture->width; i++) {
			for (int j = 0; j < m_pTexture->height; j++) {
				Part[cnt].Pos = glm::vec3(i, j, 0);
				Part[cnt].Col = m_pTexture->getColor(i, j);//glm::vec3(1, 0, 0); // Todo: Put here original image color
				cnt++;
			}
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

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Create the particle system
		m_pParticleMesh = new ParticleMesh(m_iNumParticles);
		if (!m_pParticleMesh->startup(Part))
			return false;
		// Delete all temporarly elements
		Part.clear();

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

	void sDrawParticlesImage::init()
	{
	}


	static float lastTime = 0;

	void sDrawParticlesImage::exec()
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
		m_pShader->setValue("gTime", runTime);	// Send the Time
		m_pShader->setValue("gVP", projection * view);	// Set Projection x View matrix
		m_pShader->setValue("gModel", model);			// Set Model matrix
		m_pShader->setValue("gCameraPos", m_demo.m_pActiveCamera->getPosition());		// Set camera position
		m_pShader->setValue("gNumParticles", (float)m_iNumParticles);	// Set the total number of particles

		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render(runTime);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticlesImage::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "File: " << m_pTexture->filename << std::endl;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticlesImage::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}