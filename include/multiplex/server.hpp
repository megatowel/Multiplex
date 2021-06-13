/// @file Client.h
/// @brief This file holds the MultiplexServer class, which can listen for MultiplexClients and communicate.
#ifndef MULTIPLEXSERVER_H
#define MULTIPLEXSERVER_H

#include "multiplex.hpp"
#include "base.hpp"
#include "packing.hpp"

namespace Megatowel
{
	namespace Multiplex
	{
		class MULTIPLEX_EXPORT MultiplexServer : public MultiplexBase
		{
		public:
			MultiplexServer();
			~MultiplexServer();
			void setup(const char *host_name, const unsigned short port) override;
			void disconnect(unsigned int timeout) override;
			void send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const override;
			void bind_channel(MultiplexUser *user, MultiplexInstance *instance, const unsigned int channel) override;

		protected:
			void *host, *peer;
			void process() override;

		private:
			MultiplexServer(const MultiplexServer &);
		};
	}
}
#endif
