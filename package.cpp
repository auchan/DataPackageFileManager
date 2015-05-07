#include "package.h"
#include "ScopeFile.h"
#include "zlib128/include/zlib.h"
#include <cmath>

PackageHeader::PackageHeader()
		: id(2233)
		, fileInfoLen(0)
		, fileInfoCRC(0)
		, fileAmount(0)
		, blockSize(1024)
		, blockAmount(0)
		, isZip(0)
{

}

Package::Package()
	: _packageHeader()
	, _fp(NULL)
{

}

Package::~Package()
{
	close();
}

int Package::open(const std::string& packageName, OpenMode mode)
{
	std::string modeStr;
	if (mode == OpenMode::CREATE)
	{
		modeStr = "wb";
	}
	else if (mode == OpenMode::READ_ONLY)
	{
		modeStr = "rb";
	}
	else
	{
		modeStr = "rb+";
	}

	_fp = fopen(packageName.c_str(), modeStr.c_str());
	if (NULL == _fp)
	{
		return DPFM_FILE_OPEN_FAILED;
	}

	_packageName = packageName;

	if (mode != OpenMode::CREATE)
	{
		return readFileInfo();
	}
	return DPFM_OK;
}

int Package::save()
{
	return writeFileInfo();
}

void Package::close()
{
	if (NULL != _fp)
	{
		fclose(_fp);
		_fp = NULL;
	}

	PackageFileInfoPtrMap::iterator cur, end = _fileInfoMap.end();
	for (cur = _fileInfoMap.begin(); cur != end; ++cur)
	{
		delete cur->second;
	}

	_fileInfoMap.clear();
}

int Package::addFile(const std::string& fileName, const std::string& filePath)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) != _fileInfoMap.end())
	{
		return alterFile(formattedFileName, filePath);
	}

	ScopeFile sfile(filePath);
	if (NULL == sfile._fp)
	{
		return DPFM_FILE_NOT_EXIST;
	}

	PackageFileInfo* fileInfo = new PackageFileInfo();
	if (NULL == fileInfo)
	{
		return DPFM_MEM_ALLOC_ERROR;
	}

	fseek(sfile._fp, 0, SEEK_END);
	fileInfo->size = ftell(sfile._fp);
	fileInfo->zipSize = fileInfo->size;
	fileInfo->fileName = formattedFileName;

	uint8_t *buffer = new uint8_t[_packageHeader.blockSize];
	fseek(sfile._fp, 0, SEEK_SET);

	uint16_t blockNum = std::ceil((double)fileInfo->zipSize / _packageHeader.blockSize);
	uint16_t startIdx = getIdleBlockIdx(blockNum);
	fseek(_fp, HEADER_SIZE + startIdx * _packageHeader.blockSize, SEEK_SET);
	
	uLong crc = adler32(NULL, Z_NULL, NULL);
	uint16_t blockCount = 0;
	uint32_t fileSize = fileInfo->size;
	while (!feof(sfile._fp) && fileSize != 0)
	{
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		if (fread(buffer, readSize, 1, sfile._fp) < 1)
		{
			delete [] buffer;
			delete fileInfo;
			return DPFM_FILE_READ_ERROR;
		}

		crc = adler32(crc, buffer, readSize);

		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete [] buffer;
			delete fileInfo;
			return DPFM_FILE_READ_ERROR;
		}

		uint32_t blockDesc = ((startIdx + blockCount) << 16) | (readSize & 0x0000FFFF);
		fileInfo->blockList.push_back(blockDesc);

		std::set<uint16_t>::const_iterator idleIdx = _idleBlockSet.find(startIdx + blockCount);
		if (idleIdx != _idleBlockSet.end())
		{
			_idleBlockSet.erase(idleIdx);
		}


		if (startIdx + blockCount == _packageHeader.blockAmount)
		{
			_packageHeader.blockAmount += 1;
		}
		blockCount += 1;
	}

	fileInfo->crc = crc;
	_fileInfoMap.insert(std::make_pair(formattedFileName, fileInfo));
	_packageHeader.fileAmount += 1;

	delete [] buffer;

	return DPFM_OK;
}

