#pragma once

#include "MTMultiplex.h"
#include "Base.h"
#include "Packing.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexServer : public MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT MultiplexServer();
			MTMULTIPLEX_EXPORT ~MultiplexServer();
			MTMULTIPLEX_EXPORT int setup(char* host_name, int port) override;
			MTMULTIPLEX_EXPORT int disconnect(unsigned int timeout) override;
			MTMULTIPLEX_EXPORT int send(const char* data, unsigned int dataLength, const char* info, unsigned int infoLength, unsigned int channel, int flags) override;
			MTMULTIPLEX_EXPORT int send(unsigned long long userId, unsigned long long instance, const void* packet) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned int channel, unsigned long long instance) override;
			MTMULTIPLEX_EXPORT int bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance) override;
			MTMULTIPLEX_EXPORT MultiplexEvent process_event(unsigned int timeout) override;
			MTMULTIPLEX_EXPORT void* create_system_packet(MultiplexSystemResponses responseType,
				unsigned long long userId, unsigned long long instance, int flags,
				char* data = nullptr, size_t dataSize = 0, char* info = nullptr, size_t infoSize = 0, unsigned long long* userIds = nullptr, size_t userIdsSize = 0);
		protected:
			void* client = NULL;
			void* peer = NULL;
			Megatowel::MultiplexPacking::Packing packer;
			// Our buffers that we can safely use ;>
			char* dataBuffer = NULL;
			char* infoBuffer = NULL;
			char* sendBuffer = NULL;
			std::map<unsigned long long, MultiplexUser*> Users;
			std::map<unsigned long long, MultiplexInstance> Instances;
		};
	}
}
