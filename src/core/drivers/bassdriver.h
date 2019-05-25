// bassdriver.h
// Spontz Demogroup

#ifndef BASSDRIVER_H
#define BASSDRIVER_H


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

private:

	static bassDriver* m_pThis;
};

#endif