int Package::addFile(const std::string& fileName, void *data, size_t size, size_t zipSize)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) != _fileInfoMap.end())
	{
		return alterFile(formattedFileName, data, size, zipSize);
	}

	PackageFileInfo* fileInfo = new PackageFileInfo();
	if (NULL == fileInfo)
	{
		return DPFM_MEM_ALLOC_ERROR;
	}

	fileInfo->size = size;
	fileInfo->zipSize = zipSize;
	fileInfo->fileName = formattedFileName;

	uint16_t blockNum = std::ceil((double)fileInfo->zipSize / _packageHeader.blockSize);
	uint16_t startIdx = getIdleBlockIdx(blockNum);
	fseek(_fp, HEADER_SIZE + startIdx * _packageHeader.blockSize, SEEK_SET);
	
	uLong crc = adler32(NULL, Z_NULL, NULL);
	uint8_t *buffer = NULL;
	uint16_t blockCount = 0;
	uint32_t fileSize = fileInfo->zipSize;
	while (fileSize != 0)
	{
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		buffer = (uint8_t*)data + blockCount*_packageHeader.blockSize;
		crc = adler32(crc, buffer, readSize);

		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete fileInfo;
			return DPFM_FILE_READ_ERROR;
		}

		uint32_t blockDesc = ((startIdx + blockCount) << 16) | (readSize & 0x0000FFFF);
		fileInfo->blockList.push_back(blockDesc);

		std::set<uint16_t>::const_iterator idleIdx = _idleBlockSet.find(startIdx + blockCount);
		if (idleIdx != _idleBlockSet.end())
		{
			_idleBlockSet.erase(idleIdx);
		}


		if (startIdx + blockCount == _packageHeader.blockAmount)
		{
			_packageHeader.blockAmount += 1;
		}
		blockCount += 1;
	}

	fileInfo->crc = crc;
	_fileInfoMap.insert(std::make_pair(formattedFileName, fileInfo));
	_packageHeader.fileAmount += 1;

	return DPFM_OK;
}

int Package::alterFile(const std::string& fileName, const std::string& filePath)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) == _fileInfoMap.end())
	{
		return DPFM_FILE_NOT_EXIST;
	}

	ScopeFile sfile(filePath);
	if (NULL == sfile._fp)
	{
		return DPFM_FILE_NOT_EXIST;
	}

	PackageFileInfo* fileInfo = _fileInfoMap[formattedFileName];

	fseek(sfile._fp, 0, SEEK_END);
	fileInfo->size = ftell(sfile._fp);
	fileInfo->zipSize = fileInfo->size;

	uint8_t *buffer = new uint8_t[_packageHeader.blockSize];
	fseek(sfile._fp, 0, SEEK_SET);

	uLong crc = adler32(NULL, Z_NULL, NULL);
	uint32_t fileSize = fileInfo->size;
	std::vector<uint32_t>::iterator cur, end = fileInfo->blockList.end();
	for (cur = fileInfo->blockList.begin(); cur != end; ++cur)
	{
		uint16_t blockIdx = (*cur) >> 16;
		uint16_t len = (*cur) & 0x0000FFFF;
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		if (fread(buffer, readSize, 1, sfile._fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_READ_ERROR;
		}

		crc = adler32(crc, buffer, readSize);

		fseek(_fp, HEADER_SIZE + blockIdx * _packageHeader.blockSize, SEEK_SET);
		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_WRITE_ERROR;
		}

		uint32_t blockDesc = (blockIdx << 16) | (readSize & 0x0000FFFF);
		*cur = blockDesc;

		if (fileSize == 0)
		{
			std::vector<uint32_t>::iterator un = ++cur;
			for (un; un != end; ++un)
			{
				blockIdx = (*un) >> 16;
				_idleBlockSet.insert(blockIdx);
			}
			fileInfo->blockList.erase(cur, end);
			break;
		}
	}
	

	uint16_t blockNum = std::ceil((double)fileSize / _packageHeader.blockSize);
	uint16_t startIdx = getIdleBlockIdx(blockNum);
	fseek(_fp, HEADER_SIZE + startIdx * _packageHeader.blockSize, SEEK_SET);
	
	uint16_t blockCount = 0;
	while (!feof(sfile._fp) && fileSize != 0)
	{
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		if (fread(buffer, readSize, 1, sfile._fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_READ_ERROR;
		}

		crc = adler32(crc, buffer, readSize);

		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_WRITE_ERROR;
		}

		uint32_t blockDesc = ((startIdx + blockCount) << 16) | (readSize & 0x0000FFFF);
		fileInfo->blockList.push_back(blockDesc);

		std::set<uint16_t>::const_iterator idleIdx = _idleBlockSet.find(startIdx + blockCount);
		if (idleIdx != _idleBlockSet.end())
		{
			_idleBlockSet.erase(idleIdx);
		}

		if (startIdx + blockCount == _packageHeader.blockAmount)
		{
			_packageHeader.blockAmount += 1;
		}
		blockCount += 1;
	}

	fileInfo->crc = crc;

	delete [] buffer;

	return DPFM_OK;
}

