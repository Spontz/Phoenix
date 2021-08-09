// LightManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/LightManager.h"

namespace Phoenix {

	// Init vars
	LightManager::LightManager() {
		light.clear();
	}

	LightManager::~LightManager()
	{
		clear();
	}

	// Adds a Light into the queue
	void LightManager::addLight(LightType lightType, glm::vec3 position) {
		std::string lightName = "light[" + std::to_string(this->light.size()) + "]";

		Light* new_light = new Light(lightName, lightType, position);
		light.push_back(new_light);
	}

	void LightManager::initAllLightsColors()
	{
		int lightNum = (int)light.size();
		for (int i = 0; i < lightNum; i++)
			light[i]->initColorValues();
	}

	void LightManager::clear()
	{
		light.clear();
	}

}