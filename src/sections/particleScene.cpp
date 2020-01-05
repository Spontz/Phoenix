#include "main.h"
#include "core/particleSystem.h"
#include "core/shadervars.h"

typedef struct {
	// 3D Model
	int			model;

	// Particle engine variables
	unsigned int	numMaxParticles;
	unsigned int	numEmitters;
	float			currentEmitter;
	float			emissionTime;
	float			particleLifeTime;
	float			particleSpeed;
	ParticleSystem *pSystem;

	// Particles positioning (for all the model)
	glm::vec3	traslation;
	glm::vec3	rotation;
	glm::vec3	scale;

	glm::vec3	velocity;		// Velocity
	glm::vec3	force;
	glm::vec3	color;
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object

} particleScene_section;

static particleScene_section *local;

sParticleScene::sParticleScene() {
	type = SectionType::ParticleMatrix;
}

bool sParticleScene::load() {
	// script validation
	//if (this->param.size() != 1) {
	//	LOG->Error("Particle Matrix [%s]: 1 param needed (particle number)", this->identifier.c_str());
	//	return false;
	//}

	local = (particleScene_section*)malloc(sizeof(particleScene_section));

	this->vars = (void *)local;

	// Load the shaders
	string pathShaders;
	pathShaders = DEMO->dataFolder + this->strings[0];

	// Load the model scene
	local->model = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[1]);

	if (local->model < 0)
		return false;

	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		local->exprPosition->expression += this->strings[i];

	local->exprPosition->SymbolTable.add_variable("tx", local->traslation.x);
	local->exprPosition->SymbolTable.add_variable("ty", local->traslation.y);
	local->exprPosition->SymbolTable.add_variable("tz", local->traslation.z);
	local->exprPosition->SymbolTable.add_variable("rx", local->rotation.x);
	local->exprPosition->SymbolTable.add_variable("ry", local->rotation.y);
	local->exprPosition->SymbolTable.add_variable("rz", local->rotation.z);
	local->exprPosition->SymbolTable.add_variable("sx", local->scale.x);
	local->exprPosition->SymbolTable.add_variable("sy", local->scale.y);
	local->exprPosition->SymbolTable.add_variable("sz", local->scale.z);

	local->exprPosition->SymbolTable.add_variable("partSpeed", local->particleSpeed);
	local->exprPosition->SymbolTable.add_variable("velX", local->velocity.x);
	local->exprPosition->SymbolTable.add_variable("velY", local->velocity.y);
	local->exprPosition->SymbolTable.add_variable("velZ", local->velocity.z);

	local->exprPosition->SymbolTable.add_variable("forceX", local->force.x);
	local->exprPosition->SymbolTable.add_variable("forceY", local->force.y);
	local->exprPosition->SymbolTable.add_variable("forceZ", local->force.z);

	local->exprPosition->SymbolTable.add_variable("colorR", local->color.r);
	local->exprPosition->SymbolTable.add_variable("colorG", local->color.g);
	local->exprPosition->SymbolTable.add_variable("colorB", local->color.b);

	local->exprPosition->SymbolTable.add_variable("nE", local->currentEmitter);

	// Load model properties
	Model *my_model;
	my_model = DEMO->modelManager.model[local->model];

	// Load the particle generator parameters
	int numEmitters = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		numEmitters += (int)my_model->meshes[i].unique_vertices_pos.size();
	}

	local->numEmitters = numEmitters;
	if (local->numEmitters <= 0) {
		LOG->Error("Particle Scene [%s]: No emitters found in the 3D model", this->identifier.c_str());
		return false;
	}

	local->exprPosition->SymbolTable.add_constant("TnE", (float)local->numEmitters);
	
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	local->exprPosition->compileFormula();

	local->emissionTime = this->param[0];
	if (local->emissionTime <= 0) {
		LOG->Error("Particle Scene [%s]: Emission time should be greater than 0", this->identifier.c_str());
		return false;
	}

	local->particleLifeTime = this->param[1];
	local->numMaxParticles = local->numEmitters + static_cast<unsigned int>(static_cast<float>(local->numEmitters)*local->particleLifeTime*(1.0f / local->emissionTime));

	LOG->Info(LOG_LOW, "Particle Scene [%s]: Num max of particles will be: %d", this->identifier.c_str(), local->numMaxParticles);

	vector<Particle> Emitter;
	Emitter.resize(local->numEmitters);

	// Load the emitters, based in our model vertexes
	int numEmitter = 0;
	local->currentEmitter = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		for (int j = 0; j < my_model->meshes[i].unique_vertices_pos.size(); j++) {
			local->exprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
			Emitter[numEmitter].Type = PARTICLE_TYPE_EMITTER;
			Emitter[numEmitter].Pos = my_model->meshes[i].unique_vertices_pos[j];
			Emitter[numEmitter].Vel = local->velocity;
			Emitter[numEmitter].Col = local->color;
			Emitter[numEmitter].lifeTime = 0.0f;
			numEmitter++;
			local->currentEmitter = static_cast<float>(numEmitter);
		}
	}

	// Create the particle system
	local->pSystem = new ParticleSystem(pathShaders, local->numMaxParticles, local->numEmitters, local->emissionTime, local->particleLifeTime);
	if (!local->pSystem->InitParticleSystem(this, Emitter, this->uniform))
		return false;

	return true;
}

void sParticleScene::init() {
}


static float lastTime = 0;

void sParticleScene::exec() {
	local = (particleScene_section *)this->vars;
	
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	local->exprPosition->Expression.value();
	
	glDepthMask(GL_FALSE); // Disable depth buffer writting

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, local->traslation);
	model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, local->scale);
	
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles
	float deltaTime = this->runTime - lastTime;
	deltaTime = deltaTime * local->particleSpeed;
	lastTime = this->runTime;
	if (deltaTime < 0) {
		deltaTime = -deltaTime;	// In case we rewind the demo
	}
	local->pSystem->force = local->force;
	
	local->pSystem->Render(deltaTime, vp, model, DEMO->camera->Position);


	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sParticleScene::end() {
}

string sParticleScene::debug() {
	local = (particleScene_section*)this->vars;

	string msg; 
	msg += "[ particleScene id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " numEmitters: " + std::to_string(local->numEmitters) + "\n";
	msg += " numMaxParticles: " + std::to_string(local->numMaxParticles) + "\n";
	return msg;
}
