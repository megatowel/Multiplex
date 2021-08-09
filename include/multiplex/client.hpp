/// @file Client.h
/// @brief This file holds the MultiplexClient class, which can connect to a MultiplexServer and communicate.
#ifndef MULTIPLEXCLIENT_H
#define MULTIPLEXCLIENT_H

#include "multiplex.hpp"
#include "base.hpp"
#include "packing.hpp"
#include "types.hpp"
#include <thread>
#include <atomic>

namespace Megatowel
{
	namespace Multiplex
	{
		/// @class MultiplexClient
		/// @brief Client for networking with Multiplex.
		/// This class can connect to a MultiplexServer and communicate with it.
		class MULTIPLEX_EXPORT MultiplexClient : public MultiplexBase
		{
		public:
			MultiplexClient();
			~MultiplexClient();
			void setup(const char *host_name, const unsigned short port) override;
			void disconnect() override;
			void send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const override;
			void bind_channel(MultiplexUser *user, MultiplexInstance *instance, const unsigned int channel) override;

		protected:
			std::atomic<bool> running = false;
			std::thread processThread;
			void process() override;

		private:
			MultiplexClient(const MultiplexClient &) = delete;
		};
	}
}
#endif
