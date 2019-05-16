// netdriver.h
// Spontz Demogroup

#ifndef NETDRIVER_H
#define NETDRIVER_H

#define NETDRV netDriver::getInstance()

using namespace std;

class netDriver {
	
public:
	int		port;
	bool	inited;

	static netDriver* getInstance();


	netDriver();
	void init(int port);
	virtual ~netDriver();

	
private:
	static netDriver* m_pThis;

};

#endif