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
	local->pSystem = new ParticleSystem(local->numParticles);
	
	return true;
}

void sParticleMatrix::init() {
}

void sParticleMatrix::exec() {
	local = (particleMatrix_section *)this->vars;
	
	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	


	// Generate new particles
	local->pSystem->genNewParticles(this->runTime*10);
	local->pSystem->calcParticlesProperties(this->runTime, DEMO->camera->Position);
	LOG->Info(LOG_LOW, "Section run time: %.3f", this->runTime);

	/////////////////////////
	// DRAW start
	// Use our shder
	local->pSystem->shader->use();
	local->pSystem->shader->setValue("VP", vp);	// Send the View-Projection matrix
	local->pSystem->shader->setValue("CameraRight_worldspace", DEMO->camera->Right);	// Send the Camera Right vector
	local->pSystem->shader->setValue("CameraUp_worldspace", DEMO->camera->Up);			// Send the Camera Up vector

	// Bind our texture in Texture Unit 0
	local->pSystem->texture->active();
	local->pSystem->texture->bind();
	local->pSystem->shader->setValue("myTextureSampler", 0); // Set our "myTextureSampler" sampler to use Texture Unit 0


	local->pSystem->Draw();

}

void sParticleMatrix::end() {
}
