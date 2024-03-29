// BassDriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/BassDriver.h"

namespace Phoenix {

	BassDriver& BassDriver::getInstance() {
		static BassDriver instance;
		return instance;
	}

	BassDriver::BassDriver()
		:
		m_demo(*DEMO),
		m_spectrum{ 0.0 }
	{
	}

	bool BassDriver::init() {
		return BASS_Init(-1, 44100, 0, 0, NULL);
	}

	void BassDriver::play() {
		BASS_Start();
	}

	void BassDriver::update() {
		BASS_Update(200);

		// Clear all beat values and internal fft data
		memset(m_demo.m_fBeat, 0, MAX_BEATS * sizeof(float));

		if (m_demo.m_debug)
			memset(m_spectrum, 0, SPECTRUM_SAMPLES * sizeof(float));

	}

	void BassDriver::pause() {
		BASS_Pause();
	}

	void BassDriver::stop() {
		BASS_Stop();
	}

	void BassDriver::end() {
		BASS_Free();
	}

	void BassDriver::addFFTdata(float* fftData, size_t samples) {
		assert(samples > 0);

		// FFT values are just for debug purposes, so we just add the values if we are in debug mode
		if (m_demo.m_debug)
		{
			// Populate spectrum bars based on FFT data received
			size_t b0 = 0;
			float pow_num = glm::log2(static_cast<float>(samples));
			for (size_t i = 0; i < SPECTRUM_SAMPLES; i++) {
				float peak = 0;

				// determine size of the bin
				size_t b1 = static_cast<size_t>(
					glm::pow(2.0f, static_cast<float>(i) * pow_num / static_cast<float>(SPECTRUM_SAMPLES - 1))
					);

				// upper bound on bin size
				if (b1 >= samples)
					b1 = samples - 1;

				// make sure atleast one bin is used
				if (b1 <= b0)
					b1 = b0 + 1;

				// loop over every bin
				for (; b0 < b1; b0++) {
					if (peak < fftData[1 + b0])
						peak = fftData[1 + b0];
				}

				// write each column to file
				m_spectrum[i] = glm::sqrt(peak);
			}
		}

	}

	float* BassDriver::getSpectrumData()
	{
		return &(m_spectrum[0]);
	}

	int BassDriver::getSpectrumSamples()
	{
		return SPECTRUM_SAMPLES;
	}

	float BassDriver::getCPUload() {
		return BASS_GetCPU();
	}

	const std::string BassDriver::getVersion()
	{
		return BASSVERSIONTEXT;
	}
}
