#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "BinaryPack.h"
#include "dpfm_const.h"

struct PackageHeader
{
	uint32_t id;
	uint32_t fileInfoLen;
	uint32_t fileInfoCRC;
	uint32_t fileAmount;
	uint32_t blockSize;
	uint32_t blockAmount;
	uint32_t dataBlockCRC;
	uint8_t isZip;

	PackageHeader();
};

struct PackageFileInfo
{
	std::string fileName;
	uint32_t size;
	uint32_t zipSize;
	uint16_t zipformat;
	uint32_t crc;
	std::vector<uint32_t> blockList;

	void marshal(BinaryPack& bp) {
		bp << fileName;
		bp << size;
		bp << zipSize;
		bp << zipformat;
		bp << crc;
	}
	void unmarshal(const BinaryPack& bp) {
		bp >> fileName;
		bp >> size;
		bp >> zipSize;
		bp >> zipformat;
		bp >> crc;		
	}
};

class Package
{
public:
	Package();
	~Package();

	int open(const std::string& packageName, const std::string& mode);

	int addFile(const std::string& fileName, const std::string& filePath);

	int addFile(const std::string& fileName, void *data, size_t size);

	int alterFile(const std::string& fileName, const std::string& filePath);

	int alterFile(const std::string& fileName, void *data, size_t size);

	void getFileData(const std::string& fileName, void *data, size_t size, size_t zipSize);

	int save();

private:
	void clear();

	int readFileInfo();
	int writeFileInfo();
private:
	std::string _packageName;
	PackageHeader _packageHeader;
	std::unordered_map<std::string, PackageFileInfo*> _fileInfoMap;
	FILE* _fp;
};

#endif