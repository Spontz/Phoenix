// light.h
// Spontz Demogroup

#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


enum class LightType : unsigned int {
	Undefined = 0,
	SpotLight = 1,			// A spotlight is a light source that is located somewhere in the environment that, instead of shooting light rays in all directions, only shoots them in a specific direction
	PointLight = 2			// A point light is a light source with a given position somewhere in a world that illuminates in all directions where the light rays fade out over distance
};

class Light
{
public:
	std::string	name;				// Light name, used for shaders
	LightType	lightType;			// Light type
	
	glm::vec3	position;			// Spot and Point Lights: Position of the light
	glm::vec3	direction;			// Spot Light: The direction where is looking at
	
	// Light colors
	glm::vec3	colAmbient;			// Ambient color of our light
	glm::vec3	colDiffuse;			// Diffuse color of our light
	glm::vec3	colSpecular;		// Specular color of our light
	
	glm::mat4	spaceMatrix;		// READ ONLY: Ligth space matrix, useful for shaders
	float		ambientStrength;	// Strength of the ambient color (from 0.0 to 1.0)
	float		specularStrength;	// Strength of the specular color (from 0.0 to 1.0)

	float		constant;			// Spot and Point Lights: For implementing the attenuation of the light, we need this 3 components
	float		linear;
	float		quadratic;

	// Constructor with vectors
	Light(std::string name, LightType lightType, glm::vec3 position);
	// Calculate the Space Matrix (useful for ShadowMapping): it retrieves the ortogonal view form the light point of view
	void CalcSpaceMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane);

	// For debugging: Draws a qube with the light position
	void draw(float size);
	// Init light Color Values
	void initColorValues();

private:
};


#endif