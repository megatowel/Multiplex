﻿// This unit is for server functionality.
//

#include "multiplex/server.hpp"
#include "multiplex/multiplex.hpp"
#include "multiplex/base.hpp"
#include <enet/enet.h>
#include <random>

using namespace std;
using namespace Megatowel::Multiplex;

// RNG IDs
std::random_device rd;									 //Get a random seed from the OS entropy device, or whatever
std::mt19937_64 eng(rd());								 //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
std::uniform_int_distribution<unsigned long long> distr; // From 0 to the maximum <unsigned long long> value.

MultiplexServer::MultiplexServer()
{
	dataBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	sendBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	infoBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	packer = Packer();
}

MultiplexServer::~MultiplexServer()
{
	delete[] dataBuffer;
	delete[] sendBuffer;
	delete[] infoBuffer;
	if (client != NULL)
	{
		enet_host_destroy((ENetHost *)client);
	}
}

int MultiplexServer::disconnect(unsigned int timeout)
{
	return -2;
}

int MultiplexServer::setup(const char *host_name, int port)
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	enet_address_set_host(&address, host_name);

	address.port = port;
	client = enet_host_create(&address,
							  MULTIPLEX_MAX_SERVER_CONNECTIONS,
							  MULTIPLEX_MAX_CHANNELS + 1,
							  0,
							  0);
	if (client == NULL)
	{
		fprintf(stderr,
				"An error occurred while trying to create an ENet client host.\n");
		return EXIT_FAILURE;
	}
	return 0;
}

int MultiplexServer::send(unsigned long long userId, unsigned long long instance, const void *packet)
{
	if (userId == 0)
	{
		for (const std::pair<unsigned long long, MultiplexInstanceUser> &element : instances[instance].users)
		{
			MultiplexInstanceUser instanceUser = element.second;
			ENetPeer *instanceUserPeer = (ENetPeer *)instanceUser.peer;
			enet_peer_send(instanceUserPeer, instanceUser.channel, (ENetPacket *)packet);
		}
	}
	else
	{
		MultiplexInstanceUser instanceUser = instances[instance].users[userId];
		ENetPeer *instanceUserPeer = (ENetPeer *)instanceUser.peer;
		enet_peer_send(instanceUserPeer, instanceUser.channel, (ENetPacket *)packet);
	}
	return 0;
}

int MultiplexServer::send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags)
{
	return -2;
}

int MultiplexServer::bind_channel(unsigned int channel, unsigned long long instance)
{
	return -2;
}

int MultiplexServer::bind_channel(unsigned long long userId, unsigned int editingChannel, unsigned long long instanceId)
{
	MultiplexUser *user = users[userId].get();
	unsigned long long oldInstance = user->channelInstances[editingChannel - 1];

	if (oldInstance != 0)
	{
		ENetPacket *leavePacket = (ENetPacket *)create_system_packet(MultiplexSystemResponses::InstanceUserLeave, user->userId, 0, 1);
		send(0, oldInstance, leavePacket);
		instances[oldInstance].users.erase(user->userId);
	}

	if (instanceId == 0)
	{
		user->channelInstances[editingChannel - 1] = 0;
		if (instances[oldInstance].users.size() == 0)
		{
			instances.erase(instances[oldInstance].id);
		}
		std::vector<unsigned long long> emptyVector = std::vector<unsigned long long>();
		ENetPacket *leaveInstancePacket = (ENetPacket *)create_system_packet(MultiplexSystemResponses::InstanceConnected, user->userId, 0, 1, nullptr, 0, nullptr, 0, emptyVector.data(), emptyVector.size());
		send(0, instanceId, leaveInstancePacket);
		return 1;
	}

	if (instances.count(instanceId) == 0)
	{
		MultiplexInstance instance{instanceId, "", std::map<unsigned long long, MultiplexInstanceUser>()};

		instances.insert(std::pair<unsigned long long, MultiplexInstance>(instanceId, instance));
	}

	MultiplexInstanceUser instanceUser;
	instanceUser.channel = editingChannel;
	instanceUser.peer = user->peer;

	instances[instanceId].users.insert(std::pair<unsigned long long, MultiplexInstanceUser>(user->userId, instanceUser));

	user->channelInstances[editingChannel - 1] = instanceId;

	ENetPacket *joinPacket = (ENetPacket *)create_system_packet(MultiplexSystemResponses::InstanceUserJoin, user->userId, 0, 1);
	send(0, instanceId, joinPacket);

	std::vector<unsigned long long> userids;
	for (std::map<unsigned long long, MultiplexInstanceUser>::iterator it = instances[instanceId].users.begin(); it != instances[instanceId].users.end(); ++it)
	{
		userids.push_back(it->first);
	}

	ENetPacket *joinInstancePacket = (ENetPacket *)create_system_packet(MultiplexSystemResponses::InstanceConnected, user->userId, instanceId, 1, nullptr, 0, nullptr, 0, userids.data(), userids.size());
	send(user->userId, instanceId, joinInstancePacket);

	return 0;
}

