#pragma once

#include "MTMultiplex.h"
#include "Base.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexServer : public MultiplexBase {
		public:
			using MultiplexBase::setup;
			using MultiplexBase::process_event;
			using MultiplexBase::send;
			MTMULTIPLEX_EXPORT MultiplexServer();
			MTMULTIPLEX_EXPORT ~MultiplexServer();
			MTMULTIPLEX_EXPORT int MultiplexServer::setup(char* host_name, int port) override;
			MTMULTIPLEX_EXPORT int MultiplexServer::process_event() override;
			MTMULTIPLEX_EXPORT int MultiplexServer::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) override;

		protected:
			void* client = NULL;
			void* peer = NULL;
			std::map<int, MultiplexInstance> Instances;
		};
	}
}
