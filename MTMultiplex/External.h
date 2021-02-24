#pragma once

#include "MTMultiplex.h"
#include "Base.h"
#include "Client.h"
#include "Server.h"
#include "Packing.h"

using namespace Megatowel::Multiplex;
using namespace Megatowel::MultiplexPacking;

extern "C" {
	MTMULTIPLEX_EXPORT MultiplexClient* c_make_client();
	MTMULTIPLEX_EXPORT MultiplexServer* c_make_server();
	MTMULTIPLEX_EXPORT Packing* c_make_packer();
	MTMULTIPLEX_EXPORT PackingField* c_unpack_fields(Packing* packer, char* source, size_t size);
	MTMULTIPLEX_EXPORT size_t c_pack_field(Packing* packer, uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar);
	MTMULTIPLEX_EXPORT int c_setup(MultiplexBase* multiplex, char* hostname, int port);
	MTMULTIPLEX_EXPORT int c_destroy(MultiplexBase* multiplex);
	MTMULTIPLEX_EXPORT int c_destroy_packer(Packing* packer);
	MTMULTIPLEX_EXPORT int c_disconnect(MultiplexBase* multiplex, unsigned int timeout);
	MTMULTIPLEX_EXPORT void* c_create_system_packet(MultiplexBase* multiplex, MultiplexSystemResponses responseType,
		unsigned long long userId, unsigned long long instance, int flags,
		char* data, size_t dataSize, char* info, size_t infoSize, unsigned long long* userIds, size_t userIdsSize);
	MTMULTIPLEX_EXPORT int c_send(MultiplexBase* multiplex, const char* data, unsigned int dataLength, const char* info, unsigned int infoLength, unsigned int channel, int flags = 0);
	MTMULTIPLEX_EXPORT int c_send_server(MultiplexBase* multiplex, unsigned long long userId, unsigned long long instanceId, void* packet);
	MTMULTIPLEX_EXPORT int c_bind_channel(MultiplexBase* multiplex, unsigned int channel, unsigned long long instance);
	MTMULTIPLEX_EXPORT int c_bind_channel_server(MultiplexBase* multiplex, unsigned long long userId, unsigned int channel, unsigned long long instance);
	MTMULTIPLEX_EXPORT MultiplexEvent c_process_event(MultiplexBase* multiplex, unsigned int timeout);
}