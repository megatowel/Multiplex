#include "MTMultiplexTest.h"
#include "MTMultiplex.h"
using namespace std;

void Events_Thread(Multiplex client)
{
	while (true) {
		client.Process_Event();
	}
}

int main(int argc, char** argv)
{
	if (Init_ENet() == 0) {
		cout << strcmp(argv[1], "-server") << endl;
		if (!strcmp(argv[1], "-server")) {
			cout << "we called server." << endl;
			
			Multiplex server;
			server.Setup_Host(true, "0.0.0.0", 3000);
			while (true) {
				server.Process_Server_Event();
			}
		}
		else {
			cout << "we called client." << endl;
			Multiplex client;
			client.Setup_Host(false, NULL, NULL);
			client.Client_Connect("localhost", 3000);
			std::thread t1(Events_Thread, client);
			std::string message;
			while (true) {
				cout << "Type your message: ";
				getline(cin, message);
				client.Send(message.data(), message.size(), 1, true);
			}
		}
		cout << "we called." << endl;
		return 0;
	}
}