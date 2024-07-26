// Sound.cpp
// Spontz Demogroup

#include "main.h"
#include "core/sound/Sound.h"
#include "Spectrogram.h"

namespace Phoenix {

	Phoenix::Spectrogram::Spectrogram()
	{
		memset(m_value, 0, sizeof(float)* SPECTROGRAM_SAMPLES);
	}
	Spectrogram::~Spectrogram()
	{
	}
	void Spectrogram::fill(float* fftData, uint32_t samples)
	{
		// We should call this function only if we are in debug mode
		// Populate spectrum bars based on FFT data received
		uint32_t b0 = 0;
		float pow_num = glm::log2(static_cast<float>(samples));
		for (uint32_t i = 0; i < SPECTROGRAM_SAMPLES; i++) {
			float peak = 0;

			// determine size of the bin
			uint32_t b1 = static_cast<uint32_t>(glm::pow(2.0f, static_cast<float>(i) * pow_num / static_cast<float>(SPECTROGRAM_SAMPLES - 1)));

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
			m_value[i] = glm::sqrt(peak);
		}
	}
	float* Spectrogram::getSpectogramData()
	{
		return &(m_value[0]);
	}
	int Spectrogram::getSpectogramSamples()
	{
		return SPECTROGRAM_SAMPLES;
	}
}