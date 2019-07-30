// netdriver.h
// Spontz Demogroup

#ifndef NETDRIVER_H
#define NETDRIVER_H

#define NETDRV netDriver::getInstance()

using namespace std;

class netDriver {
	
public:
	int		port;
	int		port_send;
	bool	inited;
	string	messageToSend;

	static netDriver* getInstance();

	netDriver();
	void init(int port);
	void update();
	virtual ~netDriver();

	const char *getVersion();
	char * processMessage(char *message);
	void sendMessage(string message);

private:

	char * getParamString(char *message, int requestedParameter);
	float getParamFloat(char *message, int requestedParameter);
	static netDriver* m_pThis;
};

#endif