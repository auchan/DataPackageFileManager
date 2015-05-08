#include "PackageMgr.h"
#include "package.h"
#include "zlib128/include/zlib.h"

PackageMgr::PackageMgr()
	: _curPak(NULL)
{

}

PackageMgr::~PackageMgr()
{
	std::map<std::string, Package*>::iterator cur, end = _pakMap.end();
	for (cur = _pakMap.begin(); cur != end; ++cur)
	{
		if (cur->second != NULL)
		{
			delete cur->second;
			cur->second = NULL;
		}
	}
}

int PackageMgr::packDir(const std::string& dirName, const std::string& pakName, bool isZip)
{
	if (_curPak != NULL)
	{
		// do something
		return -1;
	}

	_curPak = new Package();

	_curPak->open(pakName, OpenMode::CREATE, isZip);

	_pakMap.insert(std::make_pair(pakName, _curPak));

	setBrowseDir(dirName);
	if (beginBrowse("*.*") == true)
	{

	}

	_curPak->save();
}

bool PackageMgr::processFile(const std::string& filename)
{

	int ret = PackageMgr::addFile(filename, filename);

	if (ret == DPFM_OK)
	{
		return true;
	}

	return false;
}

int PackageMgr::addFile(const std::string& fileName, const std::string& filePath)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}

	std::string pakName = fileName.substr(0, fileName.find_first_of('/')) + ".pak";

	Package* curPak;
	if (_pakMap.find(pakName) == _pakMap.end())
	{
		Package* pak = new Package();
		int ret = pak->open(pakName, OpenMode::READ_WRITE);
		if (ret != DPFM_OK)
		{
			return ret;
		}
		curPak = pak;
	}
	else
	{
		curPak = _pakMap[pakName];
	}

	if (curPak->isZip())
	{
		ScopeFile sf(filePath);

		fseek(sf._fp, 0, SEEK_END);
		size_t fsize = ftell(sf._fp);
		fseek(sf._fp, 0, SEEK_SET);

		uint8_t* buffer = new uint8_t[fsize];
		if (NULL == buffer)
		{
			return DPFM_MEM_ALLOC_ERROR;
		}

		if (fread(buffer, fsize, 1, sf._fp) != 1)
		{
			delete [] buffer;
			return DPFM_FILE_READ_ERROR;
		}

		size_t zipfsize = compressBound(fsize);
		uint8_t* buffer2 = new uint8_t[zipfsize];
		compress(buffer2, (uLongf *)&zipfsize, buffer, fsize);
		delete [] buffer;

		int ret = curPak->addFile(fileName, buffer2, fsize, zipfsize);
		curPak->save();

		delete [] buffer2;
	}
	else
	{
		int ret = curPak->addFile(fileName, filePath);
		curPak->save();
	}

	return DPFM_OK;
}

int addFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

int alterFile(const std::string& fileName, const std::string& filePath);

int alterFile(const std::string& fileName, void *data, size_t size, size_t zipSize);

int PackageMgr::getFileData(const std::string& fileName, void *data, size_t* size, size_t* zipSize)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}

	std::string pakName = fileName.substr(0, fileName.find_first_of('/')) + ".pak";

	Package* curPak;
	if (_pakMap.find(pakName) == _pakMap.end())
	{
		Package* pak = new Package();
		int ret = pak->open(pakName, OpenMode::READ_WRITE);
		if (ret != DPFM_OK)
		{
			return ret;
		}
		curPak = pak;
	}
	else
	{
		curPak = _pakMap[pakName];
	}

	size_t fsize, zipfsize;
	int ret = curPak->getFileData(fileName, NULL, &fsize, &zipfsize);
	if (ret != DPFM_OK)
	{
		return ret;
	}

	if (data == NULL)
	{
		*size = fsize;
		*zipSize = zipfsize;
		return DPFM_OK;
	}

	uint8_t *buffer = new uint8_t[zipfsize];
	ret = curPak->getFileData(fileName, buffer, &fsize, &zipfsize);
	if (ret != DPFM_OK)
	{
		return ret;
	}

	if (curPak->isZip())
	{
		uint8_t *buffer2 = new uint8_t[fsize];
		uncompress(buffer2, (uLongf *)&fsize, buffer, zipfsize);
		memcpy(data, buffer2, fsize);

		delete [] buffer2;
	}
	else
	{
		memcpy(data, buffer, zipfsize);
	}

	delete [] buffer;
}

int deleteFile(const std::string& fileName);