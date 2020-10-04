#pragma once

#include "MTMultiplex.h"

#define MAX_MULTIPLEX_CHANNELS 33 /* The amount of channels to use. 0 for system, rest for instances. */
#define MAX_MULTIPLEX_SERVER_CONNECTIONS 1024

namespace Megatowel {
	namespace Multiplex {
		class MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT static void MultiplexBase::poopshit();
			MTMULTIPLEX_EXPORT virtual int MultiplexBase::setup(char* hostname, int port) = 0;
			MTMULTIPLEX_EXPORT virtual int MultiplexBase::send(const char* data, unsigned int dataLength, unsigned int channel, int flags = 0) = 0;
			MTMULTIPLEX_EXPORT virtual int MultiplexBase::process_event() = 0;
		protected:
			void* client = NULL;
			void* peer = NULL;
		};
	}
}

