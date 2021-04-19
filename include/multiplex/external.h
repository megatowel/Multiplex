/// @file External.h
/// @brief This file holds C bindings.
#ifndef MULTIPLEXEXTERNAL_H
#define MULTIPLEXEXTERNAL_H

#include "multiplex.hpp"
#include "base.hpp"
#include "client.hpp"
#include "server.hpp"
#include "packing.hpp"

using namespace Megatowel::Multiplex;

extern "C"
{
	MULTIPLEX_EXPORT MultiplexClient *c_make_client();
	MULTIPLEX_EXPORT MultiplexServer *c_make_server();
	MULTIPLEX_EXPORT Packing *c_make_packer();
	MULTIPLEX_EXPORT PackingField *c_unpack_fields(Packing *packer, char *source, size_t size);
	MULTIPLEX_EXPORT size_t c_pack_field(Packing *packer, uint8_t fieldNum, char *fieldData, size_t fieldSize, size_t currentPos, char *destChar);
	MULTIPLEX_EXPORT int c_setup(MultiplexBase *multiplex, char *hostname, int port);
	MULTIPLEX_EXPORT int c_destroy(MultiplexBase *multiplex);
	MULTIPLEX_EXPORT int c_destroy_packer(Packing *packer);
	MULTIPLEX_EXPORT int c_disconnect(MultiplexBase *multiplex, unsigned int timeout);
	MULTIPLEX_EXPORT void *c_create_system_packet(MultiplexBase *multiplex, MultiplexSystemResponses responseType,
												  unsigned long long userId, unsigned long long instance, int flags,
												  const char *data, size_t dataSize, const char *info, size_t infoSize, unsigned long long *userIds, size_t userIdsSize);
	MULTIPLEX_EXPORT int c_send(MultiplexBase *multiplex, const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags = 0);
	MULTIPLEX_EXPORT int c_send_server(MultiplexBase *multiplex, unsigned long long userId, unsigned long long instanceId, void *packet);
	MULTIPLEX_EXPORT int c_bind_channel(MultiplexBase *multiplex, unsigned int channel, unsigned long long instance);
	MULTIPLEX_EXPORT int c_bind_channel_server(MultiplexBase *multiplex, unsigned long long userId, unsigned int channel, unsigned long long instance);
	MULTIPLEX_EXPORT MultiplexEvent c_process_event(MultiplexBase *multiplex, unsigned int timeout);
}
#endif