// light.cpp
// Spontz Demogroup

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "main.h"
#include "core/light.h"

using namespace std;

Light::Light(string name, LightType lightType, glm::vec3 position)
{
	this->name = name;
	this->lightType = lightType;
	this->position = position;

	// Colors
	this->colAmbient = glm::vec3(1, 0, 0);	// Red by default

	this->direction = glm::vec3(0, 0, 0);	// Looking at center by default
	this->ambientStrength = 0.2f;
	this->specularStrength = 0.8f;
}

void Light::CalcSpaceMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane)
{
	glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near_plane, far_plane);				// Switch to ortogonal view
	glm::mat4 lightView = glm::lookAt(this->position, this->direction, glm::vec3(0.0, 1.0, 0.0));// View from the light perspective
	this->spaceMatrix = lightProjection * lightView;
}

void Light::draw(float size)
{
	Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_ObjColor];
	my_shad->use();

	my_shad->setValue("color", this->colAmbient);

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), GLDRV->AspectRatio, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shad->setValue("projection", projection);
	my_shad->setValue("view", view);

	// Place the quad onto desired place
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->position);
	model = glm::scale(model, glm::vec3(size, size, size));
	my_shad->setValue("model", model);

	RES->Draw_Cube();
}
