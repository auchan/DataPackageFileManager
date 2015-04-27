#include "pack.h"

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
{

}

Package::~Package()
{

}

int Package::readFileInfo()
{
	return 0;
}

void Package::writeFileInfo()
{

}

