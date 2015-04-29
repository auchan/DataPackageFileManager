#include "package.h"

PackageHeader::PackageHeader()
		: id(2233)
		, fileInfoLen(0)
		, fileInfoCRC(0)
		, fileAmount(0)
		, blockSize(1024)
		, blockAmount(0)
		, dataBlockCRC(0)
{

}

Package::Package()
	: _packageHeader()
	, _fp(NULL)
{

}

Package::~Package()
{
	clear();
}

int Package::open(const std::string& packageName, const std::string& mode)
{
	_fp = fopen(packageName.c_str(), mode.c_str());
	if (NULL == _fp)
	{
		return DPFM_FILE_OPEN_FAILED;
	}
}

int Package::save()
{
	return writeFileInfo();
}

void Package::clear()
{
	if (NULL != _fp)
	{
		fclose(_fp);
		_fp = NULL;
	}
}

int Package::readFileInfo()
{
	return 0;
}

int Package::writeFileInfo()
{
	if (NULL == _fp)
	{
		return DPFM_FILE_NOT_EXIST;
	}

	if (fwrite(&(_packageHeader.id), sizeof(_packageHeader.id), 1, _fp) != 1)
	{
		return DPFM_FILE_WRITE_ERROR;
	}
}