void *MultiplexServer::create_system_packet(MultiplexSystemResponses responseType, unsigned long long userId, unsigned long long instance, int flags, const char *data, size_t dataSize, const char *info, size_t infoSize, unsigned long long *userIds, size_t userIdsSize)
{
	size_t pos = 0;

	pos = packer.pack_field(PACK_FIELD_RESPONSE_TYPE, (char *)&responseType, sizeof(int), pos, sendBuffer);
	pos = packer.pack_field(PACK_FIELD_FROM_USERID, (char *)&userId, sizeof(unsigned long long), pos, sendBuffer);
	if (data != nullptr)
		pos = packer.pack_field(PACK_FIELD_DATA, (char *)data, dataSize, pos, sendBuffer);
	if (info != nullptr)
		pos = packer.pack_field(PACK_FIELD_INFO, (char *)info, infoSize, pos, sendBuffer);
	if (userIds != nullptr)
		pos = packer.pack_field(PACK_FIELD_USERIDS, (char *)userIds, userIdsSize * 8, pos, sendBuffer);
	if (instance != 0)
		pos = packer.pack_field(PACK_FIELD_INSTANCEID, (char *)&instance, sizeof(unsigned long long), pos, sendBuffer);
	return (void *)enet_packet_create(sendBuffer, pos, flags);
}

MultiplexEvent MultiplexServer::process_event(unsigned int timeout)
{
	ENetEvent event;
	MultiplexUser *user;
	MultiplexEvent friendlyEvent;

	// Wait for an event.
	if (enet_host_service((ENetHost *)client, &event, timeout) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				printf("A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port);

				// Store any relevant client information here.
				user = new MultiplexUser();

				if (user == NULL)
				{
					exit(2);
				}

				event.peer->data = user;

				while (user->userId == 0)
				{
					user->userId = distr(eng);
				}

				user->peer = (void *)event.peer;

				users.insert(std::pair<unsigned long long, std::unique_ptr<MultiplexUser>>(user->userId, user));
				ENetPacket *packet = (ENetPacket *)create_system_packet(MultiplexSystemResponses::UserSetup, user->userId, 0, 1);
				enet_peer_send(event.peer, 0, packet);
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				user = (MultiplexUser *)event.peer->data;
				friendlyEvent.fromUserId = user->userId;

				// We can't unpack anything less than 3 bytes.
				if (event.packet->dataLength < 3)
				{
					friendlyEvent.eventType = MultiplexEventType::Error;
					friendlyEvent.Error = MultiplexErrors::BadPacking;
				}

				PackingField *data = packer.unpack_fields((char *)event.packet->data, event.packet->dataLength);

				if (event.channelID == 0)
				{
					if (data[PACK_FIELD_ACTION].size == 0)
					{
						friendlyEvent.eventType = MultiplexEventType::Error;
						friendlyEvent.Error = MultiplexErrors::MissingActionArgs;
						break;
					}
					MultiplexActions action = (MultiplexActions)(*((int *)(data[PACK_FIELD_ACTION].data)));
					switch (action)
					{
					case MultiplexActions::EditChannel:
					{
						if (data[PACK_FIELD_CHANNELID].size == 0 || data[PACK_FIELD_INSTANCEID].size == 0)
						{
							friendlyEvent.eventType = MultiplexEventType::Error;
							friendlyEvent.Error = MultiplexErrors::MissingActionArgs;
							break;
						}
						unsigned int editingChannel = *((unsigned int *)(data[PACK_FIELD_CHANNELID].data));
						unsigned long long instanceId = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));

						friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
						friendlyEvent.channelId = editingChannel;
						friendlyEvent.instanceId = instanceId;

						break;
					}
					case MultiplexActions::ServerMessage:
					{
						friendlyEvent.eventType = MultiplexEventType::ServerCustom;

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
						friendlyEvent.Error = MultiplexErrors::FailedRelay;

						// Clean up the packet now that we're done using it.
						enet_packet_destroy(event.packet);
						
						break;
					}

					friendlyEvent.eventType = MultiplexEventType::UserMessage;
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
				friendlyEvent.eventType = MultiplexEventType::Disconnected;
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
		// Didn't get event.
		friendlyEvent.eventType = MultiplexEventType::Error;
		friendlyEvent.Error = MultiplexErrors::NoEvent;
	}
	return friendlyEvent;
}