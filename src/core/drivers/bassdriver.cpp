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
		Logger::error("bassDriver: Sound cannot be initialized, error in BASS_Init()");
		return;
	}

	for (int i = 0; i < FFT_BUFFER_SAMPLES; i++) {
		fft[i] = 0.0;
	}

	Logger::info(LogLevel::high, "BASS library inited");
}

void bassDriver::play() {
	BASS_Start();
}

void bassDriver::update() {
	BASS_Update(200);
	
	// Clear general beat value and internal fft data
	DEMO->m_beat = 0;
	memset(fft, 0, FFT_BUFFER_SAMPLES * sizeof(float));
	
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

void bassDriver::addFFTdata(float* fftData, int samples) {
	if (samples != FFT_BUFFER_SAMPLES)
		return;
	// Add FFT info, although it not really used, just or "plot" values
	for (int i = 0; i < FFT_BUFFER_SAMPLES; i++)
		fft[i] += fftData[i];
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
