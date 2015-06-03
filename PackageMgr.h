#ifndef _PACKAGE_MGR_
#define _PACKAGE_MGR_

#include <string>
#include <map>
#include "utility/BrowseDir.h"
#include <cstdint>

class Package;

class PackageMgr : public CBrowseDir
{

public:

	static PackageMgr& getInstance();

	int packDir(const std::string& dirName, const std::string& pakName, bool isZip = false);

	int addFile(const std::string& fileName, const std::string& filePath);

	int addFile(const std::string& fileName, void *data, size_t size);

	int getFileData(const std::string& fileName, void *data, size_t *size);

	int deleteFile(const std::string& fileName);

private:
	PackageMgr();
	~PackageMgr();
	PackageMgr(const PackageMgr&);
	PackageMgr& operator=(const PackageMgr&);

private:
	virtual bool processFile(const std::string& filename) override; 

	Package* getPakOfFile(const std::string& filename);

	uint16_t encryptZipHeader(uint16_t header);

	uint16_t decryptZipHeader(uint16_t enHeader);
private:
	std::map<std::string, Package*> _pakMap;

	static const int enBitNum = 16;
	static const int enShiftNum = 3;
};

#endif