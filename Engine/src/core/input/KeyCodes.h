#pragma once
#include <stdint.h>

namespace Phoenix {

	using KeyCode = uint16_t;
	
	namespace Key
	{
		enum : KeyCode
		{
			EXIT				= GLFW_KEY_ESCAPE,
			TIME				= GLFW_KEY_ENTER,
			
			SHOWLOG				= GLFW_KEY_BACKSPACE,
			SHOWVERSION			= GLFW_KEY_0,
			SHOWINFO			= GLFW_KEY_1,
			SHOWFPSHIST			= GLFW_KEY_2,
			SHOWFBO				= GLFW_KEY_3,
			CHANGEATTACH		= GLFW_KEY_4,
			SHOWSECTIONINFO		= GLFW_KEY_5,
			SHOWSOUND			= GLFW_KEY_6,
			SHOWGRIDPANEL		= GLFW_KEY_7,
			SHOWHELP			= GLFW_KEY_9,

			PLAY_PAUSE			= GLFW_KEY_F1,
			REWIND				= GLFW_KEY_F2,
			FASTFORWARD			= GLFW_KEY_F3,
			RESTART				= GLFW_KEY_F4,

			// definition of camera control keys
			CAM_CAPTURE			= GLFW_KEY_SPACE,
			CAM_FORWARD			= GLFW_KEY_W,
			CAM_BACKWARD		= GLFW_KEY_S,
			CAM_STRAFE_LEFT		= GLFW_KEY_A,
			CAM_STRAFE_RIGHT	= GLFW_KEY_D,
			CAM_ROLL_LEFT		= GLFW_KEY_Q,
			CAM_ROLL_RIGHT		= GLFW_KEY_E,
			CAM_RESET			= GLFW_KEY_R,
			CAM_MULTIPLIER		= GLFW_KEY_PAGE_UP,
			CAM_DIVIDER			= GLFW_KEY_PAGE_DOWN
		};
	}
}