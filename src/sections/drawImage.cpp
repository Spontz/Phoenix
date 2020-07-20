#include "main.h"
#include "core/shadervars.h"

typedef struct {
	char		clearScreen;	// Clear Screen buffer
	int			texture;
	float		texAspectRatio;

	int			shader;

	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} drawImage_section;

static drawImage_section *local;

// ******************************************************************

sDrawImage::sDrawImage() {
	type = SectionType::DrawImage;
}


bool sDrawImage::load() {
	local = (drawImage_section*)malloc(sizeof(drawImage_section));
	this->vars = (void *)local;

	if (this->strings.size() < 5) {
		LOG->Error("Draw Image [%s]: 5 strings required (image path, shader and 3 por image positioning)", this->identifier.c_str());
		return false;
	}

	// Texture load
	local->texture = DEMO->textureManager.addTexture(DEMO->dataFolder + this->strings[0]);

	if (local->texture == -1)
		return false;
	// Load the background texture
	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[local->texture];
	local->texAspectRatio = (float)my_tex->height / (float)my_tex->width;

	// Load the shader to apply
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1]);
	if (local->shader == -1)
		return false;

	// Load the formmula containing the Image position and scale
	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	// TODO: Change this with a for and read all strings like in the other sections
	local->exprPosition->expression = this->strings[2] + this->strings[3] + this->strings[4]; // Concatenate the 3 positioning strings (position+rotation+scale)
	local->exprPosition->SymbolTable.add_variable("tx", local->translation.x);
	local->exprPosition->SymbolTable.add_variable("ty", local->translation.y);
	local->exprPosition->SymbolTable.add_variable("tz", local->translation.z);
	local->exprPosition->SymbolTable.add_variable("rx", local->rotation.x);
	local->exprPosition->SymbolTable.add_variable("ry", local->rotation.y);
	local->exprPosition->SymbolTable.add_variable("rz", local->rotation.z);
	local->exprPosition->SymbolTable.add_variable("sx", local->scale.x);
	local->exprPosition->SymbolTable.add_variable("sy", local->scale.y);
	local->exprPosition->SymbolTable.add_variable("sz", local->scale.z);
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	if (!local->exprPosition->compileFormula())
		return false;

	// Create shader variables
	Shader *my_shad;
	my_shad = DEMO->shaderManager.shader[local->shader];
	my_shad->use();
	local->vars = new ShaderVars(this, my_shad);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->vars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->vars->setValues();

	return true;
}

void sDrawImage::init() {
	
}

void sDrawImage::exec() {
	local = (drawImage_section *)this->vars;

	// Evaluate the expression
	local->exprPosition->Expression.value();

	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[local->texture];
	Shader *my_shad;
	my_shad = DEMO->shaderManager.shader[local->shader];


	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// View / projection / model Matrixes
		glm::mat4 view = DEMO->camera->GetViewMatrix();
		float zoom = DEMO->camera->Zoom;
		glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, local->translation);
		model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(local->scale.x, local->scale.y*local->texAspectRatio, local->scale.z));

		// Draw the image
		my_shad->use();
		my_shad->setValue("model", model);
		my_shad->setValue("projection", projection);
		my_shad->setValue("view", view);
		my_shad->setValue("screenTexture", 0);

		my_tex->bind();

		// Set the values
		local->vars->setValues();

		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sDrawImage::end() {
	
}

std::string sDrawImage::debug() {
	local = (drawImage_section *)this->vars;
	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[local->texture];

	std::string msg;
	msg = "[ drawImage id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " filename: " + my_tex->filename + "\n";
	return msg;
}
