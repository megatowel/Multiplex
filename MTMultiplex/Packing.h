#pragma once

#include <map>
#include <iostream>
#include "MTMultiplex.h"

namespace Megatowel {
	namespace MultiplexPacking{
		MTMULTIPLEX_EXPORT std::map<unsigned int, std::pair<char*, size_t>> unpack_fields(char* source, size_t size);
		MTMULTIPLEX_EXPORT size_t pack_fields(std::map<unsigned int, std::pair<char*, size_t>> fields, char* destChar);
	}
}