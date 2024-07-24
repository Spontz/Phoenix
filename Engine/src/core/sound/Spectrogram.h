// Sound.h
// Spontz Demogroup

#pragma once 

#include "main.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <string_view>

namespace Phoenix {

	constexpr size_t SPECTOGRAM_SAMPLES = 32;

	class Spectogram {

	public:
		Spectogram();
		virtual ~Spectogram();

	public:


	private:
		float		m_value[SPECTOGRAM_SAMPLES];

	public:
		float*		getSpectogramData();
		int			getSpectogramSamples();

	};
}