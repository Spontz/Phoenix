#include "main.h"

namespace Phoenix {

	struct sLoading : public Section {
	public:
		sLoading();
		bool		load();
		void		init();
		void		exec();
		void		end();

	private:
		SP_Texture m_pTexFront;
		SP_Texture m_pTexBack;
		SP_Texture m_pTexBar;

		float	m_fTX = 0;		// Bar Translation
		float	m_fTY = 0;
		float	m_fSY = 1;		// Bar Scale

		bool	m_bDefaultLoader = false;	// Use the default loader?
	};

	// ******************************************************************

	Section* instance_loading()
	{
		return new sLoading();
	}

	sLoading::sLoading()
	{
		type = SectionType::Loading;
	}

	bool sLoading::load()
	{
		// script validation
		if ((this->param.size() != 3) || (this->strings.size() != 3)) {
			Logger::error("Loading [%s]: 3 strings and 3 params needed. Using default values.", this->identifier.c_str());
			m_bDefaultLoader = true;
		}
		else {
			m_bDefaultLoader = false;
		}

		if (!m_bDefaultLoader) {
			m_pTexBack = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[0]);
			m_pTexFront = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[1]);
			m_pTexBar = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[2]);
			m_fTX = param[0];
			m_fTY = param[1];
			m_fSY = param[2];
		}
		else {
			// Deault values
			m_pTexBack = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingback.jpg");
			m_pTexFront = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingfront.jpg");
			m_pTexBar = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingbar.jpg");
			m_fTX = 0.0f;
			m_fTY = -0.4f;
			m_fSY = 0.1f;
		}

		if (m_pTexBar == nullptr || m_pTexBack == nullptr || m_pTexFront == nullptr) {
			Logger::error("Loading [%s]: Could not load some of the loading textures", identifier.c_str());
		}

		return true;
	}

	void sLoading::init()
	{

	}

	void sLoading::exec()
	{
		// Prevent a crash if any texture is not found
		if (m_pTexBar == nullptr || m_pTexBack == nullptr || m_pTexFront == nullptr) {
			return;
		}

		float zero2one = 0;
		if (m_demo.m_sectionManager.m_loadSection.size() > 0)
			zero2one = (float)m_demo.m_iLoadedSections / (float)(m_demo.m_sectionManager.m_loadSection.size());

		GLDRV->initRender(true);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		// Background
		m_demo.m_pRes->Draw_QuadFS(m_pTexBack, 1 - zero2one);
		// Foreground
		m_demo.m_pRes->Draw_QuadFS(m_pTexFront, zero2one);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);


		// Draw the Loading bar
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(m_fTX, m_fTY, 0));  // Move the bar
		model = glm::scale(model, glm::vec3(zero2one, m_fSY, 0));		// Scale the bar
		m_demo.m_pRes->Draw_Obj_QuadTex(m_pTexBar, &model);

		GLDRV->swapBuffers();
	}

	void sLoading::end()
	{

	}
}