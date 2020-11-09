#include "External.h"
#include <iostream>

MultiplexClient* c_make_client() {
	return new MultiplexClient;
}

MultiplexServer* c_make_server() {
	return new MultiplexServer;
}

Packing* c_make_packer() {
	return new Packing;
}

PackingField* c_unpack_fields(Packing* packer, char* source, size_t size) {
	return packer->unpack_fields(source, size);
}

size_t c_pack_field(Packing* packer, uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar) {
	return packer->pack_field(fieldNum, fieldData, fieldSize, currentPos, destChar);
}

int c_destroy(MultiplexBase* multiplex) {
	delete multiplex;
	return 0;
}

int c_destroy_packer(Packing* packer) {
	delete packer;
	return 0;
}


int c_disconnect(MultiplexBase* multiplex, unsigned int timeout) {
	return multiplex->disconnect(timeout);
}

int c_setup(MultiplexBase* multiplex, char* hostname, int port) {
	return multiplex->setup(hostname, port);
}

int c_send(MultiplexBase* multiplex, const char* data, unsigned int dataLength, const char* info, unsigned int infoLength, unsigned int channel, int flags) {
	return multiplex->send(data, dataLength, info, infoLength, channel, flags);
}

int c_bind_channel(MultiplexBase* multiplex, unsigned int channel, unsigned long long instance) {
	return multiplex->bind_channel(channel, instance);
}

int c_bind_channel_server(MultiplexBase* multiplex, unsigned long long userId, unsigned int channel, unsigned long long instance) {
	return multiplex->bind_channel(userId, channel, instance);
}

MultiplexEvent c_process_event(MultiplexBase* multiplex, unsigned int timeout) {
	MultiplexEvent mtmp_event = multiplex->process_event(timeout);
	return mtmp_event;
}
