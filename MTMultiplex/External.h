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
	MTMULTIPLEX_EXPORT int c_destroy(MultiplexBase* multiplex);
	MTMULTIPLEX_EXPORT int c_disconnect(MultiplexBase* multiplex, unsigned int timeout);
	MTMULTIPLEX_EXPORT int c_send(MultiplexBase* multiplex, const char* data, unsigned int dataLength, const char* info, unsigned int infoLength, unsigned int channel, int flags = 0);
	MTMULTIPLEX_EXPORT int c_bind_channel(MultiplexBase* multiplex, unsigned int channel, unsigned long long instance);
	MTMULTIPLEX_EXPORT int c_bind_channel_server(MultiplexBase* multiplex, unsigned long long userId, unsigned int channel, unsigned long long instance);
	MTMULTIPLEX_EXPORT MultiplexEvent c_process_event(MultiplexBase* multiplex, unsigned int timeout);
}