#include "main.h"
#include "core/video.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
	char	fitToContent;	// Fit to content: 1=respect video aspect ratio, 0=fill entire viewport window
	int		videoNum;
	int		shaderNum;
	ShaderVars	*shaderVars;	// Shader variables
} drawVideo_section;

static drawVideo_section *local;

// ******************************************************************

sDrawVideo::sDrawVideo() {
	type = SectionType::DrawVideo;
}

bool sDrawVideo::load() {
	if ((this->param.size() != 3) || (this->strings.size() < 3)) {
		LOG->Error("DrawVideo [%s]: 3 param needed (Clear screen buffer, clear depth buffer & fit to content) and 3 strings needed (Video to play + 2 shader files)", this->identifier.c_str());
		return false;
	}

	local = (drawVideo_section*)malloc(sizeof(drawVideo_section));
	this->vars = (void*)local;

	local->clearScreen = (char)this->param[0];
	local->clearDepth = (char)this->param[1];
	local->fitToContent = (char)this->param[2];
	local->videoNum = DEMO->videoManager.addVideo(DEMO->dataFolder + this->strings[0]);

	local->shaderNum = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2]);

	if ((local->videoNum == -1) || (local->shaderNum == -1))
		return false;

	// Create Shader variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[local->shaderNum];
	my_shader->use();
	local->shaderVars = new ShaderVars(this, my_shader);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->shaderVars->setValues();

	return true;
}

void sDrawVideo::init() {
	
}

void sDrawVideo::exec() {
	local = (drawVideo_section *)this->vars;

	Video *my_video = DEMO->videoManager.video[local->videoNum];
	my_video->renderVideo(this->runTime);


	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

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
		if (local->fitToContent) {
			if (tex_aspect > view_aspect) {
				new_tex_height_scaled = view_aspect / tex_aspect;
			}
			else {
				new_tex_width_scaled = tex_aspect / view_aspect;
			}
		}
		model = glm::scale(model, glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));

		//RES->Draw_Obj_QuadTex(local->texture, &model);
		glBindVertexArray(RES->obj_quadFullscreen);
		Shader *my_shader = DEMO->shaderManager.shader[local->shaderNum];
		my_shader->use();
		my_shader->setValue("model", model);
		my_shader->setValue("screenTexture", 0);
		// Set other shader variables values
		local->shaderVars->setValues();
		my_video->bind(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawVideo::end() {
	
}

string sDrawVideo::debug() {
	return string();
}
