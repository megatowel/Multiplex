#include "Packing.h"

Megatowel::MultiplexPacking::PackingField* Megatowel::MultiplexPacking::unpack_fields(char* source, size_t size) {
	size_t readBytes = 0;
	uint8_t currentField;
	uint16_t fieldSize;

	memset(Megatowel::MultiplexPacking::fields, 0, sizeof(PackingField) * 256);

	while (readBytes < size) {
		memcpy(&currentField, source + readBytes, sizeof(uint8_t));
		memcpy(&fieldSize, source + readBytes + 1, sizeof(uint16_t));
		Megatowel::MultiplexPacking::fields[currentField].data = (char*)source + readBytes + 3;
		Megatowel::MultiplexPacking::fields[currentField].size = fieldSize;
		readBytes += (size_t)fieldSize + 3;
	}

	return Megatowel::MultiplexPacking::fields;
}

size_t Megatowel::MultiplexPacking::pack_field(uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar) {
	uint16_t size16 = (uint16_t)fieldSize;
	memcpy((destChar + currentPos), &(fieldNum), sizeof(uint8_t));
	memcpy((destChar + currentPos + 1), &(size16), sizeof(uint16_t));
	memcpy((destChar + currentPos + 3), fieldData, fieldSize);
	currentPos += fieldSize + 3;
	return currentPos;
}