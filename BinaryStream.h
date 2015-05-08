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

	BinaryStream(size_t cap);
	BinaryStream(const void *data, size_t size);

	inline int push(const void* data, size_t size);
	inline void pop(void* data, size_t size);

	void* data() { return _data; }
	size_t pos() { return _size; }
	size_t size() { return _pos; }
	size_t capacity() { return _capacity; }
private:
	void clear();

private:
	void* _data;
	size_t _size;
	size_t _pos;
	size_t _capacity;
};

inline int BinaryStream::push(const void* data, size_t size)
{
	bool resize = false;
	while (_size + size > _capacity)
	{
		_capacity += 64;
		resize = true;
	}

	if (resize)
	{
		if (NULL == _data)
		{
			//WARNING
			_data = malloc(_capacity);
		}
		else
		{
			//WARNING
			_data = realloc(_data, _capacity);
		}
	}
	if (NULL == _data)
	{
		return -1;
	}

	memcpy((uint8_t *)_data + _pos, data, size);
	_pos += size;
	_size += size;
	return 0;
}

inline void BinaryStream::pop(void* data, size_t size)
{
	memcpy(data, (uint8_t *)_data + _pos, size);
	_pos += size;
}

#endif