// light.cpp
// Spontz Demogroup

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "main.h"
#include "core/light.h"

using namespace std;

Light::Light(glm::vec3 position)
{
	this->Position = position;
	this->color = glm::vec3(1, 0, 0);	// Red by default
	this->lookAt = glm::vec3(0, 0, 0);	// Looking at center by default
	this->ambientStrenght = 0.2f;
	this->specularStrenght = 0.8f;
}
