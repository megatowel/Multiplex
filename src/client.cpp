// This unit is for client functionality.
//

#include "multiplex/client.hpp"
#include <enet/enet.h>
#include "error.hpp"

using namespace std;
using namespace Megatowel::Multiplex;

MultiplexClient::MultiplexClient()
{
	// TODO: actually figure this out
	running = true;
	processThread = std::thread([this]()
								{
									while (running)
									{
										try
										{
											process();
										}
										catch (std::exception e)
										{
											fprintf(stderr, e.what());
										}
									}
								});
	processThread.detach();
}

MultiplexClient::~MultiplexClient()
{
	disconnect();
}

void MultiplexClient::disconnect()
{
	if (running)
	{
		running = false;
		processThread.join();

		// TODO: integrate into threads and provide an appropriate callback
		if (peer != nullptr)
		{
			ENetEvent event;
			enet_peer_disconnect((ENetPeer *)peer.load(), 0);
			// Wait up to 5 seconds for the connection attempt to succeed.
			if (enet_host_service((ENetHost *)host.load(), &event, 5000) > 0 &&
				event.type != ENET_EVENT_TYPE_DISCONNECT)
			{
				// Either the 5 seconds are up or server didn't respond
				// Reset the peer in the event the 5 seconds
				// had run out without any significant event.
				enet_peer_reset((ENetPeer *)peer.load());
			}
		}

		if (host != nullptr)
		{
			enet_host_destroy((ENetHost *)host.load());
		}
	}
}

void MultiplexClient::setup(const char *host_name, unsigned short port, const std::function<void(MultiplexEvent)> callback)
{
	this->callback = callback;
	// Set up a client.
	host = enet_host_create(NULL,						  //create a client host
							  1,						  // connections limit
							  MULTIPLEX_MAX_CHANNELS + 1, // Refer to the definition of MULTIPLEX_MAX_CHANNELS.
							  0,						  // assume any amount of incoming bandwidth
							  0);						  // assume any amount of outgoing bandwidth
	if (!host)
		MULTIPLEX_ERROR("An error occurred while trying to create an ENet client host.");
	
	ENetAddress address;
	ENetEvent event;

	enet_address_set_host(&address, host_name);
	address.port = port;
	// Initiate the connection, allocating the two channels 0 and MULTIPLEX_MAX_CHANNELS.
	peer = enet_host_connect((ENetHost *)host.load(), &address, MULTIPLEX_MAX_CHANNELS + 1, 0);
	if (peer == NULL)
		MULTIPLEX_ERROR("No available peers for initiating an ENet connection.");
	
	// Wait up to 5 seconds for the connection attempt to succeed.
	if (enet_host_service((ENetHost *)host.load(), &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		return;
	}
	else
	{
		// Either the 5 seconds are up or a disconnect event was
		// received. Reset the peer in the event the 5 seconds
		// had run out without any significant event.
		enet_peer_reset((ENetPeer *)peer.load());
		MULTIPLEX_ERROR("Failed to make an ENet connection with the selected peer.");
	}
}

void MultiplexClient::send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data, const size_t dataSize) const
{
	// Reusing most of the server's sending code. This may or may not work.

	if (destination)
	{
		auto peer = (ENetPeer *)destination->peer;
		auto packet = MultiplexPacket(type, sender, instance, data, dataSize).to_native_packet();
		enet_peer_send(peer, destination->find_channel(instance), (ENetPacket *)packet); // no flags for now
	}
	else
	{
		if (instance)
		{
			for (const auto user : instance->users)
			{
				auto peer = (ENetPeer *)user->peer;
				auto packet = MultiplexPacket(type, user, instance, data, dataSize).to_native_packet();
				enet_peer_send(peer, user->find_channel(instance), (ENetPacket *)packet); // no flags for now
			}
		}
		else
		{
			auto packet = MultiplexPacket(type, nullptr, nullptr, data, dataSize).to_native_packet();
			enet_peer_send((ENetPeer *)peer.load(), 0, (ENetPacket *)packet);
		}
	}
}

void MultiplexClient::bind_channel(MultiplexUser *user, MultiplexInstance *instance, const unsigned int channel) {
	// TODO: Client channel binding
}

void MultiplexClient::process()
{
	ENetEvent event;
	MultiplexEvent friendlyEvent {};

	// Wait for an event.
	if (enet_host_service((ENetHost *)host.load(), &event, 5000) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			friendlyEvent.type = MultiplexResponse::Connect;
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			// User IDs are probably going to be exclusive to MultiplexResponse::Message
			// friendlyEvent.fromUserId = *((unsigned long long *)(data[PACK_FIELD_FROM_USERID].data));
			
			friendlyEvent.type = (MultiplexResponse)(*((int *)(event.packet->data)));

			switch (friendlyEvent.type)
			{
			case MultiplexResponse::Message:
			{
				// Messages contain an user id and any data.
				break;
			}
			case MultiplexResponse::RemoteInstanceJoin:
			{
				// Indicates we have joined an instance
				break;
			}
			case MultiplexResponse::RemoteInstanceLeave:
			{
				// Indicates we have left an instance
				break;
			}
			case MultiplexResponse::InstanceJoin:
			{
				// Indicates an user joined our instance 
				break;
			}
			case MultiplexResponse::InstanceLeave:
			{
				// Indicates an user left
				break;
			}
			}
			break;
			// Clean up the packet now that we're done using it.
			enet_packet_destroy(event.packet);
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			friendlyEvent.type = MultiplexResponse::Disconnect;
			enet_peer_reset((ENetPeer *)peer.load());
			break;
		}
		}

		callback(friendlyEvent);
	}
}
