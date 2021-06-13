#include "multiplex/packing.hpp"

using namespace Megatowel::Multiplex;

std::array<PackingField, 256> Megatowel::Multiplex::unpack_fields(const char *source, const size_t size)
{
	size_t readBytes = 0;
	uint8_t currentField;
	uint16_t fieldSize;

	std::array<PackingField, 256> fields;

	for (int i = 0; i < sizeof(fields) / sizeof(fields[0]); i++)
	{
		fields[i] = PackingField();
	}

	while (readBytes < size)
	{
		memcpy(&currentField, source + readBytes, sizeof(uint8_t));
		memcpy(&fieldSize, source + readBytes + 1, sizeof(uint16_t));
		fields[currentField].data = (char *)source + readBytes + 3;
		fields[currentField].size = fieldSize;
		readBytes += (size_t)fieldSize + 3;
	}

	return fields;
}

size_t Megatowel::Multiplex::pack_field(const uint8_t fieldNum, const char *fieldData, const size_t fieldSize, const size_t offset, char *buffer)
{
	uint16_t size16 = (uint16_t)fieldSize;
	memcpy((buffer + offset), &(fieldNum), sizeof(uint8_t));
	memcpy((buffer + offset + 1), &(size16), sizeof(uint16_t));
	memcpy((buffer + offset + 3), fieldData, fieldSize);
	return offset + fieldSize + 3;
}
