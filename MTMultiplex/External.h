#pragma once

#include "MTMultiplex.h"
#include "Base.h"
#include "Client.h"
#include "Server.h"

using namespace Megatowel::Multiplex;

extern "C" {
	MTMULTIPLEX_EXPORT MultiplexClient* c_make_client();
	MTMULTIPLEX_EXPORT MultiplexServer* c_make_server();
	MTMULTIPLEX_EXPORT int c_setup(MultiplexBase* multiplex, char* hostname, int port);
	MTMULTIPLEX_EXPORT int c_send(MultiplexBase* multiplex, const char* data, unsigned int dataLength, unsigned int channel, int flags = 0);
	MTMULTIPLEX_EXPORT MultiplexEvent* c_process_event(MultiplexBase* multiplex, unsigned int timeout);
}