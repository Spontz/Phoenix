#include "main.h"
#include "core/particleSystemEvo.h"
#include "core/shadervars.h"

typedef struct {
	// Particle engine variables
	unsigned int	numParticles;
	ParticleSystemEvo* pSystem;

	// Particle Matrix positioning (for all the model)
	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	// Individual particle properties
	float		currentParticle;
	glm::vec3	position;
	glm::vec3	color;

	mathDriver* exprPosition;	// A equation containing the calculations to position the object

} particleMatrix_section;

static particleMatrix_section* local;

sParticleMatrix::sParticleMatrix() {
	type = SectionType::ParticleMatrix;
}

bool sParticleMatrix::load() {
	// script validation
	/*if ((this->param.size() != 2) || (this->strings.size() != 9)) {
		LOG->Error("Particle Matrix [%s]: 2 param (emission time & Particle Life Time) and 9 strings needed (shader path, model, 3 for positioning, part speed, velocity, force and color)", this->identifier.c_str());
		return false;
	}
	*/

	local = (particleMatrix_section*)malloc(sizeof(particleMatrix_section));

	this->vars = (void*)local;

	// Load the shaders
	string pathShaders;
	pathShaders = DEMO->dataFolder + this->strings[0];

	// Particles number
	local->numParticles = (int)this->param[0];

	// Load particle positioning
	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 1; i < strings.size(); i++)
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

	local->exprPosition->SymbolTable.add_variable("px", local->position.x);
	local->exprPosition->SymbolTable.add_variable("py", local->position.y);
	local->exprPosition->SymbolTable.add_variable("pz", local->position.z);

	local->exprPosition->SymbolTable.add_variable("colorR", local->color.r);
	local->exprPosition->SymbolTable.add_variable("colorG", local->color.g);
	local->exprPosition->SymbolTable.add_variable("colorB", local->color.b);

	local->exprPosition->SymbolTable.add_variable("nE", local->currentParticle);
	local->exprPosition->SymbolTable.add_constant("TnE", (float)local->numParticles);

	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	local->exprPosition->compileFormula();

	// Create the particle system
	local->pSystem = new ParticleSystemEvo(local->numParticles, pathShaders);
	if (!local->pSystem->startup())
		return false;

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

	// Render particles
	float deltaTime = this->runTime - lastTime;
	lastTime = this->runTime;
	if (deltaTime < 0) {
		deltaTime = -deltaTime;	// In case we rewind the demo
	}
	

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
