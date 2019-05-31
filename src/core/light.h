// light.h
// Spontz Demogroup

#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Light
{
public:
	// Camera Attributes
	glm::vec3 Position;		// Position of the light
	glm::vec3 lookAt;		// In case it's a directional light, where is looking at
	glm::vec3 color;		// Color of our light
	glm::mat4 SpaceMatrix;	// READ ONLY: Ligth space matrix, useful for shaders
	float ambientStrenght;	// Strengt of the ambient color (from 0.0 to 1.0)
	float specularStrenght;	// Strengt of the specular color (from 0.0 to 1.0)

	// Constructor with vectors
	Light(glm::vec3 position = glm::vec3(10.0f, 10.0f, 0.0f));
	// Calculate the Space Matrix (useful for ShadowMapping): it retrieves the ortogonal view form the light point of view
	void CalcSpaceMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane);

private:
};


#endif