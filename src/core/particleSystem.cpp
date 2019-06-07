// particleSystem.cpp
// Spontz Demogroup

#include "main.h"
#include "core/particleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>


using namespace std;

ParticleSystem::ParticleSystem(unsigned int numMaxParticles)
{
	this->timeLifeStarts = 0;
	this->lastUsedParticle = 0;
	this->numMaxPart = numMaxParticles;
	this->particle = new Particle[this->numMaxPart];
	this->g_partPosData = new GLfloat[this->numMaxPart * 4];	// TODO: Should be 3, isn't it? (xyz)
	this->g_partColorData = new GLubyte[this->numMaxPart * 4];	// *4, because RGBA

	for (unsigned int i = 0; i < this->numMaxPart; i++) {
		this->particle[i].pos = glm::vec3(0);
		this->particle[i].speed = glm::vec3(0);
		this->particle[i].r = 255;
		this->particle[i].g = 255;
		this->particle[i].b = 255;
		this->particle[i].a = 255;
		this->particle[i].size = 1;
		this->particle[i].angle = 0;
		this->particle[i].weight = 1;
		this->particle[i].life = -1;			// All particles dead by default
		this->particle[i].cameraDistance = -1;
	}

	// Load the Shader
	string demoDir = DEMO->demoDir;
	this->shaderNum = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/particleSystem/particleSystem.vert", demoDir + "/resources/shaders/particleSystem/particleSystem.frag");
	if (this->shaderNum < 0)
		LOG->Error("Error loading shader for the Particle System");
	else
		this->shader = DEMO->shaderManager.shader[this->shaderNum];

	// Load the texture
	this->textureNum = DEMO->textureManager.addTexture(demoDir + "/resources/textures/part_fire.jpg");
	if (this->textureNum < 0)
		LOG->Error("Error loading texture for the Particle System");
	else
		this->texture = DEMO->textureManager.texture[this->textureNum];

	// Generate the Object Buffers (Billboard, Position Buffer, Color Buffer, etc...)
	this->genObjectBuffer();
}

// Generate new particles
void ParticleSystem::genNewParticles(unsigned int numNewPart)
{
	for (unsigned int i = 0; i < numNewPart; i++) {
		int particleIndex = FindUnusedParticle();
		this->particle[particleIndex].life = 5.0f; // This particle will live 5 seconds.
		this->particle[particleIndex].pos = glm::vec3(0, 0, 0);

		float spread = 1.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		this->particle[particleIndex].speed = maindir + randomdir * spread; // Calculate the speed of the particle


		// Very bad way to generate a random color
		this->particle[particleIndex].r = rand() % 256;
		this->particle[particleIndex].g = rand() % 256;
		this->particle[particleIndex].b = rand() % 256;
		this->particle[particleIndex].a = (rand() % 256) / 3;

		this->particle[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;

	}
}

void ParticleSystem::calcParticlesProperties(float currentTime, glm::vec3 CameraPosition)
{
	float delta = currentTime - this->timeLifeStarts;	// Calculate the time has passed since the beggining
	// Simulate all particles
	unsigned int ParticlesCount = 0;
	for (unsigned int i = 0; i < this->numMaxPart; i++) {

		Particle& p = this->particle[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= delta;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
				p.pos += p.speed * (float)delta;
				p.cameraDistance = glm::length2(p.pos - CameraPosition);
				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

				// Fill the GPU buffer
				g_partPosData[4 * ParticlesCount + 0] = p.pos.x;
				g_partPosData[4 * ParticlesCount + 1] = p.pos.y;
				g_partPosData[4 * ParticlesCount + 2] = p.pos.z;

				g_partPosData[4 * ParticlesCount + 3] = p.size;

				g_partColorData[4 * ParticlesCount + 0] = p.r;
				g_partColorData[4 * ParticlesCount + 1] = p.g;
				g_partColorData[4 * ParticlesCount + 2] = p.b;
				g_partColorData[4 * ParticlesCount + 3] = p.a;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameraDistance = -1.0f;
			}

			ParticlesCount++;

		}
	}

	SortParticles();
}

unsigned int ParticleSystem::FindUnusedParticle() {

	for (unsigned int i = this->lastUsedParticle; i < this->numMaxPart; i++) {
		if (this->particle[i].life < 0) {
			this->lastUsedParticle = i;
			return i;
		}
	}

	for (unsigned int i = 0; i < this->lastUsedParticle; i++) {
		if (this->particle[i].life < 0) {
			this->lastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void ParticleSystem::SortParticles()
{
	std::sort(&this->particle[0], &this->particle[this->numMaxPart]);
}

void ParticleSystem::Draw()
{
	// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU, 
		// but this is outside the scope of this tutorial.
		// http://www.opengl.org/wiki/Buffer_Object_Streaming


	glBindBuffer(GL_ARRAY_BUFFER, this->particlePosBuffer);
	glBufferData(GL_ARRAY_BUFFER, this->numMaxPart * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->numPartCount * sizeof(GLfloat) * 4, this->g_partPosData);

	glBindBuffer(GL_ARRAY_BUFFER, this->particleColBuffer);
	glBufferData(GL_ARRAY_BUFFER, this->numMaxPart *4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->numPartCount * sizeof(GLubyte) * 4, this->g_partColorData);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->particleBillboardBuffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->particlePosBuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->particleColBuffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	// Draw the particles !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, this->numPartCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);



}

void ParticleSystem::genObjectBuffer()
{
	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
	glGenBuffers(1, &this->particleBillboardBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->particleBillboardBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &this->particlePosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->particlePosBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, this->numMaxPart * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &this->particleColBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->particleColBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, this->numMaxPart * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}
