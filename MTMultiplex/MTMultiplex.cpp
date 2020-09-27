// MTMultiplex.cpp : Defines the entry point for the application.
//

#include <enet/enet.h>
#include <nlohmann/json.hpp>
#include "MTMultiplex.h"

// This seems a bit hacky.
// Taking any suggestion something to make this a little more neat.
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

using json = nlohmann::json;
using namespace std;

const int MAX_MULTIPLEX_CHANNELS = 33; /* allow up 33 channels to be used, 0 for system, rest for instances. */
const int MAX_MULTIPLEX_SERVER_CONNECTIONS = 1024;

int Init_ENet()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);
    return 0;
}


Multiplex::Multiplex() {
}

int Multiplex::Setup_Host(bool is_server, char* host_name, int port) {
    ENetAddress address;
    if (is_server) {
        cout << "Multiplex Server is being created" << endl;
        address.host = ENET_HOST_ANY;
        enet_address_set_host(&address, host_name);

        address.port = port;
        client = enet_host_create(&address /* create a server host */,
            MAX_MULTIPLEX_SERVER_CONNECTIONS /* connections limit */,
            MAX_MULTIPLEX_CHANNELS /* Refer to the definition of MAX_MULTIPLEX_CONNECTIONS. */,
            0 /* assume any amount of incoming bandwidth */,
            0 /* assume any amount of outgoing bandwidth */);
    }
    else {
        cout << "Multiplex Client is being created" << endl;
        client = enet_host_create(NULL /* create a client host */,
            1 /* connections limit */,
            MAX_MULTIPLEX_CHANNELS /* Refer to the definition of MAX_MULTIPLEX_CONNECTIONS. */,
            0 /* assume any amount of incoming bandwidth */,
            0 /* assume any amount of outgoing bandwidth */);
    }
    if (client == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
    }
    return 0;
}
Multiplex::~Multiplex() {
    if (client != NULL) {
        enet_host_destroy((ENetHost*)client);
    }
}

int Multiplex::Client_Connect(char* host_name, int port)
{
    ENetAddress address;
    ENetEvent event;

    enet_address_set_host(&address, host_name);
    address.port = port;
    /* Initiate the connection, allocating the two channels 0 and 32. */
    cout << "Connecting to Multiplex server... Host: " << host_name << " Port: " << port << endl;
    peer = enet_host_connect((ENetHost*)client, &address, 33, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        return -1;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
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
        /* Create a reliable packet of size 7 containing "packet\0" */
        ENetPacket* packet = enet_packet_create(rawData.data(),
            rawData.size() + 1,
            ENET_PACKET_FLAG_RELIABLE);
        /* Send the packet to the peer over channel id 0. */
        /* One could also broadcast the packet by         */
        /* enet_host_broadcast (host, 0, packet);         */
        enet_peer_send((ENetPeer*)peer, 0, packet);
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset((ENetPeer*)peer);
        cout << "Connection failed. Host: " << host_name << " Port: " << port << endl;
    }
    return 0;
}
int Multiplex::Send(const char* data, unsigned int dataLength, unsigned int channel, bool reliable) {
    ENetPacket* packet2 = enet_packet_create(data,
        dataLength + 1,
        (int)reliable);
    enet_peer_send((ENetPeer*)peer, channel, packet2);
    enet_host_flush((ENetHost*)client);
    return 0;
}

int Multiplex::Process_Event() {
    ENetEvent event;
    MultiplexUser* user;
    /* Wait up to 5000 milliseconds for an event. */
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
                cout << data["d"].get_binary().data() << endl;
            }
            /* Clean up the packet now that we're done using it. */
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
        return -1;
    }
}

int Multiplex::Process_Server_Event() {
    ENetEvent event;
    MultiplexUser* user;
    /* Wait up to 5000 milliseconds for an event. */
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
            /* Store any relevant client information here. */
            clientIdIncrement++;
            user = (MultiplexUser*)malloc(sizeof(MultiplexUser));
            //MultiplexUser* ptruser = &user;
            event.peer->data = user;
            for (int i = 0; i < 32; i++) {
                (user)->channelInstances[i] = 0;
            }
            (user)->userId = clientIdIncrement;
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
                std::vector<uint8_t> packet_vector(&event.packet->data[0], &event.packet->data[event.packet->dataLength-1]);
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
                        MultiplexInstance instance{ instanceId, "", std::map<int, MultiplexInstanceUser>()};

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
            /* Clean up the packet now that we're done using it. */
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
            /* Reset the peer's client information. */
            event.peer->data = NULL;
        }
        return (int)event.type;
    }
    else {
        cout << "Didn't get event." << endl;
        return -1;
    }
    
}
