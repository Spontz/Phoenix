#include "main.h"
#include "core\particleSystem.h"
#include "core\particleSystem_tf.h"

typedef struct {
	unsigned int numParticles;
	ParticleSystem *pSystem;
	CParticleSystemTransformFeedback *pSystem2;

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
	local->pSystem = new ParticleSystem();// (local->numParticles);
	glm::vec3 Position(0.0f);
	local->pSystem->InitParticleSystem(Position);

	// Create the particle system 2
	local->pSystem2 = new CParticleSystemTransformFeedback();
	local->pSystem2->InitalizeParticleSystem();
	
	local->pSystem2->SetGeneratorProperties(
		glm::vec3(0.0f, 0.0f, 0.0f), // Where the particles are generated
		glm::vec3(-5, 0, -5), // Minimal velocity
		glm::vec3(5, 20, 5), // Maximal velocity
		glm::vec3(0, -5, 0), // Gravity force applied to particles
		glm::vec3(0.0f, 0.5f, 1.0f), // Color (light blue)
		1.5f, // Minimum lifetime in seconds
		3.0f, // Maximum lifetime in seconds
		0.75f, // Rendered size
		0.02f, // Spawn every 0.05 seconds
		30); // And spawn 30 particles

	return true;
}

void sParticleMatrix::init() {
}


float lastTime = 0;

void sParticleMatrix::exec() {
	local = (particleMatrix_section *)this->vars;
	
	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles: Using Geometry shader 1
	//local->pSystem->Render(this->runTime * 1000, vp, DEMO->camera->Position);

	// Render particles: Using Geometry shader 2
	local->pSystem2->SetMatrices(&projection, DEMO->camera->Position, DEMO->camera->Front, DEMO->camera->Up);
	
	float delta = this->runTime - lastTime;
	lastTime = this->runTime;
	local->pSystem2->UpdateParticles(delta);
	local->pSystem2->RenderParticles();

	
	
	/*
	// Generate new particles - Not using geometry shaders
	local->pSystem->genNewParticles(this->runTime, 1);
	local->pSystem->calcParticlesProperties(this->runTime, DEMO->camera->Position);
	

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
	*/

}

void sParticleMatrix::end() {
}
