#include "BinaryStream.h"
#include <cstdint>
#include <cstring>

BinaryStream::BinaryStream()
	: _data(NULL)
	, _size(0)
	, _pos(0)
	, _capacity(0)
{

}

BinaryStream::BinaryStream(size_t size)
	: _size(size)
	, _pos(0)
	, _capacity(0)	
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

void BinaryStream::push(const void* data, size_t size)
{
	bool resize = false;
	while (_size + size > _capacity)
	{
		_capacity += 64;
		resize = true;
	}

	if (resize)
	{
		//WARNING
		realloc(_data, _capacity);
	}

	memcpy((uint8_t *)_data + _pos, data, size);
	_pos += size;
	_size += size;
}

void BinaryStream::pop(void* data, size_t size)
{
	memcpy(data, (uint8_t *)_data + _pos, size);
	_pos += size;
}