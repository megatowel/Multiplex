#include "multiplex/base.hpp"
#include <enet/enet.h>

using namespace std;

namespace Megatowel
{
	namespace Multiplex
	{
		MultiplexBase::MultiplexBase() {
			init_enet();
		}

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
