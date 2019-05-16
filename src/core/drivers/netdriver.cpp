// netdriver.cpp
// Spontz Demogroup


#include "main.h"
#include "core/drivers/netdriver.h"
#include "core/drivers/net/dyad.h"

using namespace std;



// ***********************************

#define SPZ_STL(a) _##a

char * getParamString(char *message, int requestedParameter) {
	/*********************************************************************************************
	* Returns the requested parameter from the passed message (first parameter is 1) as a string *
	**********************************************************************************************/

	char *theParameter;
	int counter = 1;

	theParameter = SPZ_STL(strdup)(message);

	for (theParameter = strtok(theParameter, "::");
		counter < requestedParameter;
		theParameter = strtok(NULL, "::"))
		counter++;

	return theParameter;
}

void SendResponse(dyad_Event *e, char *message) {
	char theResponse[4096];
	char *identifier, *type, *action;

	identifier = getParamString(message, 1);
	type = getParamString(message, 2);
	action = getParamString(message, 3);
	char theResult[] = "OK";
	char theInformation[] = "";

	sprintf((char *)theResponse, "%s::%s::%f::%d::%f::%s", identifier, theResult, DEMO->fps, DEMO->state, DEMO->runTime, theInformation);

	dyad_write(e->stream, theResponse, (int)strlen(theResponse));
	LOG->Info(LOG_LOW, "Sending response: %s\n", theResponse);
	dyad_end(e->stream);
}

// ***********************************

void onData(dyad_Event *e) {
	printf("Data received onData!\n Data: %s\n Size: %d\n", e->data, e->size);
	LOG->Info(LOG_LOW, "Message received: %s\n Bytes Received: %d\n", e->data, e->size);
	SendResponse(e, e->data);
}

void onAccept(dyad_Event *e) {
	dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
}

void onListen(dyad_Event *e) {
	LOG->Info(LOG_MED, "Network listener started in port: %d", dyad_getPort(e->stream));
}

void onError(dyad_Event *e) {
	LOG->Error("Network server error: %s", e->msg);
}


// Initialize the netDriver main pointer to NULL
netDriver* netDriver::m_pThis = NULL;

netDriver * netDriver::getInstance() {
	if (m_pThis == NULL)
		m_pThis = new netDriver();
	return m_pThis;
}

netDriver::netDriver()
{
	port = 28000;
	inited = false;
}

void netDriver::init(int port)
{
	this->port = port;
	dyad_init();

	dyad_Stream *serv = dyad_newStream();
	//dyad_setNoDelay(serv, 1);
	dyad_setUpdateTimeout(0);
	dyad_addListener(serv, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, onAccept, NULL);
	dyad_addListener(serv, DYAD_EVENT_LISTEN, onListen, NULL);
	dyad_listenEx(serv, "0.0.0.0", port, 511);
	inited = true;
}

netDriver::~netDriver()
{
	//TODO: Implement to close network
}
