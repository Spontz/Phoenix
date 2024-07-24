// Sound.cpp
// Spontz Demogroup

#include "main.h"
#include "core/sound/Sound.h"
#include "Spectrogram.h"

namespace Phoenix {

	Phoenix::Spectogram::Spectogram()
	{
		memset(m_value, 0, sizeof(float)* SPECTOGRAM_SAMPLES);
	}
	Spectogram::~Spectogram()
	{
	}
	float* Spectogram::getSpectogramData()
	{
		return &(m_value[0]);
	}
	int Spectogram::getSpectogramSamples()
	{
		return SPECTOGRAM_SAMPLES;
	}
}