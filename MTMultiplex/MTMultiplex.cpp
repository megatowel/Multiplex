// MTMultiplex.cpp : Defines the entry point for the application.
//

#include "MTMultiplex.h"
#include <enet/enet.h>
#include <nlohmann/json.hpp>

// This seems a bit hacky.
// Taking any suggestion something to make this a little more neat.
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

using json = nlohmann::json;
using namespace std;


int _ENet_Init()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);
    return 0;
}

void Init_Server(char* host, int port)
{
    _ENet_Init();
    // Using a bunch of ENet's example code for now.
    ENetAddress address;
    ENetHost* server;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    enet_address_set_host(&address, host);
    /* Bind the server to port from args. */
    address.port = port;
    cout << "Multiplex's server started. Host: " << host << " Port: " << port << endl;
    server = enet_host_create(&address /* the address to bind the server host to */,
        1024      /* allow up to 32 clients and/or outgoing connections */,
        33      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    while (enet_host_service(server, &event, 5000) > 0)
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
    enet_host_destroy(server);
	return;
}

void Init_Client(char* host, int port)
{
    _ENet_Init();

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    ENetHost* client;
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        33 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);
    if (client == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }
        
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, host);
    address.port = port;
    /* Initiate the connection, allocating the two channels 0 and 32. */
    cout << "Connecting to Multiplex server... Host: " << host << " Port: " << port << endl;
    peer = enet_host_connect(client, &address, 33, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        cout << "Connected! Host: " << host << " Port: " << port << endl;
        /* Create a reliable packet of size 7 containing "packet\0" */
        ENetPacket* packet = enet_packet_create("packet",
            strlen("packet") + 1,
            ENET_PACKET_FLAG_RELIABLE);
        /* Send the packet to the peer over channel id 0. */
        /* One could also broadcast the packet by         */
        /* enet_host_broadcast (host, 0, packet);         */
        enet_peer_send(peer, 0, packet);
        enet_host_flush(client);
        enet_peer_reset(peer);
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        cout << "Connection failed. Host: " << host << " Port: " << port << endl;
    }
    enet_host_destroy(client);
}