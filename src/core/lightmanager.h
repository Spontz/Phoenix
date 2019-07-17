// lightmanager.h
// Spontz Demogroup

#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H
#include <string>
#include <vector>
#include "core/light.h"

using namespace std;

class LightManager {
public:
	std::vector<Light*> light;	// light list

	LightManager();
	virtual ~LightManager();
	void LightManager::addLight(LightType lightType, glm::vec3 position = glm::vec3(10.0f, 10.0f, 0.0f));

private:


	
};

#endif