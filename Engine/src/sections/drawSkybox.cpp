#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawSkybox final : public Section {
	public:
		sDrawSkybox();
		~sDrawSkybox();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		SP_Cubemap	m_pCubemap;

		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		MathDriver* m_pExprPosition = nullptr;	// A equation containing the calculations to position the object
	};

	// ******************************************************************

	Section* instance_drawSkybox()
	{
		return new sDrawSkybox();
	}

	sDrawSkybox::sDrawSkybox()
	{
		type = SectionType::DrawSkybox;
	}

	sDrawSkybox::~sDrawSkybox()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
	}

	bool sDrawSkybox::load()
	{
		if ((param.size() != 2) || (strings.size() < 8)) {
			Logger::error("DrawSkybox [{}]: 2 param and 8 strings needed: clear depth buffer, draw wireframe + 6 strings with skybox faces, 2 strings with rot and scale", identifier);
			return false;
		}

		// Check if skybox is present in resources
		if (m_demo.m_pRes->m_spSkybox == nullptr || m_demo.m_pRes->m_spShdrSkybox == nullptr) {
			Logger::error("DrawSkybox [{}]: Skybox model or shader has not been properly loaded in resources, please fix it!", identifier);
			return false;
		}

		// Load parameters
		render_clearDepth = static_cast<bool>(param[0]);
		render_drawWireframe = static_cast<bool>(param[1]);

		// Load the 6 textures of our cubemap
		std::vector<std::string> faces{ m_demo.m_dataFolder + strings[0], m_demo.m_dataFolder + strings[1], m_demo.m_dataFolder + strings[2],
											m_demo.m_dataFolder + strings[3], m_demo.m_dataFolder + strings[4], m_demo.m_dataFolder + strings[5] };

		m_pCubemap = m_demo.m_textureManager.addCubemap(faces, false);
		if (!m_pCubemap)
			return false;

		// Read variables for traslation, rotation and scaling
		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 6; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("rx", m_vRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vScale.z);
		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		return !DEMO_checkGLError();
	}

	void sDrawSkybox::init()
	{

	}

	void sDrawSkybox::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// change depth function so depth test passes when values are equal to depth buffer's content
		//glDepthFunc(GL_LEQUAL); // Not needed - this is the generic Depth function - TODO: Pending testing and delete this

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		m_demo.m_pRes->m_spShdrSkybox->use(); // TODO: Do not use the Resource shader for skybox, and use our own shader!

		// view/projection transformations
		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = glm::mat4(glm::mat3(m_demo.m_cameraManager.getActiveView())); // remove translation from the view matrix
		m_demo.m_pRes->m_spShdrSkybox->setValue("projection", projection);
		m_demo.m_pRes->m_spShdrSkybox->setValue("view", view);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, m_vScale);
		m_demo.m_pRes->m_spShdrSkybox->setValue("model", model);

		m_demo.m_pRes->m_spShdrSkybox->setValue("skybox", 0);
		m_demo.m_pRes->drawSkybox(m_pCubemap);


		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawSkybox::loadDebugStatic()
	{
		std::stringstream ss;
		for (auto& file : m_pCubemap->m_filename) {
			ss << "File: " << file << std::endl;
		}
		debugStatic = ss.str();
	}

	std::string sDrawSkybox::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}