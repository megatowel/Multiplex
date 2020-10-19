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

size_t Megatowel::MultiplexPacking::pack_fields(std::map<unsigned int, std::pair<char*,size_t>> fields, char* destChar) {
	unsigned int currentField = 0;
	unsigned int currentPos = 0;
	uint8_t fieldNum;
	uint16_t fieldSize;
	for (std::map<unsigned int, std::pair<char*, size_t>>::iterator it = fields.begin();
		it != fields.end(); ++it) {
		fieldNum = (uint8_t)it->first;
		fieldSize = (uint16_t)it->second.second;
		memcpy((destChar + currentPos), &(fieldNum), sizeof(uint8_t));
		memcpy((destChar + currentPos + 1), &(fieldSize), sizeof(uint16_t));
		memcpy((destChar + currentPos + 3), it->second.first, it->second.second);
		currentField += 1;
		currentPos += it->second.second + 3;
	}
	return (size_t)currentPos;
}