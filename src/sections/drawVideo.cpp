#include "main.h"
#include "core/video.h"
#include "core/shadervars.h"

struct sDrawVideo : public Section {
public:
	sDrawVideo();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	char		clearScreen;	// Clear Screen buffer
	char		clearDepth;		// Clear Depth buffer
	char		fitToContent;	// Fit to content: 1=respect video aspect ratio, 0=fill entire viewport window
	int			videoNum;
	Shader*		shader;
	ShaderVars	*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_drawVideo() {
	return new sDrawVideo();
}

sDrawVideo::sDrawVideo() {
	type = SectionType::DrawVideo;
}

bool sDrawVideo::load() {
	if ((param.size() != 3) || (strings.size() < 2)) {
		LOG->Error("DrawVideo [%s]: 3 param needed (Clear screen buffer, clear depth buffer & fit to content) and 2 strings needed (Video to play + shader)", identifier.c_str());
		return false;
	}

	clearScreen = (char)param[0];
	clearDepth = (char)param[1];
	fitToContent = (char)param[2];
	videoNum = DEMO->videoManager.addVideo(DEMO->dataFolder + strings[0]);
	if (videoNum == -1)
		return false;


	shader = DEMO->shaderManager.addShader(DEMO->dataFolder + strings[1]);
	if (!shader)
		return false;

	
	// Create Shader variables
	shader->use();
	shaderVars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	shaderVars->setValues();

	return true;
}

void sDrawVideo::init() {
	
}

void sDrawVideo::exec() {
	Video *my_video = DEMO->videoManager.video[videoNum];
	my_video->renderVideo(runTime);


	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Texture and View aspect ratio, stored for Keeping image proportions
		float tex_aspect = (float)my_video->width / (float)my_video->height;
		float view_aspect = GLDRV->GetCurrentViewport().GetAspectRatio();
		
		// Put orthogonal mode
		glm::mat4 model = glm::mat4(1.0f);

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float new_tex_width_scaled = 1;
		float new_tex_height_scaled = 1;
		if (fitToContent) {
			if (tex_aspect > view_aspect) {
				new_tex_height_scaled = view_aspect / tex_aspect;
			}
			else {
				new_tex_width_scaled = tex_aspect / view_aspect;
			}
		}
		model = glm::scale(model, glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));

		//RES->Draw_Obj_QuadTex(texture, &model);
		glBindVertexArray(RES->obj_quadFullscreen);
		shader->use();
		shader->setValue("model", model);
		shader->setValue("screenTexture", 0);
		// Set other shader variables values
		shaderVars->setValues();
		my_video->bind(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawVideo::end() {
	
}

std::string sDrawVideo::debug() {
	Video *my_video = DEMO->videoManager.video[videoNum];

	std::string msg;
	msg = "[ drawVideo id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " filename: " + my_video->fileName + "\n";
	return msg;
}
