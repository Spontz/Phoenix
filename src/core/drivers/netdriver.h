// netdriver.h
// Spontz Demogroup

#ifndef NETDRIVER_H
#define NETDRIVER_H

#include "core/drivers/net/dyad.h"

#include <string>

#define NETDRV (&netDriver::GetInstance())

class netDriver {
	
public:
	int		portReceive_;
	int		portSend_;
	
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
	bool	inited_;
	bool	connectedToEditor_;
	dyad_Stream *serv_connect;
	
	char * getParamString(const char *message, int requestedParameter);
	float getParamFloat(const char *message, int requestedParameter);

	// Callbacks
	static void onData_SendResponse(dyad_Event* e);
	static void onAccept(dyad_Event* e);
	static void onListen(dyad_Event* e);
	static void onError(dyad_Event* e);
	static void onConnectToEngine(dyad_Event* e);

};

#endif