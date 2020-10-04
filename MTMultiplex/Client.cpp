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
		}

		MultiplexClient::~MultiplexClient() {
			if (client != NULL) {
				enet_host_destroy((ENetHost*)client);
			}
		}

		int MultiplexClient::setup(char* host_name, int port) {
			// Set up a client.
			cout << "Multiplex Client is being created" << endl;
			client = enet_host_create(NULL, //create a client host
				1, // connections limit
				MAX_MULTIPLEX_CHANNELS, // Refer to the definition of MAX_MULTIPLEX_CONNECTIONS.
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
			// Initiate the connection, allocating the two channels 0 and 32.
			cout << "Connecting to Multiplex server... Host: " << host_name << " Port: " << port << endl;
			peer = enet_host_connect((ENetHost*)client, &address, 33, 0);
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
				cout << "Connected! Host: " << host_name << " Port: " << port << endl;
				json data;
				data["action"] = MultiplexActions::EditChannel;
				data["channel"] = 1;
				data["instance"] = 1;
				auto rawData = json::to_msgpack(data);
				cout << rawData.data() << endl;
				cout << "Sent " << sizeof(rawData.data()) << endl;
				// Create a reliable packet of size 7 containing "packet\0"
				ENetPacket* packet = enet_packet_create(rawData.data(),
					rawData.size() + 1,
					ENET_PACKET_FLAG_RELIABLE);
				// Send the packet to the peer over channel id 0. 
				// One could also broadcast the packet by         
				// enet_host_broadcast (host, 0, packet);         
				enet_peer_send((ENetPeer*)peer, 0, packet);
			}
			else
			{
				// Either the 5 seconds are up or a disconnect event was 
				// received. Reset the peer in the event the 5 seconds   
				// had run out without any significant event.            
				enet_peer_reset((ENetPeer*)peer);
				cout << "Connection failed. Host: " << host_name << " Port: " << port << endl;
				return 1;
			}
			return 0;
		}

		int MultiplexClient::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) {
			ENetPacket* packet2 = enet_packet_create(data,
				dataLength + 0x00000001,
				(int)flags && MT_SEND_RELIABLE);
			enet_peer_send((ENetPeer*)peer, channel, packet2);
			enet_host_flush((ENetHost*)client);
			return 0;
		}

		int MultiplexClient::process_event() {
			ENetEvent event;
			MultiplexUser* user;
			// Wait up to 5000 milliseconds for an event. 
			if (enet_host_service((ENetHost*)client, &event, 5000) > 0) {
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received from %s on channel %u.\n",
						event.packet->dataLength,
						event.packet->data,
						event.peer->data,
						event.channelID);
					if (event.channelID != 0) {
						std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength - 1]);
						json data = json::from_msgpack(packet_vector);
						cout << data << endl;
						cout << "From ID: " << data["i"] << endl;
						if (data["d"].get_binary().data() != NULL) {
							cout << data["d"].get_binary().data() << endl;
						}
					}
					// Clean up the packet now that we're done using it. 
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					enet_peer_reset((ENetPeer*)peer);
					cout << "Disconnected from server." << endl;
					break;
				}
				return (int)event.type;
			}
			else {
				// Didn't get event. Timeout.
				return 1;
			}
		}

	}
}