int Package::alterFile(const std::string& fileName, void *data, size_t size, size_t zipSize)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) == _fileInfoMap.end())
	{
		return DPFM_FILE_NOT_EXIST;
	}

	PackageFileInfo* fileInfo = _fileInfoMap[formattedFileName];

	fileInfo->size = size;
	fileInfo->zipSize = zipSize;

	uint8_t *buffer;
	uint16_t blockCount = 0;
	uLong crc = adler32(NULL, Z_NULL, NULL);
	uint32_t fileSize = fileInfo->zipSize;
	std::vector<uint32_t>::iterator cur, end = fileInfo->blockList.end();
	for (cur = fileInfo->blockList.begin(); cur != end; ++cur)
	{
		uint16_t blockIdx = (*cur) >> 16;
		uint16_t len = (*cur) & 0x0000FFFF;
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		buffer = (uint8_t*)data + blockCount*_packageHeader.blockSize;
		crc = adler32(crc, buffer, readSize);

		fseek(_fp, HEADER_SIZE + blockIdx * _packageHeader.blockSize, SEEK_SET);
		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_WRITE_ERROR;
		}

		uint32_t blockDesc = (blockIdx << 16) | (readSize & 0x0000FFFF);
		*cur = blockDesc;
		
		blockCount += 1;

		if (fileSize == 0)
		{
			std::vector<uint32_t>::iterator un = ++cur;
			for (un; un != end; ++un)
			{
				blockIdx = (*un) >> 16;
				_idleBlockSet.insert(blockIdx);
			}
			fileInfo->blockList.erase(cur, end);
			break;
		}

	}
	

	uint16_t blockNum = std::ceil((double)fileSize / _packageHeader.blockSize);
	uint16_t startIdx = getIdleBlockIdx(blockNum);
	fseek(_fp, HEADER_SIZE + startIdx * _packageHeader.blockSize, SEEK_SET);
	
	uint16_t blockCount2 = 0;
	while (fileSize != 0)
	{
		uint32_t readSize;
		if (fileSize > _packageHeader.blockSize)
		{
			readSize = _packageHeader.blockSize;
			fileSize -= _packageHeader.blockSize;
		}
		else
		{
			readSize = fileSize;
			fileSize = 0;
		}

		buffer = (uint8_t*)data + (blockCount + blockCount2)*_packageHeader.blockSize;

		crc = adler32(crc, buffer, readSize);

		if (fwrite(buffer, readSize, 1, _fp) < 1)
		{
			delete [] buffer;
			return DPFM_FILE_WRITE_ERROR;
		}

		uint32_t blockDesc = ((startIdx + blockCount2) << 16) | (readSize & 0x0000FFFF);
		fileInfo->blockList.push_back(blockDesc);

		std::set<uint16_t>::const_iterator idleIdx = _idleBlockSet.find(startIdx + blockCount2);
		if (idleIdx != _idleBlockSet.end())
		{
			_idleBlockSet.erase(idleIdx);
		}

		if (startIdx + blockCount2 == _packageHeader.blockAmount)
		{
			_packageHeader.blockAmount += 1;
		}
		blockCount2 += 1;
	}

	fileInfo->crc = crc;

	return DPFM_OK;
}

