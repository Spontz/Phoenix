// netdriver.h
// Spontz Demogroup

#ifndef NETDRIVER_H
#define NETDRIVER_H

#define NETDRV (&netDriver::GetInstance())

class netDriver {
	
public:
	int		port;
	int		port_send;
	bool	inited;
	bool	connectedToEditor;
	std::string	messageToSend;
	
	static netDriver& GetInstance();

	netDriver();
	void init();
	void connectToEditor();
	void update();
	virtual ~netDriver();

	const char *getVersion();
	char * processMessage(const char *message);
	void sendMessage(std::string message);

private:
	char * getParamString(const char *message, int requestedParameter);
	float getParamFloat(const char *message, int requestedParameter);
};

#endif