/// @file Base.h
/// @brief This file holds the abstract base class for Multiplex classes.
#ifndef MULTIPLEXBASE_H
#define MULTIPLEXBASE_H

#include "multiplex.hpp"

namespace Megatowel
{
	namespace Multiplex
	{
		/// @class MultiplexBase
		/// @brief Base class for Multiplex classes.
		/// This class provides universal functionality between the MultiplexClient and MultiplexServer.
		class MultiplexBase
		{
		public:
			MULTIPLEX_EXPORT MultiplexBase();
			MULTIPLEX_EXPORT virtual int setup(const char *hostname, int port) = 0;
			MULTIPLEX_EXPORT virtual int disconnect(unsigned int timeout) = 0;
			MULTIPLEX_EXPORT virtual int send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags = 0) = 0;
			MULTIPLEX_EXPORT virtual int send(unsigned long long userId, unsigned long long instance, const void *packet) = 0;
			MULTIPLEX_EXPORT virtual int bind_channel(unsigned int channel, unsigned long long instance) = 0;
			MULTIPLEX_EXPORT virtual int bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance) = 0;
			MULTIPLEX_EXPORT virtual MultiplexEvent process_event(unsigned int timeout) = 0;

		protected:
			void *client = NULL;
			void *peer = NULL;

		private:
			MULTIPLEX_EXPORT MultiplexBase(const MultiplexBase &);
		};
	}
}
#endif
