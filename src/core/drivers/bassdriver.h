// bassdriver.h
// Spontz Demogroup

#pragma once


// hack: get rid of macros
#define BASSDRV (&bassDriver::GetInstance())
#define FFT_BUFFER_SAMPLES 512

// ******************************************************************

class bassDriver {
	
public:
	static bassDriver& GetInstance();
	bassDriver();

private:
	demokernel&	m_demo;
	float		m_fft[FFT_BUFFER_SAMPLES];

public:
	void init();
	void play();
	void update();
	void pause();
	void stop();
	void end();

	void				addFFTdata(float* fftData, int samples);
	float*				getFFTdata();
	int					getFFTSamples();
	float				getCPUload();
	const std::string	getVersion();
};

