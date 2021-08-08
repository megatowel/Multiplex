// This unit is for client functionality.
//

#include "multiplex/client.hpp"
#include <enet/enet.h>

using namespace std;
using namespace Megatowel::Multiplex;

MultiplexClient::MultiplexClient()
{
	dataBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	sendBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	infoBuffer = new char[MULTIPLEX_MAX_DATA_SIZE];
	userIdsBuffer = new unsigned long long[128];
	for (auto i = 0; i < MULTIPLEX_MAX_CHANNELS; i++)
	{
		instanceByChannel[i] = 0;
	}
}

MultiplexClient::~MultiplexClient()
{
	delete[] dataBuffer;
	delete[] sendBuffer;
	delete[] infoBuffer;
	delete[] userIdsBuffer;
	if (client != NULL)
	{
		enet_host_destroy((ENetHost *)host.load());
	}
}

int MultiplexClient::disconnect(unsigned int timeout)
{
	ENetEvent event;
	enet_peer_disconnect((ENetPeer *)peer, 0);
	// Wait up to 5 seconds for the connection attempt to succeed.
	if (enet_host_service((ENetHost *)host, &event, timeout) > 0 &&
		event.type == ENET_EVENT_TYPE_DISCONNECT)
	{
		return 0;
	}
	else
	{
		// Either the 5 seconds are up or server didn't respond
		// Reset the peer in the event the 5 seconds
		// had run out without any significant event.
		enet_peer_reset((ENetPeer *)peer);
		return -1;
	}
}

int MultiplexClient::setup(const char *host_name, int port)
{
	// Set up a client.
	host = enet_host_create(NULL,						  //create a client host
							  1,						  // connections limit
							  MULTIPLEX_MAX_CHANNELS + 1, // Refer to the definition of MULTIPLEX_MAX_CHANNELS.
							  0,						  // assume any amount of incoming bandwidth
							  0);						  // assume any amount of outgoing bandwidth
	if (host == NULL)
	{
		fprintf(stderr,
				"An error occurred while trying to create an ENet client host.\n");
		return EXIT_FAILURE;
	}
	ENetAddress address;
	ENetEvent event;

	enet_address_set_host(&address, host_name);
	address.port = port;
	// Initiate the connection, allocating the two channels 0 and MULTIPLEX_MAX_CHANNELS.
	peer = enet_host_connect((ENetHost *)host.load(), &address, MULTIPLEX_MAX_CHANNELS + 1, 0);
	if (peer == NULL)
	{
		fprintf(stderr,
				"No available peers for initiating an ENet connection.\n");
		return -1;
	}
	// Wait up to 5 seconds for the connection attempt to succeed.
	if (enet_host_service((ENetHost *)host.load(), &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		return 0;
	}
	else
	{
		// Either the 5 seconds are up or a disconnect event was
		// received. Reset the peer in the event the 5 seconds
		// had run out without any significant event.
		enet_peer_reset((ENetPeer *)peer);
		return 1;
	}
}

int MultiplexClient::send(const char *data, unsigned int dataLength, const char *info, unsigned int infoLength, unsigned int channel, int flags)
{
	size_t pos = 0;

	if (channel == 0)
	{
		MultiplexActions action = MultiplexActions::ServerMessage;
		pos = packer.pack_field(PACK_FIELD_ACTION, (char *)(&action), sizeof(int), pos, sendBuffer);
	}
	if (data != nullptr)
		pos = packer.pack_field(PACK_FIELD_DATA, (char *)data, dataLength, pos, sendBuffer);
	if (info != nullptr)
		pos = packer.pack_field(PACK_FIELD_INFO, (char *)info, infoLength, pos, sendBuffer);
	ENetPacket *packet = enet_packet_create(sendBuffer,
											pos,
											(int)flags && MT_SEND_RELIABLE);

	enet_peer_send((ENetPeer *)peer, channel, packet);
	if ((int)flags && MT_NO_FLUSH)
	{
		enet_host_flush((ENetHost *)host);
	}
	return 0;
}

int MultiplexClient::send(unsigned long long userId, unsigned long long instance, const void *packet)
{
	return -2;
}

int MultiplexClient::bind_channel(unsigned int channel, unsigned long long instance)
{
	size_t pos = 0;
	MultiplexActions action = MultiplexActions::EditChannel;
	pos = packer.pack_field(PACK_FIELD_ACTION, (char *)(&action), sizeof(int), pos, sendBuffer);
	pos = packer.pack_field(PACK_FIELD_CHANNELID, (char *)(&channel), sizeof(unsigned int), pos, sendBuffer);
	pos = packer.pack_field(PACK_FIELD_INSTANCEID, (char *)(&instance), sizeof(unsigned long long), pos, sendBuffer);
	ENetPacket *packet = enet_packet_create(sendBuffer,
											pos,
											ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send((ENetPeer *)peer, 0, packet);
	enet_host_flush((ENetHost *)host);
	return 0;
}

int MultiplexClient::bind_channel(unsigned long long userId, unsigned int channel, unsigned long long instance)
{
	return -2;
}

MultiplexEvent MultiplexClient::process_event(unsigned int timeout)
{
	ENetEvent event;
	MultiplexEvent friendlyEvent;
	// Wait for an event.
	if (enet_host_service((ENetHost *)host, &event, timeout) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			friendlyEvent.eventType = MultiplexResponse::Connected;
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			// User IDs are probably going to be exclusive to MultiplexResponse::Message
			// friendlyEvent.fromUserId = *((unsigned long long *)(data[PACK_FIELD_FROM_USERID].data));
			
			MultiplexResponse response = (MultiplexResponse)(*((int *)(event.packet->data)));

			switch (response)
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
			enet_peer_reset((ENetPeer *)peer);
			break;
		}
		}
	}
	else
	{
		// Didn't get event.
	}
	return friendlyEvent;
}
