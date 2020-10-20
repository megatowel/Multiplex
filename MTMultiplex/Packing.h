#pragma once

#include <map>
#include <iostream>
#include "MTMultiplex.h"

namespace Megatowel {
	namespace MultiplexPacking{
		MTMULTIPLEX_EXPORT std::map<unsigned int, std::pair<char*, size_t>> unpack_fields(char* source, size_t size);
		MTMULTIPLEX_EXPORT size_t pack_field(uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar);
	}
}