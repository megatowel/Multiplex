/// @file Client.h
/// @brief This file holds the MultiplexServer class, which can listen for MultiplexClients and communicate.
#ifndef MULTIPLEXSERVER_H
#define MULTIPLEXSERVER_H

#include "multiplex.hpp"
#include "base.hpp"
#include "packing.hpp"

namespace Megatowel
{
	namespace Multiplex
	{
		class MultiplexServer : public MultiplexBase
		{
		public:
			MULTIPLEX_EXPORT MultiplexServer();
			MULTIPLEX_EXPORT ~MultiplexServer();
			MULTIPLEX_EXPORT int setup(const char *host_name, int port) override;
			MULTIPLEX_EXPORT int disconnect(unsigned int timeout) override;
			MULTIPLEX_EXPORT int send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags) override;
			MULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void *packet) override;
			MULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;
			MULTIPLEX_EXPORT int bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance) override;
			MULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;
			MULTIPLEX_EXPORT void *create_system_packet(MultiplexSystemResponses responseType,
														unsigned long long userId, unsigned long long instance, int flags,
														const char *data = nullptr, size_t dataSize = 0, const char *info = nullptr, size_t infoSize = 0, unsigned long long *userIds = nullptr, size_t userIdsSize = 0);

		protected:
			void *client = NULL;
			void *peer = NULL;
			Megatowel::Multiplex::Packing packer;
			// Our buffers that we can safely use ;>
			char *dataBuffer = NULL;
			char *infoBuffer = NULL;
			char *sendBuffer = NULL;
			std::map<unsigned long long, MultiplexUser *> Users;
			std::map<unsigned long long, MultiplexInstance> Instances;
		};
	}
}
#endif
