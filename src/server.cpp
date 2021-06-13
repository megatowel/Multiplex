// This unit is for server functionality.
//

#include "multiplex/server.hpp"
#include <enet/enet.h>
#include "error.hpp"

using namespace std;
using namespace Megatowel::Multiplex;

MultiplexServer::MultiplexServer()
{
}

MultiplexServer::~MultiplexServer()
{
	if (host != nullptr)
	{
		enet_host_destroy((ENetHost *)host);
	}
}

void MultiplexServer::disconnect(unsigned int timeout)
{
	MULTIPLEX_ERROR("Disconnecting is not yet supported.");
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

void MultiplexServer::send(const MultiplexUser *destination, const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const
{
	if (destination)
	{
		if (instance)
		{
			auto peer = (ENetPeer *)destination->peer;
			auto packet = MultiplexPacket(type, sender, instance, data, dataSize).to_native_packet();
			enet_peer_send(peer, destination->find_channel(instance), (ENetPacket *)packet); // no flags for now
		}
		else
		{
			auto peer = (ENetPeer *)destination->peer;
			auto packet = MultiplexPacket(type, nullptr, nullptr, data, dataSize).to_native_packet();
			enet_peer_send(peer, 0, (ENetPacket *)packet); // no flags for now
		}
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
			previous->send(user, nullptr, MultiplexResponse::RemoteInstanceLeave);
			previous->users.erase(previous->users.begin() + user->get_user_index(previous));
		}

		instance->users.push_back(user);
		user->channels[channel - 1] = instance;
		user->send(instance, nullptr, MultiplexResponse::InstanceJoin, (char *)instance->users.size() - 1, sizeof(instance->users.size()));
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
				//friendlyEvent.type = MultiplexResponse::Error;
				//friendlyEvent.error = MultiplexError::BadPacking;
			}

			auto data = unpack_fields((char *)event.packet->data, event.packet->dataLength);

			if (event.channelID == 0)
			{
				if (data[PACK_FIELD_TYPE].size == 0)
				{
					//friendlyEvent.type = MultiplexResponse::Error;
					//friendlyEvent.error = MultiplexError::MissingActionArgs;
					break;
				}

				MultiplexResponse action = (MultiplexResponse)(*((int *)(data[PACK_FIELD_TYPE].data)));
				switch (action)
				{
				case MultiplexResponse::InstanceModify:
				{
					if (data[PACK_FIELD_DATA].size == 0)
					{
						//friendlyEvent.type = MultiplexResponse::Error;
						//friendlyEvent.error = MultiplexError::MissingActionArgs;
						break;
					}
					unsigned int editingChannel = *((unsigned int *)(data[PACK_FIELD_CHANNELID].data));
					unsigned long long instanceId = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));

					friendlyEvent.type = MultiplexResponse::InstanceModify;
					friendlyEvent.channelId = editingChannel;
					friendlyEvent.instanceId = instanceId;

					break;
				}
				case MultiplexResponse::Message:
				{
					if (data[PACK_FIELD_DATA].size)
					{
						friendlyEvent.data = data[PACK_FIELD_DATA].data;
						friendlyEvent.dataSize = (unsigned int)data[PACK_FIELD_DATA].size;
					}
					if (data[PACK_FIELD_CHANNELID].size)
					{
						friendlyEvent.channelId = *((unsigned int *)(data[PACK_FIELD_CHANNELID].data));
					}
					if (data[PACK_FIELD_INSTANCEID].size)
					{
						friendlyEvent.instanceId = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));
					}

					break;
				}
				}
			}
			else
			{
				unsigned long long currentInstanceId = user->channelInstances[event.channelID - 1];

				if (currentInstanceId == 0 || data[PACK_FIELD_DATA].size == 0 || data[PACK_FIELD_INFO].size == 0)
				{
					friendlyEvent.error = MultiplexError::FailedRelay;

					// Clean up the packet now that we're done using it.
					enet_packet_destroy(event.packet);

					break;
				}

				friendlyEvent.type = MultiplexResponse::UserMessage;
				friendlyEvent.fromUserId = user->userId;
				friendlyEvent.channelId = (unsigned int)event.channelID;
				friendlyEvent.instanceId = currentInstanceId;

				// Copy from packet to class buffer arrays.
				memcpy(dataBuffer, data[PACK_FIELD_DATA].data, data[PACK_FIELD_DATA].size);
				memcpy(infoBuffer, data[PACK_FIELD_INFO].data, data[PACK_FIELD_INFO].size);

				friendlyEvent.data = dataBuffer;
				friendlyEvent.info = infoBuffer;

				// Very important to state the size of the buffers.
				friendlyEvent.dataSize = (unsigned int)data[PACK_FIELD_DATA].size;
				friendlyEvent.infoSize = (unsigned int)data[PACK_FIELD_INFO].size;
			}
			// Clean up the packet now that we're done using it.
			enet_packet_destroy(event.packet);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			user = (MultiplexUser *)event.peer->data;
			friendlyEvent.type = MultiplexResponse::Disconnected;
			friendlyEvent.fromUserId = user->userId;
			for (int i = 0; i < MULTIPLEX_MAX_CHANNELS; i++)
			{
				bind_channel(user->userId, i + 1, 0);
			}

			// Remove user's data from memory. Yes, this does delete the user struct.
			// It's because of unique_ptr. Since we remove it from the users map, the user struct is deleted.
			users.erase(user->userId);

			// Reset the peer's client information.
			event.peer->data = NULL;
		}
		}
	}
	else
	{
	}
}