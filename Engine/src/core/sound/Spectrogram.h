// Sound.h
// Spontz Demogroup

#pragma once 

#include "main.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <string_view>

namespace Phoenix {

	constexpr uint32_t SPECTROGRAM_SAMPLES = 32;

	class Spectrogram {

	public:
		Spectrogram();
		virtual ~Spectrogram();

		void fill(float* fftData, uint32_t samples); // Fill the spectrogram with the FFT data

	public:


	private:
		float		m_value[SPECTROGRAM_SAMPLES];

	public:
		float*		getSpectogramData();
		int			getSpectogramSamples();

	};
}