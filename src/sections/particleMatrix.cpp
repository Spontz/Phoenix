#include "main.h"
#include "core\particleSystem.h"

typedef struct {
	unsigned int	numMaxParticles;
	unsigned int	numEmitters;
	float			emissionTime;
	float			particleLifeTime;
	float			particleSize;
	int				particleTexture;
	ParticleSystem *pSystem;

} particleMatrix_section;

static particleMatrix_section *local;

sParticleMatrix::sParticleMatrix() {
	type = SectionType::ParticleMatrix;
}

bool sParticleMatrix::load() {
	// script validation
	//if (this->param.size() != 1) {
	//	LOG->Error("Particle Matrix [%s]: 1 param needed (particle number)", this->identifier.c_str());
	//	return false;
	//}

	local = (particleMatrix_section*)malloc(sizeof(particleMatrix_section));

	this->vars = (void *)local;

	// Load the parameters
	local->numMaxParticles = (unsigned int)this->param[0];
	local->numEmitters = (unsigned int)this->param[1];
	local->emissionTime = this->param[2];
	local->particleLifeTime = this->param[3];
	local->particleSize = this->param[4];
	local->particleTexture = DEMO->textureManager.addTexture(DEMO->dataFolder + this->strings[0]);
	

	if (local->emissionTime>0)
		if (local->numMaxParticles<(local->numEmitters + local->numEmitters*local->particleLifeTime/local->emissionTime))
			LOG->Info(LOG_HIGH, "Particle Matrix [%s]: NumMaxParticles is too low! should be greater than: numEmitters + numEmitters*ParticleLifetime/EmissionTime", this->identifier.c_str());

	if (local->particleTexture == -1)
		return false;

	// Create the particle system
	local->pSystem = new ParticleSystem(local->numMaxParticles, local->numEmitters, local->emissionTime, local->particleLifeTime, local->particleSize, local->particleTexture);
	glm::vec3 Position(0, 0, -10);
	local->pSystem->InitParticleSystem(Position);

	return true;
}

void sParticleMatrix::init() {
}


float lastTime = 0;

void sParticleMatrix::exec() {
	local = (particleMatrix_section *)this->vars;
	
	// Start evaluating blending
	EvalBlendingStart();
	glDepthMask(GL_FALSE); // Disable depth buffer writting

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), GLDRV->AspectRatio, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles
	float deltaTime = this->runTime - lastTime;
	lastTime = this->runTime;
	if (deltaTime < 0) {
		deltaTime = -deltaTime;	// In case we rewind the demo
		//glm::vec3 Position(0, 0, 3.8f);
		//local->pSystem->resetParticleSystem(Position);
	}
	local->pSystem->Render(deltaTime, vp, DEMO->camera->Position);


	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sParticleMatrix::end() {
}
