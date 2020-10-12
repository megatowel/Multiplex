#pragma once

#include "MTMultiplex.h"

namespace Megatowel {
	namespace Multiplex {
		class MultiplexBase {
		public:
			MTMULTIPLEX_EXPORT virtual int setup(char* hostname, int port) = 0;
			MTMULTIPLEX_EXPORT virtual int send(const char* data, unsigned int dataLength, unsigned int channel, int flags = 0) = 0;
			MTMULTIPLEX_EXPORT virtual MultiplexEvent process_event(unsigned int timeout) = 0;
		protected:
			void* client = NULL;
			void* peer = NULL;
		};
	}
}

