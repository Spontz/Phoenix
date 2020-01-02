#include "main.h"
#include "core\particleSystem.h"

typedef struct {
	// 3D Model
	int			model;

	// Particle engine variables
	unsigned int	numMaxParticles;
	unsigned int	numEmitters;
	float			currentEmitter;
	float			emissionTime;
	float			particleLifeTime;
	float			particleSize;
	float			particleSpeed;
	int				particleTexture;
	ParticleSystem *pSystem;

	// Particles positioning (for all the model)
	float		tx, ty, tz;	// Traslation
	float		rx, ry, rz;	// Rotation
	float		sx, sy, sz;	// Scale

	float		velX, velY, velZ;		// Velocity
	float		forceX, forceY, forceZ;	// Force
	float		colorR, colorG, colorB;	// Color
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

	// Load the model scene
	local->model = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[0]);
	local->particleTexture = DEMO->textureManager.addTexture(DEMO->dataFolder + this->strings[1]);

	if (local->model < 0 || local->particleTexture < 0)
		return false;

	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		local->exprPosition->expression += this->strings[i];

	local->exprPosition->SymbolTable.add_variable("tx", local->tx);
	local->exprPosition->SymbolTable.add_variable("ty", local->ty);
	local->exprPosition->SymbolTable.add_variable("tz", local->tz);
	local->exprPosition->SymbolTable.add_variable("rx", local->rx);
	local->exprPosition->SymbolTable.add_variable("ry", local->ry);
	local->exprPosition->SymbolTable.add_variable("rz", local->rz);
	local->exprPosition->SymbolTable.add_variable("sx", local->sx);
	local->exprPosition->SymbolTable.add_variable("sy", local->sy);
	local->exprPosition->SymbolTable.add_variable("sz", local->sz);

	local->exprPosition->SymbolTable.add_variable("partSpeed", local->particleSpeed);
	local->exprPosition->SymbolTable.add_variable("velX", local->velX);
	local->exprPosition->SymbolTable.add_variable("velY", local->velY);
	local->exprPosition->SymbolTable.add_variable("velZ", local->velZ);

	local->exprPosition->SymbolTable.add_variable("forceX", local->forceX);
	local->exprPosition->SymbolTable.add_variable("forceY", local->forceY);
	local->exprPosition->SymbolTable.add_variable("forceZ", local->forceZ);

	local->exprPosition->SymbolTable.add_variable("colorR", local->colorR);
	local->exprPosition->SymbolTable.add_variable("colorG", local->colorG);
	local->exprPosition->SymbolTable.add_variable("colorB", local->colorB);

	local->exprPosition->SymbolTable.add_variable("nE", local->currentEmitter);

	//local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	//local->exprPosition->compileFormula();


	// Load model properties
	Model *my_model;
	my_model = DEMO->modelManager.model[local->model];

	// Load the particle generator parameters
	int numEmitters = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		numEmitters += (int)my_model->meshes[i].vertices.size();
	}

	local->numEmitters = numEmitters;
	local->exprPosition->SymbolTable.add_constant("TnE", (float)local->numEmitters);
	
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	local->exprPosition->compileFormula();

	local->emissionTime = this->param[0];
	if (local->emissionTime <= 0) {
		LOG->Error("Particle Scene [%s]: Emission tiem should be greater than 0", this->identifier.c_str());
		return false;
	}

	local->particleLifeTime = this->param[1];
	local->numMaxParticles = local->numEmitters + static_cast<unsigned int>(static_cast<float>(local->numEmitters)*local->particleLifeTime*(1.0f / local->emissionTime));
	local->particleSize = this->param[2];
	LOG->Info(LOG_LOW, "Particle Scene [%s]: Num max of particles will be: %d", this->identifier.c_str(), local->numMaxParticles);


	// TODO: In theory, this is not needed because the num of particles is now calculated... but could change this
	//if (local->numMaxParticles<(local->numEmitters + local->numEmitters*local->particleLifeTime/local->emissionTime))
	//	LOG->Info(LOG_HIGH, "Particle Matrix [%s]: NumMaxParticles is too low! should be greater than: numEmitters + numEmitters*ParticleLifetime/EmissionTime", this->identifier.c_str());


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
			Emitter[numEmitter].Vel = glm::vec3(local->velX, local->velY, local->velZ);
			Emitter[numEmitter].Col = glm::vec3(local->colorR, local->colorG, local->colorB);
			Emitter[numEmitter].lifeTime = 0.0f;
			numEmitter++;
			local->currentEmitter = static_cast<float>(numEmitter);
		}
	}

	// Create the particle system
	local->pSystem = new ParticleSystem(local->numMaxParticles, local->numEmitters, local->emissionTime, local->particleLifeTime, local->particleSize, local->particleTexture);
	if (!local->pSystem->InitParticleSystem(Emitter))
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
	model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
	model = glm::rotate(model, glm::radians(local->rx), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->ry), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rz), glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::	vec3(local->sx, local->sy, local->sz));
	
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles
	float deltaTime = this->runTime - lastTime;
	deltaTime = deltaTime * local->particleSpeed;
	lastTime = this->runTime;
	if (deltaTime < 0) {
		deltaTime = -deltaTime;	// In case we rewind the demo
		//glm::vec3 Position(0, 0, 3.8f);
		//local->pSystem->resetParticleSystem(Position);
	}
	local->pSystem->Render(deltaTime, vp, model, DEMO->camera->Position);


	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sParticleScene::end() {
}

string sParticleScene::debug() {
	return "[ particleScene id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
