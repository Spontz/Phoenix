// bassdriver.h
// Spontz Demogroup

#ifndef BASSDRIVER_H
#define BASSDRIVER_H


// hack: get rid of macros
#define BASSDRV (&bassDriver::GetInstance())


// ******************************************************************

class bassDriver {
	
public:
	static bassDriver& GetInstance();

private:
	bassDriver() = default;

public:
	void init();
	void play();
	void update();
	void pause();
	void stop();
	void end();

	float sound_cpu();
};

#endif
