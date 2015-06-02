#include "PackageMgr.h"
#include "package.h"
#include "zlib128/include/zlib.h"

PackageMgr::PackageMgr()
{

}

PackageMgr::~PackageMgr()
{
	std::map<std::string, Package*>::iterator cur, end = _pakMap.end();
	for (cur = _pakMap.begin(); cur != end; ++cur)
	{
		if (cur->second != NULL)
		{
			cur->second->save();
			delete cur->second;
			cur->second = NULL;
		}
	}
}

PackageMgr& PackageMgr::getInstance()
{
	static PackageMgr mgr;
	return mgr;
}

int PackageMgr::packDir(const std::string& dirName, const std::string& pakName, bool isZip)
{
	if (_pakMap.find(pakName) != _pakMap.end())
	{
		return DPFM_PAK_ALREADY_OPEN;
	}

	Package* curPak = new Package();

	curPak->open(pakName, OpenMode::CREATE, isZip);

	_pakMap.insert(std::make_pair(pakName, curPak));

	setBrowseDir(dirName);
	if (beginBrowse("*.*") == false)
	{
		return DPFM_BROWSE_DIR_INTERRUPT;
	}
	curPak->save();

	return DPFM_OK;
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

Package* PackageMgr::getPakOfFile(const std::string& fileName)
{
	Package* curPak = NULL;
	std::string pakName = fileName.substr(0, fileName.find_first_of('/')) + ".pak";
	if (_pakMap.find(pakName) == _pakMap.end())
	{
		Package* pak = new Package();
		int ret = pak->open(pakName, OpenMode::READ_WRITE);
		if (ret != DPFM_OK)
		{
			return NULL;
		}
		curPak = pak;
		_pakMap.insert(std::make_pair(pakName, curPak));
	}
	else
	{
		curPak = _pakMap[pakName];
	}

	return curPak;
}

int PackageMgr::addFile(const std::string& fileName, const std::string& filePath)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}
	
	Package* curPak = getPakOfFile(fileName);
	if (curPak == NULL)
	{
		return DPFM_PACK_NOT_EXIST;
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

		//encrypt////////////////////////////		
		uint16_t enHeader = encryptZipHeader(*((uint16_t*) buffer2));
		memcpy(buffer2, &enHeader, sizeof(enHeader));
		////////////////////////////////////

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

int PackageMgr::addFile(const std::string& fileName, void *data, size_t size)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}

	Package* curPak = getPakOfFile(fileName);
	if (curPak == NULL)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	if (curPak->isZip())
	{
		size_t zipfsize = compressBound(size);
		uint8_t* buffer = new uint8_t[zipfsize];
		compress(buffer, (uLongf *)&zipfsize, (uint8_t*)data, size);

		//encrypt////////////////////////////		
		uint16_t enHeader = encryptZipHeader(*((uint16_t*) buffer));
		memcpy(buffer, &enHeader, sizeof(enHeader));
		////////////////////////////////////

		int ret = curPak->addFile(fileName, buffer, size, zipfsize);
		if (ret != DPFM_OK)
		{
			delete [] buffer;
			return ret;
		}
		
		curPak->save();
		delete [] buffer;
	}
	else
	{
		int ret = curPak->addFile(fileName, (uint8_t*)data, size, size);
		if (ret != DPFM_OK)
		{
			return ret;
		}
		curPak->save();
	}

	return DPFM_OK;
}


int PackageMgr::getFileData(const std::string& fileName, void *data, size_t* size)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}

	Package* curPak = getPakOfFile(fileName);
	if (curPak == NULL)
	{
		return DPFM_PACK_NOT_EXIST;
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
		return DPFM_OK;
	}

	if (curPak->isZip())
	{

		uint8_t *buffer = new uint8_t[zipfsize];
		ret = curPak->getFileData(fileName, buffer, &fsize, &zipfsize);
		if (ret != DPFM_OK)
		{
			delete [] buffer;
			return ret;
		}

		//decrypt////////////////////////////		
		uint16_t header = decryptZipHeader(*((uint16_t*) buffer));
		memcpy(buffer, &header, sizeof(header));
		////////////////////////////////////

		uncompress((uint8_t *)data, (uLongf *)&size, buffer, zipfsize);
		delete [] buffer;
	}
	else
	{
		return curPak->getFileData(fileName, data, size, &zipfsize);
	}

	return DPFM_OK;
}

int PackageMgr::deleteFile(const std::string& fileName)
{
	if (fileName.find_first_of('/') == fileName.npos)
	{
		return DPFM_FILE_PATH_INVALID;
	}

	Package* curPak = getPakOfFile(fileName);
	if (curPak == NULL)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	int ret = curPak->deleteFile(fileName);
	if (ret != DPFM_OK)
	{
		return ret;
	}
	
	return curPak->save();
}

uint16_t PackageMgr::encryptZipHeader(uint16_t header)
{
	return (header << (enBitNum - enShiftNum)) | (header >> enShiftNum);
}

uint16_t PackageMgr::decryptZipHeader(uint16_t enHeader)
{
	return (enHeader >> (enBitNum - enShiftNum)) | (enHeader << enShiftNum);
}