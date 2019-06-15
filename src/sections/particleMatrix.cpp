#include "main.h"
#include "core\particleSystem.h"

typedef struct {
	unsigned int numParticles;
	ParticleSystem *pSystem;

} particleMatrix_section;

static particleMatrix_section *local;

sParticleMatrix::sParticleMatrix() {
	type = SectionType::ParticleMatrix;
}

bool sParticleMatrix::load() {
	// script validation
	if (this->param.size() != 1) {
		LOG->Error("Particle Matrix [%s]: 1 param needed (particle number)", this->identifier.c_str());
		return false;
	}

	local = (particleMatrix_section*)malloc(sizeof(particleMatrix_section));

	this->vars = (void *)local;

	// Load the parameters
	local->numParticles = (unsigned int)this->param[0];
	
	// Create the particle system
	local->pSystem = new ParticleSystem();// local->numParticles);
	glm::vec3 Position(0, 0, 2.8f);
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
	glDepthMask(0);

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles
	float delta = this->runTime - lastTime;
	lastTime = this->runTime;
	if (delta < 0) {
		delta = -delta;	// In case we rewind the demo
	}
	local->pSystem->Render((int)(delta*1000.0f), vp, DEMO->camera->Position);


	// End evaluating blending
	EvalBlendingEnd();

}

void sParticleMatrix::end() {
}
