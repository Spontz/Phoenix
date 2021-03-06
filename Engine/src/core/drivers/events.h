// Events.h
// Spontz Demogroup

#pragma once
namespace Phoenix {
	// definition of engine control keys
	#define KEY_EXIT			GLFW_KEY_ESCAPE
	#define KEY_TIME			GLFW_KEY_ENTER

	#define KEY_SHOWLOG			GLFW_KEY_BACKSPACE
	#define KEY_SHOWVERSION		GLFW_KEY_0
	#define KEY_SHOWINFO		GLFW_KEY_1
	#define KEY_SHOWFPSHIST		GLFW_KEY_2
	#define KEY_SHOWFBO			GLFW_KEY_3
	#define KEY_CHANGEATTACH	GLFW_KEY_4
	#define KEY_SHOWSECTIONINFO	GLFW_KEY_5
	#define KEY_SHOWSOUND		GLFW_KEY_6
	#define KEY_SHOWGRIDPANEL	GLFW_KEY_7
	#define KEY_SHOWHELP		GLFW_KEY_9

	#define KEY_PLAY_PAUSE		GLFW_KEY_F1
	#define KEY_REWIND			GLFW_KEY_F2
	#define KEY_FASTFORWARD		GLFW_KEY_F3
	#define KEY_RESTART			GLFW_KEY_F4

	// definition of camera control keys
	#define KEY_CAM_CAPTURE			GLFW_KEY_SPACE
	#define KEY_CAM_FORWARD			GLFW_KEY_W
	#define KEY_CAM_BACKWARD		GLFW_KEY_S
	#define KEY_CAM_STRAFE_LEFT		GLFW_KEY_A
	#define KEY_CAM_STRAFE_RIGHT	GLFW_KEY_D
	#define KEY_CAM_ROLL_LEFT		GLFW_KEY_Q
	#define KEY_CAM_ROLL_RIGHT		GLFW_KEY_E
	#define KEY_CAM_RESET			GLFW_KEY_R
	#define KEY_CAM_MULTIPLIER		GLFW_KEY_PAGE_UP
	#define KEY_CAM_DIVIDER			GLFW_KEY_PAGE_DOWN
}