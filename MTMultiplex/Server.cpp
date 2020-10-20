﻿// This unit is for server functionality.
//

#include <enet/enet.h>
#include <nlohmann/json.hpp>
#include "MTMultiplex.h"
#include "Base.h"
#include "Server.h"
#include "Packing.h"
#include <random>

using json = nlohmann::json;
using namespace std;

std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
std::uniform_int_distribution<unsigned long long> distr; // From 0 to the maximum <unsigned long long> value.

namespace Megatowel {
	namespace Multiplex {

		MultiplexServer::MultiplexServer() {
			dataBuffer = new char[4096];
			sendBuffer = new char[4096];
			infoBuffer = new char[128];
		}

		MultiplexServer::~MultiplexServer() {
			delete dataBuffer;
			delete infoBuffer;
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

		int MultiplexServer::send(const char* data, unsigned int dataLength, const char* info, unsigned int infoLength, unsigned int channel, int flags) {
			return -2;
		}

		int MultiplexServer::bind_channel(unsigned int channel, unsigned long long instance) {
			return -2;
		}

		int MultiplexServer::bind_channel(unsigned long long userId, unsigned int editingChannel, unsigned long long instanceId) {
			MultiplexUser* user = Users[userId];
			unsigned long long oldInstance = user->channelInstances[editingChannel - 1];

			if (oldInstance != 0) {
				ENetPacket* leavePacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceUserLeave, user->userId, 0, 1);
				send(0, oldInstance, leavePacket);
				Instances[oldInstance].users.erase(user->userId);
			}

			if (instanceId == 0) {
				user->channelInstances[editingChannel - 1] = 0;
				if (Instances[oldInstance].users.size() == 0) {
					Instances.erase(Instances[oldInstance].id);
				}
				std::vector<unsigned long long> emptyVector = std::vector<unsigned long long>();
				ENetPacket* leaveInstancePacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceConnected, user->userId, 0, 1, nullptr, 0, nullptr, 0, &emptyVector);
				send(0, instanceId, leaveInstancePacket);
				return 1;
			}
			if (Instances.count(instanceId) == 0) {
				MultiplexInstance instance{ instanceId, "", std::map<unsigned long long, MultiplexInstanceUser>() };

				Instances.insert(std::pair<unsigned long long, MultiplexInstance>(instanceId, instance));
			}
			MultiplexInstanceUser instanceUser;
			instanceUser.channel = editingChannel;
			instanceUser.peer = user->peer;
			Instances[instanceId].users.insert(std::pair<unsigned long long, MultiplexInstanceUser>(user->userId, instanceUser));

			user->channelInstances[editingChannel - 1] = instanceId;

			ENetPacket* joinPacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceUserJoin, user->userId, 0, 1);
			send(0, instanceId, joinPacket);

			std::vector<unsigned long long> users;
			for (std::map<unsigned long long, MultiplexInstanceUser>::iterator it = Instances[instanceId].users.begin();
				it != Instances[instanceId].users.end(); ++it) {
				users.push_back(it->first);
			}

			ENetPacket* joinInstancePacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::InstanceConnected, user->userId, instanceId, 1, nullptr, 0, nullptr, 0, &users);
			send(user->userId, instanceId, joinInstancePacket);

			return 0;
		}

		void* MultiplexServer::create_system_packet(MultiplexSystemResponses responseType,
			unsigned long long userId, unsigned long long instance, int flags,
			char* data, size_t dataSize, char* info, size_t infoSize, std::vector<unsigned long long>* userIds) {
			size_t pos = 0;

			pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_RESPONSE_TYPE, (char*)&responseType, sizeof(int), pos, sendBuffer);
			pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_FROM_USERID, (char*)&userId, sizeof(unsigned long long), pos, sendBuffer);
			if (data != nullptr)
				pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_DATA, data, dataSize, pos, sendBuffer);
			if (info != nullptr)
				pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_INFO, info, infoSize, pos, sendBuffer);
			if (userIds != nullptr)
				pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_USERIDS, (char*)(*userIds).data(), (*userIds).size() * 8, pos, sendBuffer);
			if (instance != 0)
				pos = Megatowel::MultiplexPacking::pack_field(PACK_FIELD_INSTANCEID, (char*)&instance, sizeof(unsigned long long), pos, sendBuffer);
			return (void*)enet_packet_create(sendBuffer,
				pos,
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
						user->channelInstances[i] = 0;
					}
					unsigned long long userId = 0;
					while (userId == 0) {
						userId = distr(eng);
					}
					user->userId = userId;
					user->peer = (void*)event.peer;
					cout << user->userId << endl;
					Users.insert(std::pair<unsigned long long, MultiplexUser*>(userId, user));
					ENetPacket* packet = (ENetPacket*)create_system_packet(MultiplexSystemResponses::UserSetup, user->userId, 0, 1);
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

							bind_channel(user->userId, editingChannel, instanceId);

							friendlyEvent.eventType = MultiplexEventType::InstanceUserUpdate;
							friendlyEvent.channelId = editingChannel;
							friendlyEvent.instanceId = instanceId;

							break;
						}
					}
					else {
						std::map<unsigned int, std::pair<char*, size_t>> data = Megatowel::MultiplexPacking::unpack_fields((char*)event.packet->data, event.packet->dataLength);
						unsigned long long currentInstanceId = user->channelInstances[event.channelID - 1];
						friendlyEvent.eventType = MultiplexEventType::UserMessage;
						friendlyEvent.fromUserId = user->userId;
						friendlyEvent.channelId = (unsigned int)event.channelID;

						// Copy from packet to class buffer arrays.
						memcpy(dataBuffer, data[PACK_FIELD_DATA].first, data[PACK_FIELD_DATA].second);
						memcpy(infoBuffer, data[PACK_FIELD_INFO].first, data[PACK_FIELD_INFO].second);

						friendlyEvent.data = dataBuffer;
						friendlyEvent.info = infoBuffer;

						// Very important to state the size of the buffers.
						friendlyEvent.dataSize = (unsigned int)data[PACK_FIELD_DATA].second;
						friendlyEvent.infoSize = (unsigned int)data[PACK_FIELD_INFO].second;
						if (currentInstanceId == 0) {
							friendlyEvent.Error = MultiplexErrors::FailedRelay;
							break;
						}

						ENetPacket* relayPacket = (ENetPacket*)create_system_packet(MultiplexSystemResponses::Message, user->userId, 0, event.packet->flags,
							data[PACK_FIELD_DATA].first, data[PACK_FIELD_DATA].second, data[PACK_FIELD_INFO].first, data[PACK_FIELD_INFO].second);
						
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
						bind_channel(user->userId, i + 1, 0);
					}
					// Remove user's data from memory.
					Users.erase(user->userId);
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
