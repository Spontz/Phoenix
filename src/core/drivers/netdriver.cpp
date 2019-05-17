// netdriver.cpp
// Spontz Demogroup


#include "main.h"
#include "core/drivers/netdriver.h"
#include "core/drivers/net/dyad.h"

using namespace std;



// ***********************************

#define SPZ_STL(a) _##a

// Returns the requested parameter from the passed message (first parameter is 1) as a string
char * getParamString(char *message, int requestedParameter) {
	char *theParameter;
	int counter = 1;

	theParameter = SPZ_STL(strdup)(message);

	for (theParameter = strtok(theParameter, "::");
		counter < requestedParameter;
		theParameter = strtok(NULL, "::"))
		counter++;

	return theParameter;
}

// Returns the requested parameter from the passed message (first parameter is 1) as a floating point number
float getParamFloat(char *message, int requestedParameter) {
	float theFloatResult;
	char *theStringResult;

	theStringResult = getParamString(message, requestedParameter);

	// Search for the parameter and transform it in a float
	sscanf(theStringResult, "%f", &theFloatResult);

	// Return the result
	return theFloatResult;
}

/*
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
*/
// ***********************************

void onData(dyad_Event *e) {
	char *theResponse;
	
	theResponse = NETDRV->processMessage(e->data);
	// Send the response and close the connection
	dyad_write(e->stream, theResponse, (int)strlen(theResponse));
	dyad_end(e->stream);
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
	//dyad_setNoDelay(serv, 1); // Disable Nagle's algorithm
	dyad_setUpdateTimeout(0);	// Disable waiting
	dyad_addListener(serv, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, onAccept, NULL);
	dyad_addListener(serv, DYAD_EVENT_LISTEN, onListen, NULL);
	dyad_listenEx(serv, "0.0.0.0", port, 511);
	inited = true;
}

void netDriver::update()
{
	dyad_update();
}

netDriver::~netDriver()
{
	dyad_shutdown();
}

const char * netDriver::getVersion()
{
	return dyad_getVersion();
}

char * netDriver::processMessage(char * message)
{
	// Incoming information
	char *identifier, *type, *action;

	// Outcoming information
	static char theResponse[4096];
	char *theResult, *theInformation;

	identifier = getParamString(message, 1);
	type = getParamString(message, 2);
	action = getParamString(message, 3);

	theResult = "OK";
	theInformation = (char *) calloc(1024, sizeof(char));

	LOG->Info(LOG_LOW, "Message received: [identifier: %s] [type: %s] [action: %s]", identifier, type, action);

	// ***********************
	// * COMMANDS PROCESSING *
	// ***********************

	if (strcmp(type, "command") == 0) {
		if		(strcmp(action, "pause") == 0)			{ DEMO->pauseDemo();								theResult = "OK"; }
		else if (strcmp(action, "play") == 0)			{ DEMO->playDemo();									theResult = "OK"; }
		else if (strcmp(action, "restart") == 0)		{ DEMO->restartDemo();								theResult = "OK"; }
		else if (strcmp(action, "startTime") == 0)		{ DEMO->setStartTime(getParamFloat(message, 4));	theResult = "OK"; }
		else if (strcmp(action, "currentTime") == 0)	{ DEMO->setCurrentTime(getParamFloat(message, 4));	theResult = "OK"; }
		else if (strcmp(action, "endTime") == 0)		{ DEMO->setEndTime(getParamFloat(message, 4));		theResult = "OK"; }
		else if (strcmp(action, "ping") == 0)			{ theResult = "OK"; }
		else {
			theResult = "NOK";
			sprintf((char *)theInformation, "Unknown command (%s)", message);
		}

		// ***********************
		// * SECTIONS PROCESSING *
		// ***********************
	}
	else if (strcmp(type, "section") == 0) {
		if (strcmp(action, "new") == 0)	{
			char *data = getParamString(message, 4);
			int res = DEMO->load_scriptFromNetwork(data);
			if (res) {
				theResult = "OK";
			}
			else {
				theResult = "NOK";
				sprintf((char *)theInformation, "Section load failed");
			}
		}
		
		else if (strcmp(action, "toggle") == 0) { theResult = "OK"; }
		else if (strcmp(action, "delete") == 0) { theResult = "OK"; }
		else if (strcmp(action, "update") == 0) { theResult = "OK"; }
		else if (strcmp(action, "setStartTime") == 0) { theResult = "OK"; }
		else if (strcmp(action, "setEndTime") == 0) { theResult = "OK"; }
		else if (strcmp(action, "setLayer") == 0) { theResult = "OK"; }
		/*
		else if (strcmp(action, "toggle") == 0) { dkernel_toggleSection(getParamString(message, 4)); theResult = "OK"; }
		else if (strcmp(action, "delete") == 0) { dkernel_deleteSection(getParamString(message, 4)); theResult = "OK"; }
		else if (strcmp(action, "update") == 0) { dkernel_updateSection(getParamString(message, 4), getParamString(message, 5)); theResult = "OK"; }
		else if (strcmp(action, "setStartTime") == 0) { dkernel_setSectionsStartTime(getParamString(message, 4), getParamString(message, 5)); theResult = "OK"; }
		else if (strcmp(action, "setEndTime") == 0) { dkernel_setSectionsEndTime(getParamString(message, 4), getParamString(message, 5)); theResult = "OK"; }
		else if (strcmp(action, "setLayer") == 0) { dkernel_setSectionsLayer(getParamString(message, 4), getParamString(message, 5)); theResult = "OK"; }
		*/
		else {
			theResult = "NOK";
			sprintf((char *)theInformation, "Unknown section (%s)", message);
		}
	}

	// Check for non-processed messages (If the result has not been set, the message has not been processed)
	if (strcmp(theResult, "") == 0) {
		theResult = "NOK";
		sprintf((char *)theInformation, "Unknown message (%s)", message);
	}

	// Create the response
	sprintf((char *)theResponse, "%s::%s::%f::%d::%f::%s", identifier, theResult, DEMO->fps, DEMO->state, DEMO->runTime, (char *)theInformation);
	LOG->Info(LOG_LOW, "Sending response: [%s]", theResponse);
		
	// Free memory
	free(theInformation);

	// and return the response (will be freed later)
	return theResponse;//return (char *)theResponse;
}
