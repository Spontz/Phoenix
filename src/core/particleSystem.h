// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

//#include <glad/glad.h>
//#include <glm/glm.hpp>

#include "main.h"
#include "core/shadervars.h"

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
	ParticleSystem(std::string shaderPath, unsigned int	numMaxParticles, unsigned int numEmitters, float emissionTime, float particleLifeTime);
	~ParticleSystem();

	bool InitParticleSystem(Section* sec, const std::vector<Particle> emitter, std::vector<std::string>	billboardShaderVars);
	void Render(float deltaTime, const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos);
	void UpdateEmitters(float deltaTime);

	glm::vec3 force; // Force to be applied globally

private:

	void UpdateParticles(float deltaTime);
	void RenderParticles(const glm::mat4 &VP, const glm::mat4 &model, const glm::vec3 &CameraPos);

	bool initShaderBillboard();		// For drawing the quads using geometry shaders
	bool initShaderParticleSystem();// For drawing the particles using geometry shaders

	std::string		shaderPath;
	std::string		pathBillboardVS, pathBillboardGS, pathBillboardFS;
	ShaderVars		*varsBillboard;	// Billboard Shader variables

	std::string		pathUpdateVS, pathUpdateGS, pathUpdateFS;
	unsigned int	numMaxParticles;	// Number of maximum particles
	unsigned int	numEmitters;		// Number of emmiters
	float			emissionTime;
	float			particleLifeTime;

	bool m_isFirst;
	unsigned int m_currVB;			// Variable that indicates the current VB (The value is 0 or 1)
	unsigned int m_currTFB;			// Variable that indicates the current TFB (The value is 0 or 1)
	GLuint m_VAO;					// Vertex Array Object buffer
	GLuint m_particleBuffer[2];		// Vertex Buffer: input
	GLuint m_transformFeedback[2];	// Transform feedback buffers: output
	Particle* m_emitterData;		// Emitter info (for updating emitters positions)
	int particleSystemShader;
	int billboardShader;

	bool initRandomTexture(unsigned int Size); //TODO: Add it into the textureManager
	void bindRandomTexture(GLuint TexUnit = 0);//TODO: this should be removed once is included in the textureManager
	unsigned int m_textureRandID; // TODO: This will be removed once is included in the textureManager

	float m_time;
};

#endif