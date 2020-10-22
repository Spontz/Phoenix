#include "main.h"
#include "core/video.h"
#include "core/shadervars.h"

int kVideoStreamIndex = -1; // TODO: allow selecting the video stream index

class sDrawVideo final : public Section {
public:
	sDrawVideo();

public:
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	bool		m_bClearScreen  = false;	// Clear Screen buffer
	bool		m_bClearDepth   = false;	// Clear Depth buffer
	bool		m_bFitToContent = false;	// Fit to content: true:respect video aspect ratio, false:stretch to viewport.
	Video		*m_pVideo        = nullptr;
	Shader		*m_pShader       = nullptr;
	ShaderVars	*m_pShaderVars   = nullptr;	// Shader variables
};

// ******************************************************************

Section* instance_drawVideo()
{
	return new sDrawVideo();
}

sDrawVideo::sDrawVideo()
{
	type = SectionType::DrawVideo;
}

bool sDrawVideo::load()
{
	if ((param.size() != 3) || (strings.size() < 2)) {
		LOG->Error(
			"DrawVideo [%s]: 3 param needed (Clear screen buffer, clear depth buffer & "
			"fit to content) and 2 strings needed (Video to play + shader)",
			identifier.c_str()
		);
		return false;
	}

	m_bClearScreen = static_cast<bool>(param[0]);
	m_bClearDepth = static_cast<bool>(param[1]);
	m_bFitToContent = static_cast<bool>(param[2]);

	m_pVideo = m_demo.videoManager.addVideo(m_demo.dataFolder + strings[0], kVideoStreamIndex);
	if (m_pVideo == nullptr)
		return false;

	m_pShader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[1]);
	if (m_pShader == nullptr)
		return false;

	// Create Shader variables
	m_pShader->use();
	m_pShaderVars = new ShaderVars(this, m_pShader);

	// Read the shader variables
	for (auto const& s : uniform)
		m_pShaderVars->ReadString(s.c_str());

	// Set shader variables values
	m_pShaderVars->setValues();

	return true;
}

void sDrawVideo::init()
{

}

void sDrawVideo::exec()
{
	m_pVideo->renderVideo(runTime);

	if (m_bClearScreen)
		glClear(GL_COLOR_BUFFER_BIT);

	if (m_bClearDepth)
		glClear(GL_DEPTH_BUFFER_BIT);

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Texture and View aspect ratio, stored for Keeping image proportions
		const auto fTexAspect = static_cast<float>(m_pVideo->getWidth()) / static_cast<float>(m_pVideo->getHeight());
		const auto fViewportAspect = GLDRV->GetCurrentViewport().GetAspectRatio();

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float fXScale = 1.0f;
		float fYScale = 1.0f;
		if (m_bFitToContent) {
			if (fTexAspect > fViewportAspect)
				fYScale = fViewportAspect / fTexAspect;
			else
				fXScale = fTexAspect / fViewportAspect;
		}

		const auto modelMatrix = glm::scale(
			glm::identity<glm::mat4>(),
			glm::vec3(fXScale, fYScale, 0.0f)
		);

		m_pShader->use();
		m_pShader->setValue("model", modelMatrix);
		m_pShader->setValue("screenTexture", 0);
		// Set other shader variables values
		m_pShaderVars->setValues();
		m_pVideo->bind(0);
		m_demo.res->Draw_QuadFS(); // Draw a quad with the video

	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawVideo::end()
{
}

std::string sDrawVideo::debug()
{
	std::stringstream ss;
	ss << "+ DrawVideo id: " << identifier << " layer: " << layer << std::endl;
	ss << "  file: " << m_pVideo->getFileName() << std::endl;
	return ss.str();
}
