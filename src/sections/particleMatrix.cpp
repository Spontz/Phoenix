#include "main.h"
#include "core/particleMesh.h"
#include "core/shadervars.h"

typedef struct {
	// Particle engine variables
	unsigned int	numParticles;
	ParticleMesh*	pSystem;
	int				shader;

	// Particle Matrix positioning (for all the model)
	glm::vec3		translation;
	glm::vec3		rotation;
	glm::vec3		scale;

	mathDriver		*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars		*vars;			// For storing any other shader variables
} particleMatrix_section;

static particleMatrix_section* local;

sParticleMatrix::sParticleMatrix() {
	type = SectionType::ParticleMatrix;
}

bool sParticleMatrix::load() {
	// script validation
	if ((this->param.size() != 1) || (this->strings.size() != 5)) {
		LOG->Error("Particle Matrix [%s]: 1 param (Particles number) and 5 strings needed (2 for shader files, 3 for positioning)", this->identifier.c_str());
		return false;
	}
	

	local = (particleMatrix_section*)malloc(sizeof(particleMatrix_section));

	this->vars = (void*)local;

	// Load the shader
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shader < 0)
		return false;

	// Particles number
	local->numParticles = (int)this->param[0];

	// Load particle positioning
	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		local->exprPosition->expression += this->strings[i];

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
	local->exprPosition->compileFormula();

	// Create the particle system
	local->pSystem = new ParticleMesh(local->numParticles);
	if (!local->pSystem->startup())
		return false;

	// Create Shader variables
	Shader* my_shader;
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	local->vars = new ShaderVars(this, my_shader);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->vars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->vars->setValues();

	return true;
}

void sParticleMatrix::init() {
}


static float lastTime = 0;

void sParticleMatrix::exec() {
	local = (particleMatrix_section*)this->vars;

	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	local->exprPosition->Expression.value();

	glDepthMask(GL_FALSE); // Disable depth buffer writting

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, local->translation);
	model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, local->scale);

	glm::mat4 pvm = projection * view * model;

	// Get the shader
	Shader* my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	my_shader->setValue("gTime", this->runTime);	// Send the Time
	my_shader->setValue("gPVM", pvm);				// Set (Projection x View x Model) matrix
	my_shader->setValue("gNumParticles", (float)local->numParticles);	// Set the total number of particles

	// Set the other shader variable values
	local->vars->setValues();

	// Render particles
	local->pSystem->render(this->runTime, pvm);

	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sParticleMatrix::end() {
}

string sParticleMatrix::debug() {
	local = (particleMatrix_section*)this->vars;

	string msg;
	msg += "[ particleMatrix id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " numParticles: " + std::to_string(local->numParticles) + "\n";
	return msg;
}
