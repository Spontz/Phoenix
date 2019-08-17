#include "main.h"
#include "core/shadervars.h"

typedef struct {
	char		clearScreen;	// Clear Screen buffer
	int			texture;
	Texture		*my_tex;
	float		texAspectRatio;

	int			shader;
	Shader		*my_shad;

	float		tx, ty, tz;	// Traslation
	float		rx, ry, rz;	// Rotation
	float		sx, sy, sz;	// Scale

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

	if (this->strings.size() < 6) {
		LOG->Error("Draw Image [%s]: 6 strings required (image path, 2 for shader and 3 por image positioning)", this->identifier.c_str());
		return false;
	}

	// Texture load
	local->texture = DEMO->textureManager.addTexture(DEMO->dataFolder + this->strings[0], true);

	if (local->texture == -1)
		return false;
	// Load the background texture
	local->my_tex = DEMO->textureManager.texture[local->texture];
	local->texAspectRatio = (float)local->my_tex->height / (float)local->my_tex->width;

	// Load the shader to apply
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2]);
	if (local->shader == -1)
		return false;
	local->my_shad = DEMO->shaderManager.shader[local->shader];

	// Load the formmula containing the Image position and scale
	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprPosition->expression = this->strings[3] + this->strings[4] + this->strings[5]; // Concatenate the 3 positioning strings (position+rotation+scale)
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

	// Create shader variables
	local->my_shad->use();
	local->vars = new ShaderVars(this, local->my_shad);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->vars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->vars->setValues(true);

	return true;
}

void sDrawImage::init() {
	
}

void sDrawImage::exec() {
	local = (drawImage_section *)this->vars;

	// Evaluate the expression
	local->exprPosition->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// View / projection / model Matrixes
		glm::mat4 view = DEMO->camera->GetViewMatrix();
		float zoom = DEMO->camera->Zoom;
		glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->AspectRatio, 0.1f, 10000.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
		model = glm::rotate(model, glm::radians(local->rx), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(local->ry), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(local->rz), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(local->sx, local->sy*local->texAspectRatio, local->sz));

		// Draw the image
		local->my_shad->use();
		local->my_shad->setValue("model", model);
		local->my_shad->setValue("projection", projection);
		local->my_shad->setValue("view", view);
		local->my_shad->setValue("screenTexture", 0);

		local->my_tex->bind();

		// Set the values
		local->vars->setValues(false);

		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sDrawImage::end() {
	
}
