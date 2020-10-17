#include "MTMultiplexTest.h"
#include "Client.h"
#include "Server.h"
#include <cstring>
using namespace std;
using namespace Megatowel::Multiplex;

char* info = "chat";

void events_thread(MultiplexClient client)
{
	while (true) {
		MultiplexEvent mtmp_event = client.process_event(5000);
		if (mtmp_event.eventType == MultiplexEventType::UserMessage) {
			bool isChat = true;
			
			if (mtmp_event.infoSize != (unsigned int)4) {
				isChat = false;
			}
			else {
				for (unsigned int i = 0; i < mtmp_event.infoSize; ++i)
				{
					if (mtmp_event.info[i] != info[i]) {
						isChat = false;
					}
				}
			}
			if (isChat) {
				cout << mtmp_event.fromUserId << ": ";
				for (unsigned int i = 0; i < mtmp_event.dataSize; ++i)
				{
					cout << mtmp_event.data[i];
				}
				cout << endl;
			}
		}
	}
}

int main(int argc, char** argv)
{
	if (init_enet() == 0) {
		if (argc == 2) {
			if (!strcmp(argv[1], "-server")) {
				// Starting server without arguments
				cout << "Starting Multiplex server (Bound to 0.0.0.0)" << endl;

				MultiplexServer server;
				if (server.setup("0.0.0.0", 3000)) {
					cout << "Failed to start server." << endl;
				}
				while (true) {
					server.process_event(5000);
				}
				return 0;
			}
			if (!strcmp(argv[1], "-client")) {
				// Starting client without arguments
				cout << "-client requires additional arguments." << endl;
				return 0;
			}
		}
		else if (argc == 3) {
			if (!strcmp(argv[1], "-server")) {
				// Starting server with arguments
				cout << "Starting Multiplex server (Bound to " << argv[2] << ")" << endl;

				MultiplexServer server;
				if (server.setup(argv[2], 3000)) {
					cout << "Failed to start server." << endl;
				}
				while (true) {
					server.process_event(5000);
				}
				return 0;

			}
			if (!strcmp(argv[1], "-client")) {
				// Starting client with arguments
				cout << "Starting Multiplex client (" << argv[2] << ")" << endl;
				MultiplexClient client;
				if (client.setup(argv[2], 3000)) {
					cout << "Failed to start client." << endl;
					return 1;
				}
				client.bind_channel(1, 1);
				std::thread t1(events_thread, client);
				std::string message;
				while (true) {
					cout << "Type your message: ";
					getline(cin, message);
					client.send(message.data(), message.size(), info, (unsigned int)4, 1, true);
				}
			}
		}
		// Starting client without arguments
		cout << "Starting Multiplex client (no args)" << endl;
		MultiplexClient client;
		if (client.setup("localhost", 3000)) {
			cout << "Failed to start client." << endl;
			return 1;
		}

		cout << "MTMultiplexTest.exe" << endl;
		cout << "  Examples:" << endl;
		cout << "    " << "MTMultiplexTest.exe" << " -client 127.0.0.1" << endl;
		cout << "    " << "MTMultiplexTest.exe" << " -server (0.0.0.0)" << endl;
		return 0;
	}
}