#include "main.h"

namespace Phoenix {

	class sLoading final : public Section {
	public:
		sLoading();
		~sLoading();

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

	sLoading::~sLoading()
	{
	}

	bool sLoading::load()
	{
		// script validation
		if ((param.size() != 3) || (strings.size() != 3)) {
			Logger::error("Loading [{}]: 3 strings and 3 params needed. Using default values.", identifier);
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
			Logger::error("Loading [{}]: Could not load some of the loading textures", identifier);
		}

		return !DEMO_checkGLError();
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
			zero2one = static_cast<float>(m_demo.m_sectionManager.m_LoadedSections) / static_cast<float>(m_demo.m_sectionManager.m_loadSection.size());

		m_demo.m_Window->InitRender(true);
		{
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			// Background
			m_demo.m_pRes->drawQuadFS(m_pTexBack, 1 - zero2one);
			// Foreground
			m_demo.m_pRes->drawQuadFS(m_pTexFront, zero2one);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);


			// Draw the Loading bar
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(m_fTX, m_fTY, 0));  // Move the bar
			model = glm::scale(model, glm::vec3(zero2one, m_fSY, 0));		// Scale the bar
			m_demo.m_pRes->drawObjQuadTex(m_pTexBar, &model);
		}
		m_demo.m_Window->OnUpdate();
	}

	void sLoading::end()
	{

	}
}