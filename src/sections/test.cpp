#include "main.h"


typedef struct {
	int			texture;
	int			shader;
	float		texAspectRatio;

	float		tx, ty, tz;	// Traslation
	float		rx, ry, rz;	// Rotation
	float		sx, sy, sz;	// Scale

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object

} test_section;

static test_section *local;


sTest::sTest() {
	type = SectionType::Test;
}

bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));
	this->vars = (void *)local;
	
	if (this->strings.size() < 2) {
		LOG->Error("Draw Image [%s]: 3 strings", this->identifier.c_str());
		return false;
	}

	// Texture load
	local->texture = DEMO->textureManager.addTexture(DEMO->dataFolder + this->strings[0], true);

	if (local->texture == -1)
		return false;
	// Load the background texture
	Texture *my_tex = DEMO->textureManager.texture[local->texture];
	local->texAspectRatio = (float)my_tex->height / (float)my_tex->width;

	// Load the shader to apply
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2]);

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

	return true;
}

void sTest::init() {
}

int error = 0;

void sTest::exec() {
	local = (test_section *)this->vars;

	// Evaluate the expression
	local->exprPosition->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Load the background texture
		Texture *my_tex = DEMO->textureManager.texture[local->texture];
		// Texture and View aspect ratio, stored for Keeping image proportions
		
		// Put orthogonal mode
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
		model = glm::rotate(model, glm::radians(local->rx), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(local->ry), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(local->rz), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(local->sx, local->sy*local->texAspectRatio, local->sz));
		
		RES->Draw_Obj_QuadTex(local->texture, &model);
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sTest::end() {
}
