// This unit is for client functionality.
//

#include <enet/enet.h>
#include <nlohmann/json.hpp>
#include "MTMultiplex.h"
#include "Base.h"
#include "Client.h"

using json = nlohmann::json;
using namespace std;

namespace Megatowel {
	namespace Multiplex {

		MultiplexClient::MultiplexClient() {
			dataBuffer = new char[2048];
		}

		MultiplexClient::~MultiplexClient() {
			delete dataBuffer;
			if (client != NULL) {
				enet_host_destroy((ENetHost*)client);
			}
		}

		int MultiplexClient::setup(char* host_name, int port) {
			// Set up a client.
			cout << "Multiplex Client is being created" << endl;
			client = enet_host_create(NULL, //create a client host
				1, // connections limit
				MAX_MULTIPLEX_CHANNELS + 1, // Refer to the definition of MAX_MULTIPLEX_CONNECTIONS.
				0, // assume any amount of incoming bandwidth
				0); // assume any amount of outgoing bandwidth
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
			peer = enet_host_connect((ENetHost*)client, &address, MAX_MULTIPLEX_CHANNELS + 1, 0);
			if (peer == NULL)
			{
				fprintf(stderr,
					"No available peers for initiating an ENet connection.\n");
				return -1;
			}
			// Wait up to 5 seconds for the connection attempt to succeed.
			if (enet_host_service((ENetHost*)client, &event, 5000) > 0 &&
				event.type == ENET_EVENT_TYPE_CONNECT)
			{
				return 0;
			}
			else
			{
				// Either the 5 seconds are up or a disconnect event was 
				// received. Reset the peer in the event the 5 seconds   
				// had run out without any significant event.            
				enet_peer_reset((ENetPeer*)peer);
				return 1;
			}
		}

		int MultiplexClient::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) {
			ENetPacket* packet2 = enet_packet_create(data,
				(size_t)dataLength + (size_t)0x00000001,
				(int)flags && MT_SEND_RELIABLE);
			enet_peer_send((ENetPeer*)peer, channel, packet2);
			if ((int)flags && MT_NO_FLUSH) {
				enet_host_flush((ENetHost*)client);
			}
			return 0;
		}

		int MultiplexClient::send(unsigned long long userId, unsigned long long instance, const void* packet) {
			return -2;
		}
		
		int MultiplexClient::bind_channel(unsigned int channel, unsigned long long instance) {
			json data;
			data["action"] = MultiplexActions::EditChannel;
			data["channel"] = channel;
			data["instance"] = instance;
			auto rawData = json::to_msgpack(data);
			ENetPacket* packet = enet_packet_create(rawData.data(),
				rawData.size(),
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send((ENetPeer*)peer, 0, packet);
			enet_host_flush((ENetHost*)client);
			return 0;
		}
		MultiplexEvent MultiplexClient::process_event(unsigned int timeout) {
			ENetEvent event;
			MultiplexEvent friendlyEvent;
			// Wait for an event. 
			if (enet_host_service((ENetHost*)client, &event, timeout) > 0) {
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT: {
					friendlyEvent.eventType = MultiplexEventType::Connected;
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE: {
					std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength]);
					json data = json::from_msgpack(packet_vector);
					switch ((MultiplexSystemResponses)data["r"])
						{
						case MultiplexSystemResponses::Message: {
							friendlyEvent.eventType = MultiplexEventType::UserMessage;
							friendlyEvent.fromUserId = (unsigned long long)data["i"];
							friendlyEvent.channelId = (unsigned int)event.channelID;
							vector<uint8_t> bin = data["d"].get_binary();

							// Copy from vector.
							// We don't want to use .data() because reference out of scope is bad ;/
							for (unsigned int i = 0; i < (unsigned int)bin.size(); ++i)
							{
								dataBuffer[i] = bin[i];
							}

							// Very important now that we have a data buffer.
							friendlyEvent.dataSize = (unsigned int)bin.size();

							friendlyEvent.data = dataBuffer;
							break;
						}
						case MultiplexSystemResponses::UserSetup: {
							friendlyEvent.eventType = MultiplexEventType::UserSetup;
							friendlyEvent.fromUserId = (unsigned long long)data["i"];
							break;
						}
						case MultiplexSystemResponses::InstanceConnected: {
							friendlyEvent.eventType = MultiplexEventType::InstanceConnected;
							// Nothing here for the moment
							break;
						}
						case MultiplexSystemResponses::InstanceUserJoin: {
							friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
							friendlyEvent.fromUserId = (unsigned long long)data["i"];
							friendlyEvent.channelId = (unsigned int)event.channelID;
							friendlyEvent.instanceId = (unsigned int)event.channelID;
							break;
						}
						case MultiplexSystemResponses::InstanceUserLeave: {
							friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
							friendlyEvent.fromUserId = (unsigned long long)data["i"];
							friendlyEvent.channelId = (unsigned int)event.channelID;
							friendlyEvent.instanceId = 0;
							break;
						}

					}
					break;
					// Clean up the packet now that we're done using it. 
					enet_packet_destroy(event.packet);
				}
				case ENET_EVENT_TYPE_DISCONNECT: {
					enet_peer_reset((ENetPeer*)peer);
					cout << "Disconnected from server. State:" << event.peer->state << endl;
					friendlyEvent.eventType = MultiplexEventType::Disconnected;
					break;
				}
				}
			}
			else {
				// Didn't get event.
				friendlyEvent.eventType = MultiplexEventType::Error;
				friendlyEvent.Error = MultiplexErrors::NoEvent;
			}
			return friendlyEvent;
		}

	}
}
