// This unit is for server functionality.
//

#include <enet/enet.h>
#include <nlohmann/json.hpp>
#include "MTMultiplex.h"
#include "Base.h"
#include "Server.h"
#include <random>

using json = nlohmann::json;
using namespace std;

std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
std::uniform_int_distribution<unsigned long long> distr; // From 0 to the maximum <unsigned long long> value.

namespace Megatowel {
	namespace Multiplex {

		MultiplexServer::MultiplexServer() {
		}

		MultiplexServer::~MultiplexServer() {
			if (client != NULL) {
				enet_host_destroy((ENetHost*)client);
			}
		}

		int MultiplexServer::setup(char* host_name, int port) {
			ENetAddress address;
			cout << "Multiplex Server is being created" << endl;
			address.host = ENET_HOST_ANY;
			enet_address_set_host(&address, host_name);

			address.port = port;
			client = enet_host_create(&address,
				MAX_MULTIPLEX_SERVER_CONNECTIONS,
				MAX_MULTIPLEX_CHANNELS,
				0,
				0
			);
			if (client == NULL)
			{
				fprintf(stderr,
					"An error occurred while trying to create an ENet client host.\n");
				return EXIT_FAILURE;
			}
			return 0;
		}

		int MultiplexServer::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) {
			ENetPacket* packet2 = enet_packet_create(data,
				dataLength + 0x0001,
				(int)flags && MT_SEND_RELIABLE);
			enet_peer_send((ENetPeer*)peer, channel, packet2);
			enet_host_flush((ENetHost*)client);
			return 0;
		}

		int MultiplexServer::process_event() {
			ENetEvent event;
			MultiplexUser* user;
			// Wait up to 5000 milliseconds for an event.
			cout << "Trying for event." << endl;
			if (enet_host_service((ENetHost*)client, &event, 5000) > 0)
			{
				cout << "Event type: " << (int)event.type << endl;
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);

					// Store any relevant client information here.
					user = (MultiplexUser*)malloc(sizeof(MultiplexUser));
					//MultiplexUser* ptruser = &user;
					event.peer->data = user;
					for (int i = 0; i < 32; i++) {
						(user)->channelInstances[i] = 0;
					}
					(user)->userId = distr(eng);
					cout << (user)->userId << endl;
					enet_peer_timeout(event.peer, 5, 5, 5);
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					user = (MultiplexUser*)event.peer->data;
					cout << user->userId << endl;
					printf("A packet of length %u containing %s was received from %s on channel %u.\n",
						event.packet->dataLength,
						event.packet->data,
						to_string(user->userId),
						event.channelID);

					if (event.channelID == 0) {
						std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength - 1]);
						json data = json::from_msgpack(packet_vector);
						MultiplexActions action = data["action"];
						cout << data << endl;
						switch (action) {
						case EditChannel:
							unsigned int editingChannel = data["channel"];
							unsigned int instanceId = data["instance"];

							unsigned int oldInstance = user->channelInstances[editingChannel - 1];
							if (oldInstance != 0) {
								Instances[oldInstance].users.erase(user->userId);
							}
							cout << user->userId << endl;

							if (instanceId == 0) {
								user->channelInstances[editingChannel - 1] = 0;
								return -2;
							}
							if (Instances.count(instanceId) == 0) {
								cout << "new instance" << endl;
								MultiplexInstance instance{ instanceId, "", std::map<int, MultiplexInstanceUser>() };

								Instances.insert(std::pair<int, MultiplexInstance>(instanceId, instance));
							}
							MultiplexInstanceUser instanceUser;
							instanceUser.channel = editingChannel;
							instanceUser.peer = event.peer;
							Instances[instanceId].users.insert(std::pair<int, MultiplexInstanceUser>(user->userId, instanceUser));

							cout << "We have " << Instances[instanceId].users.size() << " users" << endl;

							user->channelInstances[editingChannel - 1] = instanceId;

							cout << Instances[instanceId].id << endl;

							break;
						}
					}
					else {
						cout << "Relay Time!" << endl;
						std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength - 1]);
						unsigned int currentInstanceId = user->channelInstances[event.channelID - 1];
						cout << Instances[1].id << endl;
						cout << user->channelInstances[event.channelID - 1] << endl;
						if (currentInstanceId == 0) {
							cout << "Failed to relay due to no instance assigned to channel " << event.channelID << endl;
							return -3;
						}
						json relayMsg;
						relayMsg["i"] = user->userId;
						relayMsg["d"] = json::binary_t(packet_vector);
						auto jsonData = json::to_msgpack(relayMsg);
						cout << "a" << endl;
						ENetPacket* packet = enet_packet_create(jsonData.data(),
							jsonData.size() + 1,
							event.packet->flags);
						cout << "Sending to " << Instances[currentInstanceId].users.size() << " users" << endl;
						for (std::pair<int, MultiplexInstanceUser> const element : Instances[currentInstanceId].users) {
							cout << "c" << endl;
							MultiplexInstanceUser g = element.second;
							cout << "d" << endl;
							cout << g.channel << endl;
							ENetPeer* h = (ENetPeer*)g.peer;
							cout << ((MultiplexUser*)(h->data))->userId << endl;
							enet_peer_send(h, g.channel, packet);
						}
						cout << "z" << endl;
					}
					// Clean up the packet now that we're done using it.
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					cout << "Disconnecting..." << endl;
					user = (MultiplexUser*)event.peer->data;
					cout << "User ID: " << user->userId << " has disconnected." << endl;
					for (int i = 0; i < 32; i++) {
						unsigned int oldInstance = user->channelInstances[i];
						if (oldInstance != 0) {
							Instances[oldInstance].users.erase(user->userId);
							if (Instances[oldInstance].users.size() == 0) {
								Instances.erase(Instances[oldInstance].id);
							}
						}
					}
					// Reset the peer's client information.
					event.peer->data = NULL;

				}
				return (int)event.type;
			}
			else {
				// Didn't get event. Timeout.
				cout << "Didn't get event." << endl;
				return 1;
			}
		}

	}
}
