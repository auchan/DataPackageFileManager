#ifndef _BINARY_STREAM_H_
#define _BINARY_STREAM_H_
#include <cstdlib>
#include <cstdint>
#include <cstring>

class BinaryStream
{
public:
	BinaryStream();
	~BinaryStream();

	BinaryStream(size_t size);
	BinaryStream(const void *data, size_t size);

	inline void push(const void* data, size_t size);
	inline void pop(void* data, size_t size);
private:
	void clear();

private:
	void* _data;
	size_t _size;
	size_t _pos;
	size_t _capacity;
};

inline void BinaryStream::push(const void* data, size_t size)
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

inline void BinaryStream::pop(void* data, size_t size)
{
	memcpy(data, (uint8_t *)_data + _pos, size);
	_pos += size;
}

#endif