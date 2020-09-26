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
        /* Create a reliable packet of size 7 containing "packet\0" */
        ENetPacket* packet = enet_packet_create("packet",
            strlen("packet") + 1,
            ENET_PACKET_FLAG_RELIABLE);
        /* Send the packet to the peer over channel id 0. */
        /* One could also broadcast the packet by         */
        /* enet_host_broadcast (host, 0, packet);         */
        enet_peer_send((ENetPeer*)peer, 0, packet);
        enet_host_flush((ENetHost*)client);
        enet_peer_reset((ENetPeer*)peer);
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

int Multiplex::Process_Event() {
    ENetEvent event;
    /* Wait up to 5000 milliseconds for an event. */
    if (enet_host_service((ENetHost*)client, &event, 5000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf("A new client connected from %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);
            /* Store any relevant client information here. */
            event.peer->data = "Client information";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                event.packet->dataLength,
                event.packet->data,
                event.peer->data,
                event.channelID);
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s disconnected.\n", event.peer->data);
            /* Reset the peer's client information. */
            event.peer->data = NULL;
        }
    }
    return 0;
}
