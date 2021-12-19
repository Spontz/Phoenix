#pragma once

#include <glm/glm.hpp>

#include "core/input/KeyCodes.h"
#include "core/input/MouseCodes.h"

namespace Phoenix {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
