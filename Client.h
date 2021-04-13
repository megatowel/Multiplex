/// @file Client.h
/// @brief This file holds the MultiplexClient class, which can connect to a MultiplexServer and communicate.
#ifndef CLIENT_H
#define CLIENT_H

#include "MTMultiplex.h"
#include "Base.h"
#include "Packing.h"

namespace Megatowel
{
	namespace Multiplex
	{
		/// @class MultiplexClient
		/// @brief Client for networking with Multiplex.
		/// This class can connect to a MultiplexServer and communicate with it.
		class MultiplexClient : public MultiplexBase
		{
		public:
			MTMULTIPLEX_EXPORT MultiplexClient();
			MTMULTIPLEX_EXPORT ~MultiplexClient();
			MTMULTIPLEX_EXPORT int setup(const char *host_name, int port) override;
			MTMULTIPLEX_EXPORT int disconnect(unsigned int timeout) override;
			MTMULTIPLEX_EXPORT int send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags) override;
			MTMULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void *packet) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance) override;
			MTMULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;

		protected:
			void *client = NULL;
			void *peer = NULL;
			Megatowel::MultiplexPacking::Packing packer;
			// Our buffers that we can safely use ;>
			char *dataBuffer = NULL;
			char *infoBuffer = NULL;
			char *sendBuffer = NULL;
			unsigned long long *userIdsBuffer = NULL;
			std::vector<unsigned long long> usersByChannel[MAX_MULTIPLEX_CHANNELS];
			unsigned long long instanceByChannel[MAX_MULTIPLEX_CHANNELS];
		};
	}
}
#endif