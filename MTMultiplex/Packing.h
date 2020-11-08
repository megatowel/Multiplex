#pragma once

#include <map>
#include <iostream>
#include "MTMultiplex.h"

namespace Megatowel {
	namespace MultiplexPacking {
		struct PackingField {
			uint16_t size = 0;
			char* data = nullptr;
		};
		
		static PackingField* fields = new PackingField[256]();

		MTMULTIPLEX_EXPORT PackingField* unpack_fields(char* source, size_t size);
		MTMULTIPLEX_EXPORT size_t pack_field(uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar);
	}
}