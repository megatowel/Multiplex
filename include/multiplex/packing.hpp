/// @file Packing.h
/// @brief This file contains the Packer class, whch packs memory for use in replication.
#ifndef MULTIPLEXPACKING_H
#define MULTIPLEXPACKING_H

#include <map>
#include <iostream>
#include "multiplex.hpp"

namespace Megatowel
{
	namespace Multiplex
	{
		class Packer
		{
		public:
			MULTIPLEX_EXPORT Packer();
			MULTIPLEX_EXPORT PackingField *unpack_fields(char *source, size_t size);
			MULTIPLEX_EXPORT size_t pack_field(uint8_t fieldNum, char *fieldData, size_t fieldSize, size_t currentPos, char *destChar);
		};
	}
}
#endif