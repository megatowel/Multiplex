#include "Packing.h"

std::map<unsigned int, std::pair<char*, size_t>> Megatowel::MultiplexPacking::unpack_fields(char* source, size_t size) {
	size_t readBytes = 0;
	uint8_t fieldNum;
	uint16_t fieldSize = 0;
	std::map<unsigned int, std::pair<char*, size_t>> readFields = std::map<unsigned int, std::pair<char*, size_t>>();
	while (readBytes < size) {
		memcpy(&fieldNum, source + readBytes, sizeof(uint8_t));
		memcpy(&fieldSize, source + readBytes + 1, sizeof(uint16_t));
		readFields.insert(std::pair<unsigned int, std::pair<char*, size_t>>(fieldNum, std::pair<char*, size_t>((char*)source+readBytes+3, (size_t)fieldSize)));
		readBytes += (size_t)fieldSize + 3;
	}
	return readFields;
}

size_t Megatowel::MultiplexPacking::pack_field(uint8_t fieldNum, char* fieldData, size_t fieldSize, size_t currentPos, char* destChar) {
	uint16_t size16 = (uint16_t)fieldSize;
	memcpy((destChar + currentPos), &(fieldNum), sizeof(uint8_t));
	memcpy((destChar + currentPos + 1), &(size16), sizeof(uint16_t));
	memcpy((destChar + currentPos + 3), fieldData, fieldSize);
	currentPos += fieldSize + 3;
	return currentPos;
}