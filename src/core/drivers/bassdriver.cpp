// bassdriver.cpp
// Spontz Demogroup

#include <bass/bass.h>
#include "main.h"


// Initialize the bassDriver main pointer to NULL
bassDriver* bassDriver::m_pThis = NULL;


bassDriver * bassDriver::getInstance() {
	if (m_pThis == NULL)
		m_pThis = new bassDriver();
	return m_pThis;
}

bassDriver::bassDriver() {

}

void bassDriver::init() {
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		LOG->Error("bassDriver: Sound cannot be initialized, error in BASS_Init()");
		return;
	}
	LOG->Info(LOG_HIGH, "BASS library inited");
	LOG->Info(LOG_MED, "BASS library version is: %s", BASSVERSIONTEXT);
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

int bassDriver::sound_channels() {
	// TODO: To be implemented in BASS, as it was in FMOD
	//return FSOUND_GetChannelsPlaying();
	return 0;
}