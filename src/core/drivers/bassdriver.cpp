// bassdriver.cpp
// Spontz Demogroup



#include "main.h"

#include "bassdriver.h"

bassDriver& bassDriver::GetInstance() {
	static bassDriver instance;
	return instance;
}

void bassDriver::init() {
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		LOG->Error("bassDriver: Sound cannot be initialized, error in BASS_Init()");
		return;
	}

	for (int i = 0; i < FFT_BUFFER_SAMPLES; i++) {
		fft[i] = 0.0;
	}

	LOG->Info(LogLevel::HIGH, "BASS library inited");
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

void bassDriver::copyFFTdata(float* fftData, int samples) {
	if (samples != FFT_BUFFER_SAMPLES)
		return;
	else
		memcpy(fft, fftData, FFT_BUFFER_SAMPLES);
}

float* bassDriver::getFFTdata()
{
	return &(fft[0]);
}

float bassDriver::sound_cpu() {
	return BASS_GetCPU();
}

const std::string bassDriver::getVersion()
{
	return BASSVERSIONTEXT;
}
