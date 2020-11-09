// BassDriver.h
// Spontz Demogroup

#pragma once


// hack: get rid of macros
#define BASSDRV (&BassDriver::GetInstance())
#define SPECTRUM_SAMPLES 32

// ******************************************************************

class BassDriver {
	
public:
	static BassDriver& GetInstance();
	BassDriver();

private:
	demokernel&	m_demo;
	float		m_spectrum[SPECTRUM_SAMPLES];

public:
	void init();
	void play();
	void update();
	void pause();
	void stop();
	void end();

	void				addFFTdata(float* fftData, int samples);
	float*				getSpectrumData();
	int					getSpectrumSamples();
	float				getCPUload();
	const std::string	getVersion();
};

