// netdriver.cpp
// Spontz Demogroup


#include "main.h"
#include "core/drivers/netdriver.h"
#include "core/drivers/net/dyad.h"

using namespace std;

static void onData(dyad_Event *e) {
	printf("Data received onData!\n Data: %s\n Size: %d\n", e->data, e->size);
	//dyad_write(e->stream, e->data, e->size);
	//printf("Data received onData!\n Data: %s\n Size: %d\n", e->data, e->size);
	//SendResponse(e, e->data);
}

static void onAccept(dyad_Event *e) {
	dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
	//printf("Data received onAccept!\n Data: %s\n Size: %d\n", e->data, e->size);
	//dyad_writef(e->remote, "Echo server\r\n");
}

static void onListen(dyad_Event *e) {
	printf("Server listening: http://localhost:%d\n", dyad_getPort(e->stream));
}

static void onError(dyad_Event *e) {
	printf("Server error: %s\n", e->msg);
}

netDriver::netDriver()
{
	dyad_init();

	dyad_Stream *serv = dyad_newStream();
	//dyad_setNoDelay(serv, 1);
	dyad_setUpdateTimeout(0);
	dyad_addListener(serv, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, onAccept, NULL);
	dyad_addListener(serv, DYAD_EVENT_LISTEN, onListen, NULL);
	dyad_listenEx(serv, "0.0.0.0", 28000, 511); // listen port 28000 from any computer
}

netDriver::~netDriver()
{
}
