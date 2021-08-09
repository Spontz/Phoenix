// LightManager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/renderer/Light.h"

namespace Phoenix {

	class LightManager {
	public:
		std::vector<Light*> light;	// light list

		LightManager();
		virtual ~LightManager();
		void addLight(LightType lightType, glm::vec3 position = glm::vec3(10.0f, 10.0f, 0.0f));
		void initAllLightsColors();

		void clear();

	private:
	};
}
