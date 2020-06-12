// netdriver.h
// Spontz Demogroup

#ifndef NETDRIVER_H
#define NETDRIVER_H

#define NETDRV netDriver::getInstance()

class netDriver {
	
public:
	int		port;
	int		port_send;
	bool	inited;
	bool	connectedToEditor;
	std::string	messageToSend;
	
	static netDriver* getInstance();

	netDriver();
	void init();
	void connectToEditor();
	void update();
	virtual ~netDriver();

	const char *getVersion();
	char * processMessage(char *message);
	void sendMessage(std::string message);

private:
	char * getParamString(char *message, int requestedParameter);
	float getParamFloat(char *message, int requestedParameter);
	static netDriver* m_pThis;
};

#endif