int Package::getFileData(const std::string& fileName, void *data, size_t* size, size_t* zipSize)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) == _fileInfoMap.end())
	{
		return DPFM_FILE_NOT_IN_PACK;
	}

	PackageFileInfo *fileInfo =  _fileInfoMap[formattedFileName];
		
	*size = fileInfo->size;
	*zipSize = fileInfo->zipSize;
	if (NULL == data)
	{
		return DPFM_OK;
	}

	uint8_t *buffer = new uint8_t[_packageHeader.blockSize];
	size_t offset = 0;
	std::vector<uint32_t>::iterator cur, end = fileInfo->blockList.end();
	for (cur = fileInfo->blockList.begin(); cur != end; ++cur)
	{
		uint16_t blockIdx = (*cur) >> 16;
		uint16_t len = (*cur) & 0x0000FFFF;

		fseek(_fp, HEADER_SIZE + blockIdx * _packageHeader.blockSize, SEEK_SET);

		if (fread(buffer, len, 1, _fp) != 1)
		{
			delete [] buffer;
			return DPFM_FILE_READ_ERROR;
		}

		memcpy((uint8_t *)data + offset, buffer, len);
		offset += len;
	}
	delete [] buffer;

	return DPFM_OK;
}

int Package::deleteFile(const std::string& fileName)
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	std::string formattedFileName = formatFilePath(fileName);
	if (_fileInfoMap.find(formattedFileName) == _fileInfoMap.end())
	{
		return DPFM_FILE_NOT_IN_PACK;
	}

	PackageFileInfo *fileInfo =  _fileInfoMap[formattedFileName];
	std::vector<uint32_t>::iterator cur, end = fileInfo->blockList.end();
	for (cur = fileInfo->blockList.begin(); cur != end; ++cur)
	{
		uint16_t blockIdx = (*cur) >> 16;
		uint16_t len = (*cur) & 0x0000FFFF;

		_idleBlockSet.insert(blockIdx);
	}

	_fileInfoMap.erase(_fileInfoMap.find(formattedFileName));
	_packageHeader.fileAmount -= 1;
}

int Package::readFileInfo()
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	int ret = DPFM_OK;
	do
	{
		fseek(_fp, 0, SEEK_SET);

		if (fread(&(_packageHeader.id), sizeof(_packageHeader.id), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.fileInfoLen), sizeof(_packageHeader.fileInfoLen), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.fileInfoCRC), sizeof(_packageHeader.fileInfoCRC), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.fileAmount), sizeof(_packageHeader.fileAmount), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.blockSize), sizeof(_packageHeader.blockSize), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.blockAmount), sizeof(_packageHeader.blockAmount), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		if (fread(&(_packageHeader.isZip), sizeof(_packageHeader.isZip), 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		size_t blockDataLen = _packageHeader.blockSize * _packageHeader.blockAmount;

		fseek(_fp, HEADER_SIZE + blockDataLen, SEEK_SET);

		uint8_t *buffer = new uint8_t[_packageHeader.fileInfoLen];
		if (fread(buffer, _packageHeader.fileInfoLen, 1, _fp) != 1)
		{
			ret = DPFM_FILE_READ_ERROR;
			break;
		}

		BinaryPack bp(buffer, _packageHeader.fileInfoLen);
		delete [] buffer;
		_fileInfoMap.clear();
		
		for (uint16_t i = 1; i <= _packageHeader.fileAmount; ++i)
		{
			PackageFileInfo *fileInfo = new PackageFileInfo();
			fileInfo->unmarshal(bp);
			_fileInfoMap.insert(std::make_pair(fileInfo->fileName, fileInfo));
		}

		bp >> _idleBlockSet;
	}
	while (0);

	return ret;
}

