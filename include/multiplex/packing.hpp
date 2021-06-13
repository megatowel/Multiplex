/// @file Packing.h
/// @brief This file contains the Packer class, whch packs memory for use in replication.
#ifndef MULTIPLEXPACKING_H
#define MULTIPLEXPACKING_H

#include <array>
#include "multiplex.hpp"

#define PACK_FIELD_TYPE 0
#define PACK_FIELD_FROM_USERID 1
#define PACK_FIELD_DATA 2

namespace Megatowel
{
	namespace Multiplex
	{
		struct PackingField
		{
			uint16_t size;
			char *data;
		};

		MULTIPLEX_EXPORT std::array<PackingField, 256> unpack_fields(const char *source, const size_t size);
		MULTIPLEX_EXPORT size_t pack_field(const uint8_t fieldNum, const char *fieldData, const size_t fieldSize, const size_t offset, char *buffer);
	}
}
#endif