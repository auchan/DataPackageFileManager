#include "BinaryStream.h"

BinaryStream::BinaryStream()
	: _data(NULL)
	, _size(0)
	, _pos(0)
	, _capacity(0)
{

}

BinaryStream::BinaryStream(size_t cap)
	: _size(0)
	, _pos(0)
	, _capacity(cap)	
{
	//WARNING
	_data = malloc(cap);
}

BinaryStream::BinaryStream(const void *data, size_t size)
	: _pos(0)
	, _size(size)
	, _capacity(size)
{
	//WARNING
	_data = malloc(size);
	memcpy(_data, data, size);
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
