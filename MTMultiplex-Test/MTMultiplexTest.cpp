#include "MTMultiplexTest.h"
#include "MTMultiplex.h"
using namespace std;

int main(int argc, char** argv)
{
	cout << strcmp(argv[1], "-server") << endl;
	if (!strcmp(argv[1], "-server")) {
		cout << "we called server." << endl;
		Init_Server("0.0.0.0", 3000);
	}
	else if (!strcmp(argv[1], "-client")) {
		cout << "we called client." << endl;
		Init_Client("localhost", 3000);
	}
	cout << "we called." << endl;
	return 0;
}