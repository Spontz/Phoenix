#include "main.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	int			cubemap;
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;

	float	tx,ty,tz;// Translation
	float	rx,ry,rz;// Rotation
	float	sx,sy,sz;// Scale
	
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
} drawSkybox_section;

static drawSkybox_section *local;

// ******************************************************************

sDrawSkybox::sDrawSkybox() {
	type = SectionType::ObjectShader;
}

bool sDrawSkybox::load() {
	string s_demo = DEMO->demoDir;
	if ((this->paramNum < 2) || (this->stringNum < 9)) {
		LOG->Error("DrawSkybox [%s]: 2 param and 9 strings needed: enable depthBuffer, drawWireframe, 6 strings with skybox faces, 3 strings with pos, rot and scale", this->identifier.c_str());
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
	local->drawWireframe= (int)this->param[1];
	
	// Load the 6 texture cubemaps
	vector<std::string> faces {	s_demo + this->strings[0],s_demo + this->strings[1],s_demo + this->strings[2],
								s_demo + this->strings[3],s_demo + this->strings[4],s_demo + this->strings[5]};

	local->cubemap = DEMO->textureManager.addCubemap(faces, false);
	if (local->cubemap < 0)
		return false;

	// Read variables for traslation, rotation and scaling
	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprPosition->expression = std::string(this->strings[6]) + this->strings[7] + this->strings[8]; // Concatenate the 3 positioning strings (position+rotation+scale)
	local->exprPosition->SymbolTable.add_variable("tx", local->tx);
	local->exprPosition->SymbolTable.add_variable("ty", local->ty);
	local->exprPosition->SymbolTable.add_variable("tz", local->tz);
	local->exprPosition->SymbolTable.add_variable("rx", local->rx);
	local->exprPosition->SymbolTable.add_variable("ry", local->ry);
	local->exprPosition->SymbolTable.add_variable("rz", local->rz);
	local->exprPosition->SymbolTable.add_variable("sx", local->sx);
	local->exprPosition->SymbolTable.add_variable("sy", local->sy);
	local->exprPosition->SymbolTable.add_variable("sz", local->sz);
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	local->exprPosition->compileFormula();

	return true;
}

void sDrawSkybox::init() {
	
}

void sDrawSkybox::exec() {
	local = (drawSkybox_section *)this->vars;
	// Evaluate the expression
	local->exprPosition->Expression.value();

	// Start evaluating blending
	//EvalBlendingStart();
	
	// change depth function so depth test passes when values are equal to depth buffer's content
	glDepthFunc(GL_LEQUAL);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//if (local->enableDepthBufferClearing)
	//	glClear(GL_DEPTH_BUFFER_BIT);

	Shader *my_shader = DEMO->shaderManager.shader[RES->shdr_Skybox];
	my_shader->use();

	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shader->setValue("projection", projection);
	my_shader->setValue("view", view);

	// render the loaded model
	/*glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
	model = glm::rotate(model, glm::radians(local->rx), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->ry), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rz), glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(local->sx, local->sy, local->sz));
	my_shader->setValue("model", model);
	*/
	//glDisable(GL_DEPTH_TEST);
	//{
	//glDisable(GL_BLEND);
		my_shader->setValue("skybox", 0);
		RES->Draw_Skybox(local->cubemap);
	//}
	//glEnable(GL_DEPTH_TEST);

	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	//EvalBlendingEnd();
}

void sDrawSkybox::end() {
	
}
