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
				MAX_MULTIPLEX_CHANNELS + 1,
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

		int MultiplexServer::send(unsigned long long userId, unsigned long long instance, const void* packet) {
			if (userId == 0) {
				for (const std::pair<unsigned long long, MultiplexInstanceUser> &element : Instances[instance].users) {
					MultiplexInstanceUser instanceUser = element.second;
					ENetPeer* instanceUserPeer = (ENetPeer*)instanceUser.peer;
					enet_peer_send(instanceUserPeer, instanceUser.channel, (ENetPacket*)packet);
				}
			}
			else {
				MultiplexInstanceUser instanceUser = Instances[instance].users[userId];
				ENetPeer* instanceUserPeer = (ENetPeer*)instanceUser.peer;
				enet_peer_send(instanceUserPeer, instanceUser.channel, (ENetPacket*)packet);
			}
			return 0;
		}

		int MultiplexServer::send(const char* data, unsigned int dataLength, unsigned int channel, int flags) {
			return -2;
		}
		int MultiplexServer::bind_channel(unsigned int channel, unsigned long long instance) {
			return -2;
		}

		void* MultiplexServer::create_system_packet(MultiplexSystemResponses responseType,
			unsigned long long userId, int flags, std::vector<uint8_t> data) {
			json msg;
			msg["r"] = responseType;
			msg["i"] = userId;
			msg["d"] = json::binary_t(data);
			vector<uint8_t> jsonData = json::to_msgpack(msg);
			return (void*)enet_packet_create(jsonData.data(),
				jsonData.size(),
				flags);
		}

		void* MultiplexServer::create_system_packet(MultiplexSystemResponses responseType,
			unsigned long long userId, int flags) {
			json msg;
			msg["r"] = responseType;
			msg["i"] = userId;
			vector<uint8_t> jsonData = json::to_msgpack(msg);
			return (void*)enet_packet_create(jsonData.data(),
				jsonData.size(),
				flags);
		}

		MultiplexEvent MultiplexServer::process_event(unsigned int timeout) {
			ENetEvent event;
			MultiplexUser* user;
			MultiplexEvent friendlyEvent;
			
			// Wait up to 5000 milliseconds for an event.
			if (enet_host_service((ENetHost*)client, &event, timeout) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT: {
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);

					// Store any relevant client information here.
					user = new MultiplexUser;
					if (user == NULL) {
						exit(2);
					}
					event.peer->data = user;
					for (int i = 0; i < MAX_MULTIPLEX_CHANNELS; i++) {
						(user)->channelInstances[i] = 0;
					}
					unsigned long long userId = 0;
					while (userId == 0) {
						userId = distr(eng);
					}
					(user)->userId = userId;
					cout << (user)->userId << endl;
					ENetPacket* packet = (ENetPacket*)create_system_packet(MultiplexSystemResponses::UserSetup, user->userId, 0);
					enet_peer_send(event.peer, 0, packet);
					break;}
				case ENET_EVENT_TYPE_RECEIVE: {
					user = (MultiplexUser*)event.peer->data;
					friendlyEvent.fromUserId = user->userId;
					
					if (event.channelID == 0) {
						std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[(int)event.packet->dataLength]);
						json data = json::from_msgpack(packet_vector);
						MultiplexActions action = data["action"];
						cout << data << endl;
						switch (action) {
						case MultiplexActions::EditChannel:
							unsigned int editingChannel = data["channel"];
							unsigned long long instanceId = data["instance"];

							friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
							friendlyEvent.channelId = editingChannel;
							friendlyEvent.instanceId = instanceId;

							unsigned long long oldInstance = user->channelInstances[editingChannel - 1];

							ENetPacket* leavePacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceUserLeave, user->userId, 0);
							send(0, oldInstance, leavePacket);

							if (oldInstance != 0) {
								Instances[oldInstance].users.erase(user->userId);
							}

							if (instanceId == 0) {
								user->channelInstances[editingChannel - 1] = 0;
								if (Instances[oldInstance].users.size() == 0) {
									Instances.erase(Instances[oldInstance].id);
								}
								break;
							}
							if (Instances.count(instanceId) == 0) {
								cout << "new instance" << endl;
								MultiplexInstance instance{ instanceId, "", std::map<unsigned long long, MultiplexInstanceUser>() };

								Instances.insert(std::pair<unsigned long long, MultiplexInstance>(instanceId, instance));
							}
							MultiplexInstanceUser instanceUser;
							instanceUser.channel = editingChannel;
							instanceUser.peer = event.peer;
							Instances[instanceId].users.insert(std::pair<unsigned long long, MultiplexInstanceUser>(user->userId, instanceUser));

							user->channelInstances[editingChannel - 1] = instanceId;

							ENetPacket* joinPacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceUserJoin, user->userId, 0);
							send(0, instanceId, joinPacket);
							

							break;
						}
					}
					else {
						std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength - 1]);
						unsigned long long currentInstanceId = user->channelInstances[event.channelID - 1];
						friendlyEvent.eventType = MultiplexEventType::UserMessage;
						friendlyEvent.fromUserId = user->userId;
						friendlyEvent.channelId = (unsigned int)event.channelID;
						friendlyEvent.data = (char*)event.packet->data;
						friendlyEvent.dataSize = (unsigned int)event.packet->dataLength;
						if (currentInstanceId == 0) {
							friendlyEvent.Error = MultiplexErrors::FailedRelay;
							break;
						}

						ENetPacket* relayPacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::Message, user->userId, event.packet->flags, packet_vector);
						
						send(0, currentInstanceId, relayPacket);
					}
					// Clean up the packet now that we're done using it.
					enet_packet_destroy(event.packet);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT: {
					cout << "Disconnecting... State: " << event.peer->state << endl;
					user = (MultiplexUser*)event.peer->data;
					cout << "User ID: " << user->userId << " has disconnected." << endl;
					friendlyEvent.eventType = MultiplexEventType::Disconnected;
					friendlyEvent.fromUserId = user->userId;
					for (int i = 0; i < MAX_MULTIPLEX_CHANNELS; i++) {
						unsigned long long oldInstance = user->channelInstances[i];
						if (oldInstance != 0) {
							Instances[oldInstance].users.erase(user->userId);
							if (Instances[oldInstance].users.size() == 0) {
								Instances.erase(Instances[oldInstance].id);
							}
							else {
								ENetPacket* packet = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceUserLeave, user->userId, 1);
								send(0, oldInstance, packet);
							}
						}
					}
					// Remove user's data from memory.
					delete user;
					// Reset the peer's client information.
					event.peer->data = NULL;
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
