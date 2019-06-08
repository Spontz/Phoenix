// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEM_H
#define LIPARTICLESYSTEM_HGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>


struct Particle {
	glm::vec3 pos;				// Position
	glm::vec3 speed;			// Speed
	unsigned char r, g, b, a;	// Color
	float size, angle, weight;
	float life;					// Remaining life of the particle. if < 0 : dead and unused.

	float cameraDistance;		// Distance to the camera. If the particle is dead, the value is -1
	
	// Sort in reverse order : far particles drawn first.
	bool operator<(const Particle& that) const {
		return this->cameraDistance > that.cameraDistance;
	}
};

class ParticleSystem
{
public:
	Particle *particle;
	Shader *shader;
	Texture *texture;
	unsigned int numMaxPart;		// Maximum number of particles to draw
	unsigned int numPartCount;		// Number of particles to draw
	unsigned int lastUsedParticle;	// Last Used Particle number
	float timeLifeStarts;			// Time when the life starts

	// Constructor with vectors
	ParticleSystem(unsigned int numMaxParticles);
	
	// Generate new Particles
	void genNewParticles(unsigned int numNewPart);
	// Calculate new Particles postion, color, life, etc... Based on the current time -> Should be called on each frame
	void calcParticlesProperties(float currentTime, glm::vec3 CameraPosition);
	unsigned int FindUnusedParticle();	// Finds a Particle in ParticlesContainer which isn't used yet.
	void SortParticles();
	void Draw();

private:
	int shaderNum;		// Shader number used for drawing the Particles
	int textureNum;		// Texture number used for drawing the Particles
	GLuint particleVA;				// Particle Vertex Array Object
	GLuint particleBillboardBuffer;	// Billboard vertex buffer with the quad for drawing the Particles
	GLuint particlePosBuffer;		// Buffer with each particle position
	GLuint particleColBuffer;		// Buffer with each particle color
	GLfloat* g_partPosData;	// array of floats with the Particle positions (for openGL buffer)
	GLubyte* g_partColorData;// array of bytes with the Particle colors (for openGL buffer)


	// Generation fo the Object Buffer
	void genObjectBuffer();

};


#endif