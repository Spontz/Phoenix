// BassDriver.h
// Spontz Demogroup

#pragma once

namespace Phoenix {

	// hack: get rid of macros
#define BASSDRV (&BassDriver::getInstance())
	constexpr size_t SPECTRUM_SAMPLES = 32;

	class BassDriver final {

	public:
		BassDriver();

	public:
		static BassDriver& getInstance();

	private:
		DemoKernel& m_demo;
		float		m_spectrum[SPECTRUM_SAMPLES];

	public:
		bool init();
		void play();
		void update();
		void pause();
		void stop();
		void end();

		void				addFFTdata(float* fftData, size_t samples);
		float*				getSpectrumData();
		int					getSpectrumSamples();
		float				getCPUload();
		const std::string	getVersion();
	};
}