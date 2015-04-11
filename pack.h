#ifndef _PACK_H_
#define _PACK_H_

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

enum class OpenMode
{
	CREATE, ALTER, READ
};

class PackageHeader
{
public:
	uint32_t id;
	uint32_t fileInfoLen;
	uint32_t fileInfoCRC;
	uint32_t fileAmount;
	uint32_t blockSize;
	uint32_t blockAmount;
	uint32_t dataBlockCRC;
	uint8_t isZip;
};

class PackageFileInfo
{
public:
	std::string fileName;
	uint32_t size;
	uint32_t zipSize;
	uint32_t crc;
	std::vector<uint32_t> blockList;
};

class Package
{
public:
	Package();
	~Package();

	int open(const std::string& packageName, OpenMode mode);

	int addFile(const std::string& fileName, const std::string& filePath);

	int addFile(const std::string& fileName, void *data, size_t size);

	int alterFile(const std::string& fileName, const std::string& filePath);

	int alterFile(const std::string& fileName, void *data, size_t size);

	void getFileData(const std::string& fileName, void *data, size_t size, size_t zipSize);

private:
	std::string _packageName;
	PackageHeader _packageHeader;
	std::unordered_map<std::string, PackageFileInfo*> _fileInfoMap;
};

#endif