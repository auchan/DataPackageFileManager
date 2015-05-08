#ifndef _PACKAGE_MGR_
#define _PACKAGE_MGR_

#include <string>
#include <map>
#include "utility/BrowseDir.h"

class Package;

class PackageMgr : public CBrowseDir
{

public:
	PackageMgr();
	~PackageMgr();

	int packDir(const std::string& dirName, const std::string& pakName, bool isZip = false);

	int addFile(const std::string& fileName, const std::string& filePath);

	int addFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

	int alterFile(const std::string& fileName, const std::string& filePath);

	int alterFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

	int getFileData(const std::string& fileName, void *data, size_t* size, size_t* zipSize);

	int deleteFile(const std::string& fileName);

private:
	virtual bool processFile(const std::string& filename) override; 

private:
	Package* _curPak;
	std::map<std::string, Package*> _pakMap;
};

#endif