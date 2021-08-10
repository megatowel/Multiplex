// This unit is for server functionality.
//

#include "multiplex/server.hpp"
#include <enet/enet.h>
#include "error.hpp"

using namespace std;
using namespace Megatowel::Multiplex;

MultiplexServer::MultiplexServer()
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

MultiplexServer::~MultiplexServer()
{
	disconnect();
}

void MultiplexServer::disconnect()
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

void MultiplexServer::setup(const char *host_name, const unsigned short port, const std::function<void(MultiplexEvent)> callback)
{
	ENetAddress address{ENET_HOST_ANY, port};
	enet_address_set_host(&address, host_name);
	host = enet_host_create(&address,
							MULTIPLEX_MAX_SERVER_CONNECTIONS,
							MULTIPLEX_MAX_CHANNELS + 1,
							0,
							0);
	if (!host)
		MULTIPLEX_ERROR("An error occurred while trying to create an ENet server host.");
}

void MultiplexServer::send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data, const size_t dataSize) const
{
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
			enet_host_broadcast((ENetHost *)host.load(), 0, (ENetPacket *)packet);
		}
	}
}

void MultiplexServer::bind_channel(MultiplexUser *user, MultiplexInstance *instance, const unsigned int channel)
{
	if (user && instance)
	{
		MultiplexInstance *previous = user->channels[channel - 1];
		if (previous)
		{
			previous->send(nullptr, user, MultiplexResponse::RemoteInstanceLeave);
			previous->users.erase(previous->users.begin() + user->get_user_index(previous));
		}

		instance->users.push_back(user);
		user->channels[channel - 1] = instance;
		user->send(instance, nullptr, MultiplexResponse::InstanceJoin, (char *)(instance->users.size() - 1), sizeof(instance->users.size()));
	}
	else
	{
		MULTIPLEX_ERROR("Tried to bind a null user or instance to a channel.");
	}
}

void MultiplexServer::process()
{
	ENetEvent event;
	MultiplexUser *user;
	MultiplexEvent friendlyEvent {};

	// Wait for an event.
	if (enet_host_service((ENetHost *)host.load(), &event, 5000) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			printf("A new client connected from %x:%u.\n",
				   event.peer->address.host,
				   event.peer->address.port);

			// Store any relevant client information here.
			user = new MultiplexUser(this);

			event.peer->data = (void *)user;
			user->peer = (void *)event.peer;

			users.push_back(user);
			user->send(nullptr, nullptr, MultiplexResponse::Connect);

			// A callback for a new user connecting to us.
			friendlyEvent.sender = user;
			friendlyEvent.type = MultiplexResponse::Connect;
			callback(friendlyEvent);
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			user = (MultiplexUser *)event.peer->data;
			friendlyEvent.sender = user;

			// We can't receive anything less than 1 byte. (not sure if ENet allows sending of 0 bytes)
			if (event.packet->dataLength < 1)
			{
				break;
			}

			// Only recognizing packets from channel 0. instance stuff soon(tm) 
			if (event.channelID == 0)
			{
				friendlyEvent.type = (MultiplexResponse)(*((int *)(event.packet->data)));

				// This will be used to populate friendlyEvent with data that's only included because of its type.
				switch (friendlyEvent.type)
				{
				case MultiplexResponse::InstanceModify:
				{
					// TODO: bind user to requested instance maybe
					// or actually leave it up to the user to implement.
					//user->bind()

					break;
				}
				case MultiplexResponse::Message:
				{
					// TODO: message recieved callback

					break;
				}
				}

				callback(friendlyEvent);
			}

			// Clean up the packet now that we're done using it.
			enet_packet_destroy(event.packet);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			user = (MultiplexUser *)event.peer->data;

			// Do a callback and tell us this user is about to no longer exist.
			friendlyEvent.sender = user;
			friendlyEvent.type = MultiplexResponse::Disconnect;
			callback(friendlyEvent);

			// User is removed from users vector in destructor.
			delete user;

			// Reset the peer's client information.
			event.peer->data = NULL;
		}
		}
	}
}