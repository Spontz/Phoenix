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

		glm::vec2	startPoint =	{ -0.5, -0.5 };
		glm::vec2	endPoint =		{  0.5, -0.5 };
		float		size = 0.1f;

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
		if ((param.size() != 5) || (strings.size() != 3)) {
			Logger::error("Loading [{}]: 3 strings and 5 params needed. Using default values.", identifier);
			m_bDefaultLoader = true;
		}
		else {
			m_bDefaultLoader = false;
		}

		if (!m_bDefaultLoader) {
			m_pTexBack = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[0]);
			m_pTexFront = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[1]);
			m_pTexBar = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[2]);
			startPoint.x = param[0];
			startPoint.y = param[1];
			endPoint.x = param[2];
			endPoint.y = param[3];
			size = param[4];
		}
		else {
			// Deault values
			m_pTexBack = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingback.jpg");
			m_pTexFront = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingfront.jpg");
			m_pTexBar = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/loading/loadingbar.jpg");
			startPoint.x = -0.5f;
			startPoint.y = -0.5f;
			endPoint.x = 0.5f;
			endPoint.y = -0.5f;
			size = 0.1f;
		}

		if (m_pTexBar == nullptr || m_pTexBack == nullptr || m_pTexFront == nullptr) {
			Logger::error("Loading [{}]: Could not load some of the loading textures", identifier);
		}

		return !DEMO_checkGLError();
	}

	void sLoading::init()
	{

	}

	// Get the Model Matrix of the bar for given 2 points
	// barStartPoint - point where the bar should be started
	// barEndPoint - point where the bar should finish
	// barHeight - Height of the bar
	// zero2one - variable form 0.0 to 1.0 that tells us how the bar should be drawn (0 means that the bar is at
	// the start point, and 1.0 means that the bar is complete (from start to end)
	glm::mat4 getModelMatrixForBar(glm::vec2 barStartPoint, glm::vec2 barEndPoint, float barHeight, float zero2one)
	{
		// Draw the Loading bar
		glm::vec2 direction = barEndPoint - barStartPoint;
		float length = glm::length(direction);
		glm::vec2 directionNormalized = glm::normalize(direction);
		float angle = atan2(directionNormalized.y, directionNormalized.x);
		float scaledLength = length * zero2one;
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(barStartPoint, 0.0f));									// Translate to the startPoint
		model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));								// Rotate to align with the direction vector
		model = glm::scale(model, glm::vec3(scaledLength * 0.5, barHeight, 1.0f));					// Scale the bar to the correct length (in the x-direction)

		// Correct the effect of the scale and angle
		float newX = (scaledLength * 0.5f) * cos(angle);
		float newY = (scaledLength * 0.5f) * sin(angle);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(newX, newY, 0)) * model;// Correct the effect of the scale

		return model;
	}


	void sLoading::exec()
	{
		// Prevent a crash if any texture is not found
		if (m_pTexBar == nullptr || m_pTexBack == nullptr || m_pTexFront == nullptr) {
			return;
		}

		// Evaluate section loading
		float zero2oneLoading = 0;
		if (m_demo.m_sectionManager.m_loadSection.size() > 0)
			zero2oneLoading = static_cast<float>(m_demo.m_sectionManager.m_LoadedSections) / static_cast<float>(m_demo.m_sectionManager.m_loadSection.size());

		// Evaluate section warming
		float zero2oneWarming = 0;
		if (m_demo.m_sectionManager.m_loadSection.size() > 0)
			zero2oneWarming = static_cast<float>(m_demo.m_sectionManager.m_WarmedSections) / static_cast<float>(m_demo.m_sectionManager.m_loadSection.size());

		// Final zero2one /(70% is section loading, 30% section warming)
		float zero2one = 0.7f * zero2oneLoading + 0.3f * zero2oneWarming;

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
			glm::mat4 model = getModelMatrixForBar(startPoint, endPoint, size, zero2one);
			
			m_demo.m_pRes->drawObjQuadTex(m_pTexBar, &model);

		}
		m_demo.m_Window->OnUpdate();
	}

	void sLoading::end()
	{

	}
}