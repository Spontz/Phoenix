#include "main.h"
#include "core/shadervars.h"

struct sDrawImage : public Section {
public:
	sDrawImage();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	char		clearScreen;	// Clear Screen buffer
	int			texture;
	float		texAspectRatio;

	Shader*		shader;

	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} drawImage_section;

// ******************************************************************

Section* instance_drawImage() {
	return new sDrawImage();
}

sDrawImage::sDrawImage() {
	type = SectionType::DrawImage;
}


bool sDrawImage::load() {
	if (strings.size() < 5) {
		LOG->Error("Draw Image [%s]: 5 strings required (image path, shader and 3 por image positioning)", identifier.c_str());
		return false;
	}

	// Texture load
	texture = DEMO->textureManager.addTexture(DEMO->dataFolder + strings[0]);

	if (texture == -1)
		return false;
	// Load the background texture
	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[texture];
	texAspectRatio = (float)my_tex->height / (float)my_tex->width;

	// Load the shader to apply
	shader = DEMO->shaderManager.addShader(DEMO->dataFolder + strings[1]);
	if (!shader)
		return false;

	// Load the formmula containing the Image position and scale
	exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	// TODO: Change this with a for and read all strings like in the other sections
	exprPosition->expression = strings[2] + strings[3] + strings[4]; // Concatenate the 3 positioning strings (position+rotation+scale)
	exprPosition->SymbolTable.add_variable("tx", translation.x);
	exprPosition->SymbolTable.add_variable("ty", translation.y);
	exprPosition->SymbolTable.add_variable("tz", translation.z);
	exprPosition->SymbolTable.add_variable("rx", rotation.x);
	exprPosition->SymbolTable.add_variable("ry", rotation.y);
	exprPosition->SymbolTable.add_variable("rz", rotation.z);
	exprPosition->SymbolTable.add_variable("sx", scale.x);
	exprPosition->SymbolTable.add_variable("sy", scale.y);
	exprPosition->SymbolTable.add_variable("sz", scale.z);
	exprPosition->Expression.register_symbol_table(exprPosition->SymbolTable);
	if (!exprPosition->compileFormula())
		return false;

	// Create shader variables
	shader->use();
	vars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		vars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	vars->setValues();

	return true;
}

void sDrawImage::init() {
	
}

void sDrawImage::exec() {
	// Evaluate the expression
	exprPosition->Expression.value();

	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[texture];


	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// View / projection / model Matrixes
		glm::mat4 view = DEMO->camera->GetViewMatrix();
		float zoom = DEMO->camera->Zoom;
		glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, translation);
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(scale.x, scale.y*texAspectRatio, scale.z));

		// Draw the image
		shader->use();
		shader->setValue("model", model);
		shader->setValue("projection", projection);
		shader->setValue("view", view);
		shader->setValue("screenTexture", 0);

		my_tex->bind();

		// Set the values
		vars->setValues();

		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sDrawImage::end() {
	
}

std::string sDrawImage::debug() {
	Texture *my_tex;
	my_tex = DEMO->textureManager.texture[texture];

	std::string msg;
	msg = "[ drawImage id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " filename: " + my_tex->filename + "\n";
	return msg;
}
