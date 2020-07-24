#include "main.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	int			cubemap;
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;

	glm::vec3	rotation;
	glm::vec3	scale;
	
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
} drawSkybox_section;

static drawSkybox_section *local;

// ******************************************************************

sDrawSkybox::sDrawSkybox() {
	type = SectionType::DrawSkybox;
}

bool sDrawSkybox::load() {
	if ((this->param.size() != 2) || (this->strings.size() != 8)) {
		LOG->Error("DrawSkybox [%s]: 2 param and 8 strings needed: enable depthBuffer, drawWireframe + 6 strings with skybox faces, 2 strings with rot and scale", this->identifier.c_str());
		return false;
	}

	// Check if skybox is present in resources
	if (RES->obj_skybox < 0 || RES->shdr_Skybox < 0) {
		LOG->Error("DrawSkybox [%s]: Skybox model or shader has not been properly loaded in resources, please fix it!", this->identifier.c_str());
		return false;
	}

	local = (drawSkybox_section*)malloc(sizeof(drawSkybox_section));
	this->vars = (void*)local;

	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe = (int)this->param[1];
	
	// Load the 6 textures of our cubemap
	std::vector<std::string> faces {	DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2],
										DEMO->dataFolder + this->strings[3], DEMO->dataFolder + this->strings[4], DEMO->dataFolder + this->strings[5]};

	local->cubemap = DEMO->textureManager.addCubemap(faces, false);
	if (local->cubemap < 0)
		return false;

	// Read variables for traslation, rotation and scaling
	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprPosition->expression = std::string(this->strings[6]) + this->strings[7]; // Concatenate the 2 positioning strings (rotation+scale)
	local->exprPosition->SymbolTable.add_variable("rx", local->rotation.x);
	local->exprPosition->SymbolTable.add_variable("ry", local->rotation.y);
	local->exprPosition->SymbolTable.add_variable("rz", local->rotation.z);
	local->exprPosition->SymbolTable.add_variable("sx", local->scale.x);
	local->exprPosition->SymbolTable.add_variable("sy", local->scale.y);
	local->exprPosition->SymbolTable.add_variable("sz", local->scale.z);
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	if (!local->exprPosition->compileFormula())
		return false;

	return true;
}

void sDrawSkybox::init() {
	
}

void sDrawSkybox::exec() {
	local = (drawSkybox_section *)this->vars;
	// Evaluate the expression
	local->exprPosition->Expression.value();

	// Start evaluating blending
	EvalBlendingStart();
	
	// change depth function so depth test passes when values are equal to depth buffer's content
	glDepthFunc(GL_LEQUAL);


	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (local->enableDepthBufferClearing)
		glClear(GL_DEPTH_BUFFER_BIT);

	RES->shdr_Skybox->use(); // TODO: Do not use the Resource shader for skybox, and use our own shader!

	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = glm::mat4(glm::mat3(DEMO->camera->GetViewMatrix())); // remove translation from the view matrix
	RES->shdr_Skybox->setValue("projection", projection);
	RES->shdr_Skybox->setValue("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, local->scale);
	RES->shdr_Skybox->setValue("model", model);
	
	RES->shdr_Skybox->setValue("skybox", 0);
	RES->Draw_Skybox(local->cubemap);
	

	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawSkybox::end() {
	
}

std::string sDrawSkybox::debug() {
	return "[ drawSkybox id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
