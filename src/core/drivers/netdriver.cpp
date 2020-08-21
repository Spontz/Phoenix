// netdriver.cpp
// Spontz Demogroup


#include "core/drivers/net/dyad.h"
#include "main.h"

#define DELIMITER '\x1f'
#define DELIMITER_SEND "\x1f"

// ***********************************

#define SPZ_STL(a) _##a

// Returns the requested parameter from the passed message (first parameter is 1) as a string
char * netDriver::getParamString(const char *message, int requestedParameter) {
	char *theParameter;
	int counter = 1;

	theParameter = SPZ_STL(strdup)(message);

	for (theParameter = strtok(theParameter, DELIMITER_SEND);	counter < requestedParameter; theParameter = strtok(NULL, DELIMITER_SEND))
		counter++;

	return theParameter;
}

// Returns the requested parameter from the passed message (first parameter is 1) as a floating point number
float netDriver::getParamFloat(const char *message, int requestedParameter) {
	float theFloatResult;
	char *theStringResult;

	theStringResult = getParamString(message, requestedParameter);

	// Search for the parameter and transform it in a float
	if (sscanf(theStringResult, "%f", &theFloatResult) != 1)
		throw std::exception();

	// Return the result
	return theFloatResult;
}

void onData_SendResponse(dyad_Event *e) {
	auto theResponse = NETDRV->processMessage(e->data);

	// Send the response and close the connection
	dyad_write(e->stream, theResponse, int(strlen(theResponse)));
	dyad_end(e->stream);

	delete[] theResponse;
}

void onAccept(dyad_Event *e) {
	dyad_addListener(e->remote, DYAD_EVENT_DATA, onData_SendResponse, NULL);
}

void onListen(dyad_Event *e) {
	LOG->Info(LogLevel::MED, "Network listener started in port: %d", dyad_getPort(e->stream));
}

void onError(dyad_Event *e) {
	LOG->Error("Network server error: %s", e->msg);
}

void onConnectToEngine(dyad_Event *e) {
	LOG->Info(LogLevel::MED, "Network: Connected to editor through port: %d", dyad_getPort(e->stream));
	NETDRV->connectedToEditor = true;
}

netDriver& netDriver::GetInstance() {
	static netDriver obj;
	return obj;
}

netDriver::netDriver()
{
	port		= 28000;	// Port for receiving data from the Editor
	port_send	= 28001;	// Port for sending data to the Editor
	inited = false;
	connectedToEditor = false;
}



void netDriver::init()
{
	this->messageToSend = "";
	this->connectedToEditor = false;

	dyad_init();

	dyad_Stream *serv = dyad_newStream();
	//dyad_setNoDelay(serv, 1); // Disable Nagle's algorithm
	dyad_setUpdateTimeout(0);	// Disable waiting
	// Listeners for answering responses from the editor
	dyad_addListener(serv, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, onAccept, NULL);
	dyad_addListener(serv, DYAD_EVENT_LISTEN, onListen, NULL);
	dyad_listenEx(serv, "0.0.0.0", port, 511);
	
	this->connectToEditor();

	inited = true;
}

// TODO: We should fix this! it should not be a global variable!
dyad_Stream *serv_connect;
void netDriver::connectToEditor()
{
	// Listener for sending messages to the editor
	LOG->Info(LogLevel::MED, "Network: outgoing messages will be done through port: %d", this->port_send);
	serv_connect = dyad_newStream();
	dyad_addListener(serv_connect, DYAD_EVENT_CONNECT, onConnectToEngine, NULL);
	dyad_connect(serv_connect, "127.0.0.1", port_send);
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

char * netDriver::processMessage(const char * message)
{
	// Incoming information
	char *identifier, *type, *action;

	// Outcoming information
	auto theResponse = new char[4096];
	char *theResult;

	identifier = getParamString(message, 1);
	type = getParamString(message, 2);
	action = getParamString(message, 3);

	theResult = "OK";
	char theInformation[1024];

	LOG->Info(LogLevel::LOW, "Message received: [identifier: %s] [type: %s] [action: %s]", identifier, type, action);

	// Commands processing
	if (strcmp(type, "command") == 0) {
		if		(strcmp(action, "pause") == 0)			{ DEMO->pauseDemo();								theResult = "OK"; }
		else if (strcmp(action, "play") == 0)			{ DEMO->playDemo();									theResult = "OK"; }
		else if (strcmp(action, "restart") == 0)		{ DEMO->restartDemo();								theResult = "OK"; }
		else if (strcmp(action, "startTime") == 0)		{ DEMO->setStartTime(getParamFloat(message, 4));	theResult = "OK"; }
		else if (strcmp(action, "currentTime") == 0)	{ DEMO->setCurrentTime(getParamFloat(message, 4));	theResult = "OK"; }
		else if (strcmp(action, "endTime") == 0)		{ DEMO->setEndTime(getParamFloat(message, 4));		theResult = "OK"; }
		else if (strcmp(action, "ping") == 0)			{ theResult = "OK"; }
		else if (strcmp(action, "end") == 0)			{ DEMO->exitDemo = true;							theResult = "OK"; }
		else {
			theResult = "NOK";
			sprintf((char *)theInformation, "Unknown command (%s)", message);
		}
	}
	// Sections processing
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
		
		else if (strcmp(action, "toggle") == 0)			{ DEMO->sectionManager.toggleSection(getParamString(message, 4));										theResult = "OK"; }
		else if (strcmp(action, "delete") == 0)			{ DEMO->sectionManager.deleteSection(getParamString(message, 4));										theResult = "OK"; }
		else if (strcmp(action, "update") == 0)			{ DEMO->sectionManager.updateSection(getParamString(message, 4),getParamString(message, 5));			theResult = "OK"; }
		else if (strcmp(action, "setStartTime") == 0)	{ DEMO->sectionManager.setSectionsStartTime(getParamString(message, 4), getParamString(message, 5));	theResult = "OK"; }
		else if (strcmp(action, "setEndTime") == 0)		{ DEMO->sectionManager.setSectionsEndTime(getParamString(message, 4), getParamString(message, 5));		theResult = "OK"; }
		else if (strcmp(action, "setLayer") == 0)		{ DEMO->sectionManager.setSectionLayer(getParamString(message, 4), getParamString(message, 5));		theResult = "OK"; }
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
	sprintf((char *)theResponse, "%s%c%s%c%f%c%d%c%f%c%s",	identifier, DELIMITER,
															theResult, DELIMITER,
															DEMO->fps, DELIMITER,
															DEMO->state, DELIMITER,
															DEMO->demo_runTime, DELIMITER,
															(char *)theInformation);
	
	//LOG->Info(LogLevel::LOW, "Sending response: [%s]", theResponse);
		
	// and return the response (will be freed later)
	return theResponse;
}

void netDriver::sendMessage(std::string message)
{
	if (this->connectedToEditor) {
		dyad_write(serv_connect, message.c_str(), (int)message.length());
	}
}
