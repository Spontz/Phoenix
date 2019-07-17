// lightmanager.cpp
// Spontz Demogroup

#include "main.h"
#include "lightmanager.h"


// Init vars
LightManager::LightManager() {
	light.clear();
}

LightManager::~LightManager()
{
	light.clear();
}

// Adds a Light into the queue, returns the Number of the texture added
void LightManager::addLight(LightType lightType, glm::vec3 position) {
	string lightName = "light[" + std::to_string(this->light.size()) + "]";

	Light *new_light = new Light(lightName, lightType, position);
	light.push_back(new_light);
}

