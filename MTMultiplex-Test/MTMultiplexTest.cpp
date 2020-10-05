#include "MTMultiplexTest.h"
#include "Client.h"
#include "Server.h"
using namespace std;
using namespace Megatowel::Multiplex;

void events_thread(MultiplexClient client)
{
	while (true) {
		MultiplexEvent mtmp_event = client.process_event(5000);
		if (mtmp_event.eventType == MultiplexEventType::UserMessage) {
			if (mtmp_event.data.size() != 0) {
				cout << mtmp_event.fromUserId << ": ";
				for (int i = 0; i <= mtmp_event.data.size() - 1; ++i)
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
				std::thread t1(events_thread, client);
				std::string message;
				while (true) {
					cout << "Type your message: ";
					getline(cin, message);
					client.send(message.data(), message.size(), 1, true);
				}
			}
		}
		cout << "MTMultiplexTest.exe" << endl;
		cout << "  Examples:" << endl;
		cout << "    " << "MTMultiplexTest.exe" << " -client 127.0.0.1" << endl;
		cout << "    " << "MTMultiplexTest.exe" << " -server (0.0.0.0)" << endl;
		return 0;
	}
}