// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

//#include <glad/glad.h>
//#include <glm/glm.hpp>

#include "main.h"

#define PARTICLE_TYPE_EMITTER 0
#define PARTICLE_TYPE_SHELL 1

struct Particle
{
	glm::vec3 Pos;	// Position:	loc 0 (vec3)
	glm::vec3 Vel;	// Velocity:	loc 1 (vec3)
	glm::vec3 Col;	// Color:		loc 2 (vec3)
	float lifeTime;	// lifeTime:	loc 3 (float)
	int Type;		// type:		loc 4 (int)
};

class ParticleSystem
{
public:
	ParticleSystem(unsigned int	numMaxParticles, unsigned int numEmitters, float emissionTime, float particleLifeTime, float particleSize, int particleTexture);
	~ParticleSystem();

	bool InitParticleSystem(const vector<Particle> emitter);
	void Render(float deltaTime, const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos);
	void resetParticleSystem(const glm::vec3 &Pos);

private:

	void UpdateEmitters(float deltaTime);
	void UpdateParticles(float deltaTime);
	void RenderParticles(const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos);

	bool initShaderBillboard();		// For drawing the quads using geometry shaders
	bool initShaderParticleSystem();// For drawing the particles using geometry shaders

	unsigned int	numMaxParticles;	// Number of maximum particles
	unsigned int	numEmitters;		// Number of emmiters
	float			emissionTime;
	float			particleLifeTime;
	float			particleSize;
	int				particleTexture;


	bool m_isFirst;
	unsigned int m_currVB;			// Variable that indicates the current VB (The value is 0 or 1)
	unsigned int m_currTFB;			// Variable that indicates the current TFB (The value is 0 or 1)
	GLuint m_VAO;					// Vertex Array Object buffer
	GLuint m_particleBuffer[2];		// Vertex Buffer: input
	GLuint m_transformFeedback[2];	// Transform feedback buffers: output
	int particleSystemShader;
	int billboardShader;

	bool initRandomTexture(unsigned int Size); //TODO: Add it into the textureManager
	void bindRandomTexture(unsigned int TextureUnit);//TODO: this should be removed once is included in the textureManager
	unsigned int m_textureRandID; // TODO: This will be removed once is included in the textureManager

	//int m_pTextureNum; // TODO: Fix this, use the Number of the Texture*, but not both
	Texture* m_pTexture;
	float m_time;
};

#endif