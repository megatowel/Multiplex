#include "MTMultiplexTest.h"
#include "MTMultiplex.h"
using namespace std;

int main(int argc, char** argv)
{
	if (Init_ENet() == 0) {
		cout << strcmp(argv[1], "-server") << endl;
		if (!strcmp(argv[1], "-server")) {
			cout << "we called server." << endl;
			
			Multiplex server;
			server.Setup_Host(true, "0.0.0.0", 3000);
			server.Process_Event();
			server.Process_Event();
		}
		else {
			cout << "we called client." << endl;
			Multiplex client;
			client.Setup_Host(false, NULL, NULL);
			client.Client_Connect("localhost", 3000);
		}
		cout << "we called." << endl;
		return 0;
	}
}