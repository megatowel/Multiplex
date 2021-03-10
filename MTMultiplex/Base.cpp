#include "Base.h"
#include "MTMultiplex.h"
#include <enet/enet.h>

using namespace std;

namespace Megatowel
{
	namespace Multiplex
	{
		int init_enet()
		{
			if (enet_initialize() != 0)
			{
				fprintf(stderr, "An error occurred while initializing ENet.\n");
				return EXIT_FAILURE;
			}
			atexit(enet_deinitialize);
			return 0;
		}
	}
}
