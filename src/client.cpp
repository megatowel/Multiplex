// This unit is for client functionality.
//

#include "multiplex/client.hpp"
#include <enet/enet.h>


using namespace std;
using namespace Megatowel::MultiplexPacking;

namespace Megatowel
{
	namespace Multiplex
	{

		MultiplexClient::MultiplexClient() {
			dataBuffer = new char[MAX_MULTIPLEX_DATA_SIZE];
			sendBuffer = new char[MAX_MULTIPLEX_DATA_SIZE];
			infoBuffer = new char[MAX_MULTIPLEX_DATA_SIZE];
			packer = Packing();
			userIdsBuffer = new unsigned long long[128];
			for (auto i = 0; i < MAX_MULTIPLEX_CHANNELS; i++)
			{
				instanceByChannel[i] = 0;
			}
		}

		MultiplexClient::~MultiplexClient()
		{
			delete dataBuffer;
			delete sendBuffer;
			delete infoBuffer;
			if (client != NULL)
			{
				enet_host_destroy((ENetHost *)client);
			}
		}

		int MultiplexClient::disconnect(unsigned int timeout)
		{
			ENetEvent event;
			enet_peer_disconnect((ENetPeer *)peer, 0);
			// Wait up to 5 seconds for the connection attempt to succeed.
			if (enet_host_service((ENetHost *)client, &event, timeout) > 0 &&
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
			cout << "Multiplex Client is being created" << endl;
			client = enet_host_create(NULL,						  //create a client host
									  1,						  // connections limit
									  MAX_MULTIPLEX_CHANNELS + 1, // Refer to the definition of MAX_MULTIPLEX_CHANNELS.
									  0,						  // assume any amount of incoming bandwidth
									  0);						  // assume any amount of outgoing bandwidth
			if (client == NULL)
			{
				fprintf(stderr,
						"An error occurred while trying to create an ENet client host.\n");
				return EXIT_FAILURE;
			}
			ENetAddress address;
			ENetEvent event;

			enet_address_set_host(&address, host_name);
			address.port = port;
			// Initiate the connection, allocating the two channels 0 and MAX_MULTIPLEX_CHANNELS.
			cout << "Connecting to Multiplex server... Host: " << host_name << " Port: " << port << endl;
			peer = enet_host_connect((ENetHost *)client, &address, MAX_MULTIPLEX_CHANNELS + 1, 0);
			if (peer == NULL)
			{
				fprintf(stderr,
						"No available peers for initiating an ENet connection.\n");
				return -1;
			}
			// Wait up to 5 seconds for the connection attempt to succeed.
			if (enet_host_service((ENetHost *)client, &event, 5000) > 0 &&
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
				enet_host_flush((ENetHost *)client);
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
			enet_host_flush((ENetHost *)client);
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
			if (enet_host_service((ENetHost *)client, &event, timeout) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					friendlyEvent.eventType = MultiplexEventType::Connected;
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					PackingField *data = packer.unpack_fields((char *)event.packet->data, event.packet->dataLength);

					friendlyEvent.fromUserId = *((unsigned long long *)(data[PACK_FIELD_FROM_USERID].data));
					// help me please
					MultiplexSystemResponses response = (MultiplexSystemResponses)(*((int *)(data[PACK_FIELD_RESPONSE_TYPE].data)));
					switch (response)
					{
					case MultiplexSystemResponses::Message:
					{
						friendlyEvent.eventType = MultiplexEventType::UserMessage;
						friendlyEvent.channelId = (unsigned int)event.channelID;

						friendlyEvent.data = data[PACK_FIELD_DATA].data;
						friendlyEvent.info = data[PACK_FIELD_INFO].data;

						// Very important to state the size of the buffers.
						friendlyEvent.dataSize = (unsigned int)data[PACK_FIELD_DATA].size;
						friendlyEvent.infoSize = (unsigned int)data[PACK_FIELD_INFO].size;

						break;
					}
					case MultiplexSystemResponses::UserSetup:
					{
						friendlyEvent.eventType = MultiplexEventType::UserSetup;
						break;
					}
					case MultiplexSystemResponses::InstanceConnected:
					{
						friendlyEvent.eventType = MultiplexEventType::InstanceConnected;
						instanceByChannel[event.channelID] = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));

						if (data[PACK_FIELD_INSTANCEID].size > 0)
						{
							instanceByChannel[event.channelID] = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));
						}
						else
						{
							instanceByChannel[event.channelID] = 0;
						}

						// Since we may be switched to an instance, we need to refresh our users vector.
						// Even when we get switched to instance 0, the instance id for no instance.
						usersByChannel[event.channelID] = std::vector<unsigned long long>();

						for (int i = 0; i < data[PACK_FIELD_USERIDS].size / 8; ++i)
						{
							userIdsBuffer[i] = ((unsigned long long *)(data[PACK_FIELD_USERIDS].data))[i];
							usersByChannel[event.channelID].push_back(((unsigned long long *)(data[PACK_FIELD_USERIDS].data))[i]);
						}
						friendlyEvent.userIds = userIdsBuffer;
						friendlyEvent.userIdsSize = (unsigned int)data[PACK_FIELD_USERIDS].size / 8;
						friendlyEvent.channelId = (unsigned int)event.channelID;
						friendlyEvent.instanceId = *((unsigned long long *)(data[PACK_FIELD_INSTANCEID].data));
						break;
					}
					case MultiplexSystemResponses::InstanceUserJoin:
					{
						friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
						friendlyEvent.channelId = (unsigned int)event.channelID;
						friendlyEvent.instanceId = instanceByChannel[event.channelID];
						usersByChannel[event.channelID].push_back(friendlyEvent.fromUserId);
						break;
					}
					case MultiplexSystemResponses::InstanceUserLeave:
					{
						friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
						friendlyEvent.channelId = (unsigned int)event.channelID;
						friendlyEvent.instanceId = 0;
						usersByChannel[event.channelID].erase(std::find(usersByChannel[event.channelID].begin(), usersByChannel[event.channelID].end(), friendlyEvent.fromUserId));
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
					cout << "Disconnected from server. State:" << event.peer->state << endl;
					friendlyEvent.eventType = MultiplexEventType::Disconnected;
					break;
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

	}
}
