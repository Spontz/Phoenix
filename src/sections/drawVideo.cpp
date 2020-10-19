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
	bool		m_bClearScreen_  = false;	// Clear Screen buffer
	bool		m_bClearDepth_   = false;	// Clear Depth buffer
	bool		m_bFitToContent_ = false;	// Fit to content: true:respect video aspect ratio, false:stretch to viewport.
	Video*		m_pVideo_        = nullptr;
	Shader*		m_pShader_       = nullptr;
	ShaderVars* m_pShaderVars_   = nullptr;	// Shader variables
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

	m_bClearScreen_ = static_cast<bool>(param[0]);
	m_bClearDepth_ = static_cast<bool>(param[1]);
	m_bFitToContent_ = static_cast<bool>(param[2]);

	m_pVideo_ = m_demo.videoManager.addVideo(m_demo.dataFolder + strings[0], kVideoStreamIndex);
	if (m_pVideo_ == nullptr)
		return false;

	m_pShader_ = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[1]);
	if (m_pShader_ == nullptr)
		return false;

	// Create Shader variables
	m_pShader_->use();
	m_pShaderVars_ = new ShaderVars(this, m_pShader_);

	// Read the shader variables
	for (auto const& s : uniform)
		m_pShaderVars_->ReadString(s.c_str());

	// Set shader variables values
	m_pShaderVars_->setValues();

	return true;
}

void sDrawVideo::init()
{

}

void sDrawVideo::exec()
{
	m_pVideo_->renderVideo(runTime);

	if (m_bClearScreen_)
		glClear(GL_COLOR_BUFFER_BIT);

	if (m_bClearDepth_)
		glClear(GL_DEPTH_BUFFER_BIT);

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Texture and View aspect ratio, stored for Keeping image proportions
		const auto fTexAspect = static_cast<float>(m_pVideo_->getWidth()) / static_cast<float>(m_pVideo_->getHeight());
		const auto fViewportAspect = GLDRV->GetCurrentViewport().GetAspectRatio();

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float fXScale = 1.0f;
		float fYScale = 1.0f;
		if (m_bFitToContent_) {
			if (fTexAspect > fViewportAspect)
				fYScale = fViewportAspect / fTexAspect;
			else
				fXScale = fTexAspect / fViewportAspect;
		}

		const auto modelMatrix = glm::scale(
			glm::identity<glm::mat4>(),
			glm::vec3(fXScale, fYScale, 0.0f)
		);

		m_pShader_->use();
		m_pShader_->setValue("model", modelMatrix);
		m_pShader_->setValue("screenTexture", 0);
		// Set other shader variables values
		m_pShaderVars_->setValues();
		m_pVideo_->bind(0);
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
	ss << "[ drawVideo id: " << identifier << " layer:" << layer << std::endl;
	ss << " filename: " << m_pVideo_->getFileName() << std::endl;
	return ss.str();
}
