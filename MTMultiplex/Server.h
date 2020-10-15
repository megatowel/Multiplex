#pragma once

#include "MTMultiplex.h"
#include "Base.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexServer : public MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT MultiplexServer();
			MTMULTIPLEX_EXPORT ~MultiplexServer();
			MTMULTIPLEX_EXPORT int setup(char* host_name, int port) override;
			MTMULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;
			MTMULTIPLEX_EXPORT int send(const char* data, unsigned int dataLength, unsigned int channel, int flags) override;
			MTMULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void* packet) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;

		protected:
			void* create_system_packet(Megatowel::Multiplex::MultiplexSystemResponses responseType,
				unsigned long long userId, int flags, std::vector<uint8_t> data);
			void* create_system_packet(Megatowel::Multiplex::MultiplexSystemResponses responseType,
				unsigned long long userId, int flags);
			void* client = NULL;
			void* peer = NULL;
			std::map<unsigned long long, MultiplexInstance> Instances;
		};
	}
}
