/// @file Client.h
/// @brief This file holds the MultiplexClient class, which can connect to a MultiplexServer and communicate.
#ifndef MULTIPLEXCLIENT_H
#define MULTIPLEXCLIENT_H

#include "multiplex.hpp"
#include "base.hpp"
#include "packing.hpp"

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
			MULTIPLEX_EXPORT MultiplexClient();
			MULTIPLEX_EXPORT ~MultiplexClient();
			MULTIPLEX_EXPORT int setup(const char *host_name, int port) override;
			MULTIPLEX_EXPORT int disconnect(unsigned int timeout) override;
			MULTIPLEX_EXPORT int send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags) override;
			MULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void *packet) override;
			MULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;
			MULTIPLEX_EXPORT int bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance) override;
			MULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;

		protected:
			void *client = NULL;
			void *peer = NULL;
			Packer packer;
			char *dataBuffer = NULL;
			char *infoBuffer = NULL;
			char *sendBuffer = NULL;
			unsigned long long *userIdsBuffer = NULL;
			std::vector<unsigned long long> usersByChannel[MULTIPLEX_MAX_CHANNELS];
			unsigned long long instanceByChannel[MULTIPLEX_MAX_CHANNELS];

		private:
			MULTIPLEX_EXPORT MultiplexClient(const MultiplexClient &) = delete;
		};
	}
}
#endif
