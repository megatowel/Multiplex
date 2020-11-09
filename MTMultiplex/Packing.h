#pragma once

#include <map>
#include <iostream>
#include "MTMultiplex.h"

namespace Megatowel {
	namespace MultiplexPacking {
		class Packing
		{
		public:
			MTMULTIPLEX_EXPORT Packing();
			MTMULTIPLEX_EXPORT ~Packing();
			MTMULTIPLEX_EXPORT PackingField* unpack_fields(char* source, size_t size);
			MTMULTIPLEX_EXPORT size_t pack_field(uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar);
			PackingField fields[256];
		};
		
	}
}