int Package::writeFileInfo()
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	int ret = DPFM_OK;
	do
	{
		fseek(_fp, 0, SEEK_SET);

		if (fwrite(&(_packageHeader.id), sizeof(_packageHeader.id), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		BinaryPack bp(1024);
		PackageFileInfoPtrMap::iterator cur = _fileInfoMap.begin();
		PackageFileInfoPtrMap::iterator end = _fileInfoMap.end();
		for (cur; cur != end; ++cur)
		{
			cur->second->marshal(bp);
		}
		bp << _idleBlockSet;

		_packageHeader.fileInfoLen = bp.GetDataSize();
		if (fwrite(&(_packageHeader.fileInfoLen), sizeof(_packageHeader.fileInfoLen), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		uLong crc = adler32(NULL, Z_NULL, NULL);
		crc = (crc, bp.GetData(), bp.GetDataSize());
		_packageHeader.fileInfoCRC = crc;
		if (fwrite(&(_packageHeader.fileInfoCRC), sizeof(_packageHeader.fileInfoCRC), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		if (fwrite(&(_packageHeader.fileAmount), sizeof(_packageHeader.fileAmount), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		if (fwrite(&(_packageHeader.blockSize), sizeof(_packageHeader.blockSize), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		if (fwrite(&(_packageHeader.blockAmount), sizeof(_packageHeader.blockAmount), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		if (fwrite(&(_packageHeader.isZip), sizeof(_packageHeader.isZip), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}

		size_t blockDataLen = _packageHeader.blockSize * _packageHeader.blockAmount;

		fseek(_fp, HEADER_SIZE + blockDataLen, SEEK_SET);
		if (fwrite(bp.GetData(), bp.GetDataSize(), 1, _fp) != 1)
		{
			ret = DPFM_FILE_WRITE_ERROR;
			break;
		}
	}
	while (0);

	return ret;
}

std::string Package::formatFilePath(const std::string& path)
{
	std::string formatPath = path;
	std::string::iterator cur = formatPath.begin();
	std::string::iterator end = formatPath.end();
	for (cur; cur != end; ++cur)
	{
		if (*cur == '\\')
		{
			*cur = '/';
		}
	}

	return formatPath;
}

uint16_t Package::getIdleBlockIdx(uint16_t num, int startIdx)
{
	if (num > _idleBlockSet.size() || num <= 0)
	{
		return _packageHeader.blockAmount;
	}

	std::set<uint16_t>::iterator cur;
	std::set<uint16_t>::iterator pre = _idleBlockSet.begin(), end = _idleBlockSet.end();
	uint16_t count = 0;
	for (cur = pre; cur != end; ++cur)
	{
		if (*cur < startIdx)
			continue;

		if (cur == pre || *cur - *(pre) == 1)
		{
			count += 1;
		}
		else
		{
			count = 1;
		}


		if (count == num)
		{
			return *cur;
		}

		pre = cur;
	}

	return _packageHeader.blockAmount;
}

int Package::reorganize()
{
	if (NULL == _fp)
	{
		return DPFM_PACK_NOT_EXIST;
	}

	Package reoPak;
	if (reoPak.open(_packageName + ".reo", OpenMode::CREATE) != DPFM_OK)
	{
		return DPFM_FILE_OPEN_FAILED;
	}

	PackageFileInfoPtrMap::iterator cur, end = _fileInfoMap.end();
	size_t size, zipSize;
	for (cur = _fileInfoMap.begin(); cur != end; ++cur)
	{
		getFileData(cur->first, NULL, &size, &zipSize);
		uint8_t *data = new uint8_t[zipSize];
		if (NULL == data)
		{
			return DPFM_MEM_ALLOC_ERROR;
		}
		getFileData(cur->first, data, &size, &zipSize);
		reoPak.addFile(cur->first, data, size, zipSize);
		delete [] data;
	}
	reoPak.save();
	reoPak.close();

	this->close();
	remove(_packageName.c_str());
	rename((_packageName + ".reo").c_str(), _packageName.c_str());

	return DPFM_OK;
}

