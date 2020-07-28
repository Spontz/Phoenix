// bassdriver.h
// Spontz Demogroup

#ifndef BASSDRIVER_H
#define BASSDRIVER_H


// hack: get rid of macros
#define BASSDRV (&bassDriver::GetInstance())
#define FFT_BUFFER_SAMPLES 512

// ******************************************************************

class bassDriver {
	
public:
	static bassDriver& GetInstance();

private:
	bassDriver() = default;
	float fft[FFT_BUFFER_SAMPLES];

public:
	void init();
	void play();
	void update();
	void pause();
	void stop();
	void end();

	void copyFFTdata(float* fftData, int samples);
	float* getFFTdata();
	float sound_cpu();
	const std::string getVersion();
};

#endif
