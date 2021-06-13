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
		class MULTIPLEX_EXPORT MultiplexBase
		{
		public:
			MultiplexBase();
			virtual void setup(const char *hostname, const unsigned short port) = 0;
			virtual void disconnect(unsigned int timeout) = 0;
			virtual void send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const = 0;
			virtual void bind_channel(MultiplexUser *user, MultiplexInstance *instance, const unsigned int channel) = 0;
			std::vector<MultiplexUser *> users;
			std::vector<MultiplexInstance *> instances;

		protected:
			void *client, *peer;
			virtual void process() = 0;

		private:
			MultiplexBase(const MultiplexBase &);
		};
	}
}
#endif
