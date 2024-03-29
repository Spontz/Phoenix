﻿// Light.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Light.h"
#include "core/resource/resource.h"

namespace Phoenix {

	Light::Light(std::string name, LightType lightType, glm::vec3 position) {
		name = name;
		lightType = lightType;
		position = position;
		direction = glm::vec3(0, 0, 0);	// Looking at center by default

		initColorValues();
	}

	void Light::CalcSpaceMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane) {
		glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near_plane, far_plane); // Switch to ortogonal view
		glm::mat4 lightView = glm::lookAt(position, direction, glm::vec3(0.0, 1.0, 0.0)); // View from the light perspective
		spaceMatrix = lightProjection * lightView;
	}

	void Light::draw(float size) {
		DEMO->m_pRes->m_spShdrObjColor->use();

		DEMO->m_pRes->m_spShdrObjColor->setValue("color", colAmbient);

		glm::mat4 projection = DEMO->m_cameraManager.getActiveProjection();
		glm::mat4 view = DEMO->m_cameraManager.getActiveView();

		DEMO->m_pRes->m_spShdrObjColor->setValue("projection", projection);
		DEMO->m_pRes->m_spShdrObjColor->setValue("view", view);

		// Place the quad onto desired place
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::scale(model, glm::vec3(size, size, size));
		DEMO->m_pRes->m_spShdrObjColor->setValue("model", model);

		DEMO->m_pRes->drawCube();
	}

	void Light::initColorValues()
	{
		// By defalt, light colors and strenght is at zeroNo light
		colAmbient = glm::vec3(0, 0, 0);
		colDiffuse = glm::vec3(0, 0, 0);
		colSpecular = glm::vec3(0, 0, 0);
		ambientStrength = 0;
		specularStrength = 0;

		// Spot and Point Lights: For implementing the attenuation of the light, we need this 3 components
		constant = 0;
		linear = 0;
		quadratic = 0;
	}
}