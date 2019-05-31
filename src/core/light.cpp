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

void Light::CalcSpaceMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane)
{
	//glm::mat4 lightProjection = glm::perspective(glm::radians(DEMO->camera->Zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);// glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);				// Switch to ortogonal view TODO: Review this numbers
	//glm::mat4 lightView = DEMO->camera->GetViewMatrix(); //glm::lookAt(DEMO->light->Position, DEMO->light->lookAt, glm::vec3(0.0, 1.0, 0.0));	// View from the light perspective
	glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near_plane, far_plane);				// Switch to ortogonal view
	glm::mat4 lightView = glm::lookAt(DEMO->light->Position, DEMO->light->lookAt, glm::vec3(0.0, 1.0, 0.0));// View from the light perspective
	this->SpaceMatrix = lightProjection * lightView;
}
