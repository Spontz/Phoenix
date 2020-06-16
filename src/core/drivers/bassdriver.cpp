// bassdriver.cpp
// Spontz Demogroup

#include <bass.h>

#include "main.h"

bassDriver& bassDriver::GetInstance() {
	static bassDriver instance;
	return instance;
}

void bassDriver::init() {
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		LOG->Error("bassDriver: Sound cannot be initialized, error in BASS_Init()");
		return;
	}
	LOG->Info(LogLevel::HIGH, "BASS library inited");
	LOG->Info(LogLevel::MED, "BASS library version is: %s", BASSVERSIONTEXT);
}

void bassDriver::play() {
	BASS_Start();
}

void bassDriver::update() {
	BASS_Update(200);
}

void bassDriver::pause() {
	BASS_Pause();
}

void bassDriver::stop() {
	BASS_Stop();
}

void bassDriver::end() {
	BASS_Free();
}

float bassDriver::sound_cpu() {
	return BASS_GetCPU();
}