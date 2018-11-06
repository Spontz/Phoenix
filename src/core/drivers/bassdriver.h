// bassdriver.h
// Spontz Demogroup

#ifndef BASSDRIVER_H
#define BASSDRIVER_H


//#include <bass/bass.h>

#define BASSDRV bassDriver::getInstance()

using namespace std;

// ******************************************************************

class bassDriver {
	
public:
	
	static bassDriver* getInstance();
	bassDriver();
	void init();
	void play();
	void update();
	void pause();
	void stop();
	void end();

	float sound_cpu();
	int sound_channels();

private:

	static bassDriver* m_pThis;
};

#endif