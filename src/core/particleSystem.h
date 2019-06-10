// particleSystem.h
// Spontz Demogroup

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>


class ParticleSystem
{
public:
	ParticleSystem();

	~ParticleSystem();

	bool InitParticleSystem(const glm::vec3 &Pos);

	void Render(int DeltaTimeMillis, const glm::mat4 &VP, const glm::vec3 &CameraPos);

private:

	void UpdateParticles(int DeltaTimeMillis);
	void RenderParticles(const glm::mat4 &VP, const glm::vec3 &CameraPos);

	// Billboard Technique
	bool initBillboard();		// For drawing the quads using geometry shaders
	bool initParticleSystem();	// For drawing the particles using geometry shaders

	bool m_isFirst;
	unsigned int m_currVB;
	unsigned int m_currTFB;
	GLuint m_particleBuffer[2];
	GLuint m_transformFeedback[2];
	//PSUpdateTechnique m_updateTechnique;
	//BillboardTechnique m_billboardTechnique;
	int particleSystemShader;
	int billboardShader;

	//RandomTexture m_randomTexture;
	bool initRandomTexture(unsigned int Size); //TODO: Add it into the textureManager
	void bindRandomTexture(unsigned int TextureUnit);//TODO: tshi should be removed once is included in the textureManager
	unsigned int m_textureRandID; // TODO: This will be removed once is included in the textureManager

	int m_pTextureNum; // TODO: Fix this, use the Number of the Texture*, but not both
	Texture* m_pTexture;
	int m_time;	// TODO: Time should be a float with the value
};

#endif