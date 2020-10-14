#pragma once

#include "MTMultiplex.h"
#include "Base.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexClient : public MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT MultiplexClient();
			MTMULTIPLEX_EXPORT ~MultiplexClient();
			MTMULTIPLEX_EXPORT int setup(char* host_name, int port) override;
			MTMULTIPLEX_EXPORT int send(const char* data, unsigned int dataLength, unsigned int channel, int flags) override;
			MTMULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void* packet) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;
			MTMULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;

		protected:
			void* client = NULL;
			void* peer = NULL;
			// Our data buffer that we can safely use ;>
			char* dataBuffer = NULL;
			std::map<int, MultiplexInstance> Instances;
		};
	}
}
