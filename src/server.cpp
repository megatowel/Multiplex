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
		send(nullptr, nullptr, nullptr, MultiplexResponse::Disconnect);
		running = false;
		processThread.join();
		if (host != nullptr)
		{
			enet_host_destroy((ENetHost *)host);
		}
	}
}

void MultiplexServer::setup(const char *host_name, const unsigned short port)
{
	ENetAddress address{ENET_HOST_ANY, port};
	enet_address_set_host(&address, host_name);
	host = enet_host_create(&address,
							MULTIPLEX_MAX_SERVER_CONNECTIONS,
							MULTIPLEX_MAX_CHANNELS + 1,
							0,
							0);
	if (!host)
		MULTIPLEX_ERROR("An error occurred while trying to create an ENet client host.");
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
			enet_host_broadcast((ENetHost *)host, 0, (ENetPacket *)packet);
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

	// Wait for an event.
	if (enet_host_service((ENetHost *)client, &event, 5000) > 0)
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
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			user = (MultiplexUser *)event.peer->data;

			// We can't unpack anything less than 3 bytes.
			if (event.packet->dataLength < 3)
			{
				break;
			}

			auto data = unpack_fields((char *)event.packet->data, event.packet->dataLength);

			if (event.channelID == 0)
			{
				if (data[PACK_FIELD_TYPE].size == 0)
				{
					// Ignore packet without opcode
					break;
				}

				MultiplexResponse action = (MultiplexResponse)(*((int *)(data[PACK_FIELD_TYPE].data)));
				switch (action)
				{
				case MultiplexResponse::InstanceModify:
				{
					if (data[PACK_FIELD_DATA].size == 0)
					{
						// Ignore packet without parameter
						break;
					}

					// TODO: bind user to requested instance maybe
					//user->bind()

					break;
				}
				case MultiplexResponse::Message:
				{
					// TODO: message recieved callback

					break;
				}
				}
			}

			// Clean up the packet now that we're done using it.
			enet_packet_destroy(event.packet);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			user = (MultiplexUser *)event.peer->data;

			// User is removed from users vector in destructor.
			delete user;

			// Reset the peer's client information.
			event.peer->data = NULL;
		}
		}
	}
}