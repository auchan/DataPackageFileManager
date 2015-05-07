#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <set>

#include "BinaryPack.h"
#include "dpfm_const.h"

struct PackageHeader
{
	uint32_t id;
	uint32_t fileInfoLen;
	uint32_t fileInfoCRC;
	uint16_t fileAmount;
	uint16_t blockSize;
	uint16_t blockAmount;
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
		bp << blockList;
	}
	void unmarshal(BinaryPack& bp) {
		bp >> fileName;
		bp >> size;
		bp >> zipSize;
		bp >> zipformat;
		bp >> crc;		
		bp >> blockList;
	}

	PackageFileInfo() :zipformat(0) {}
};

class Package
{
public:
	Package();
	~Package();

	int open(const std::string& packageName, const std::string& mode);

	int addFile(const std::string& fileName, const std::string& filePath);

	int addFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

	int alterFile(const std::string& fileName, const std::string& filePath);

	int alterFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

	int getFileData(const std::string& fileName, void *data, size_t* size, size_t* zipSize);

	int save();

private:
	void clear();

	int readFileInfo();
	int writeFileInfo();

	std::string formatFilePath(const std::string& path);
	uint16_t getIdleBlockIdx(uint16_t num, int startIdx = -1);

private:
	std::string _packageName;
	PackageHeader _packageHeader;

	typedef std::map<std::string, PackageFileInfo*> PackageFileInfoPtrMap;
	PackageFileInfoPtrMap _fileInfoMap;

	std::set<uint16_t> _idleBlockSet;

	FILE* _fp;
	static const size_t HEADER_SIZE = 64;
};

#endif