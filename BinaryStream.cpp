#include "BinaryStream.h"

BinaryStream::BinaryStream()
	: _data(NULL)
	, _size(0)
	, _pos(0)
	, _capacity(0)
{

}

BinaryStream::BinaryStream(size_t size)
	: _size(0)
	, _pos(0)
	, _capacity(size)	
{
	//WARNING
	_data = malloc(size);
}

BinaryStream::~BinaryStream()
{
	clear();
}

void BinaryStream::clear()
{
	_size = 0;
	_pos = 0;
	_capacity = 0;
	if (_data != NULL)
	{
		free(_data);
		_data = NULL;
	}
}
