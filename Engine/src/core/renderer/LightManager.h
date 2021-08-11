// LightManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Light.h"

namespace Phoenix{

	class LightManager final {

	public:
		LightManager();
		~LightManager();

	public:
		void addLight(LightType lightType, glm::vec3 position = { 10, 10, 0 });
		void initAllLightsColors();
		void clear();

	public:
		std::vector<Light*> light;	// light list
	};

}
