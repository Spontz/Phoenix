// bassdriver.cpp
// Spontz Demogroup



#include "main.h"
#include "bassdriver.h"

bassDriver& bassDriver::GetInstance() {
	static bassDriver instance;
	return instance;
}

bassDriver::bassDriver()
	:
	m_demo(demokernel::GetInstance()),
	m_fft{0.0}
{
}

void bassDriver::init() {
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		Logger::error("bassDriver: Sound cannot be initialized, error in BASS_Init()");
		return;
	}

	Logger::info(LogLevel::high, "BASS library inited");
}

void bassDriver::play() {
	BASS_Start();
}

void bassDriver::update() {
	BASS_Update(200);
	
	// Clear general beat value and internal fft data
	m_demo.m_beat = 0;
	
	if (m_demo.m_debug)
		memset(m_fft, 0, FFT_BUFFER_SAMPLES *sizeof(float));
	
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
	// FFT values are just for debug purposes, so we just add the values if we are in debug mode
	if (m_demo.m_debug)
	{
		if (samples != FFT_BUFFER_SAMPLES)
			return;
		// Add FFT info, although it not really used, just or "plot" values
		for (int i = 0; i < FFT_BUFFER_SAMPLES; i++)
			m_fft[i] += fftData[i];
	}
	
}

float* bassDriver::getFFTdata()
{
	return &(m_fft[0]);
}

int bassDriver::getFFTSamples()
{
	return FFT_BUFFER_SAMPLES;
}

float bassDriver::getCPUload() {
	return BASS_GetCPU();
}

const std::string bassDriver::getVersion()
{
	return BASSVERSIONTEXT;
}
