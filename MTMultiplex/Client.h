#pragma once

#include "MTMultiplex.h"
#include "Base.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexClient : public MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT MultiplexClient();
			MTMULTIPLEX_EXPORT ~MultiplexClient();
			MTMULTIPLEX_EXPORT int MultiplexClient::setup(char* host_name, int port) override;
			MTMULTIPLEX_EXPORT MultiplexEvent MultiplexClient::process_event(unsigned int timeout) override;
			MTMULTIPLEX_EXPORT int MultiplexClient::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) override;

		protected:
			void* client = NULL;
			void* peer = NULL;
			std::map<int, MultiplexInstance> Instances;
		};
	}
}
