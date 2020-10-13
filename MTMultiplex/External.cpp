#include "External.h"
#include <iostream>

MultiplexClient* c_make_client() {
	return new MultiplexClient;
}

MultiplexServer* c_make_server() {
	return new MultiplexServer;
}

int c_setup(MultiplexBase* multiplex, char* hostname, int port) {
	return multiplex->setup(hostname, port);
}

int c_send(MultiplexBase* multiplex, const char* data, unsigned int dataLength, unsigned int channel, int flags) {
	return multiplex->send(data, dataLength, channel, flags);
}

MultiplexEvent* c_process_event(MultiplexBase* multiplex, unsigned int timeout) {
	return &multiplex->process_event(timeout);
}
