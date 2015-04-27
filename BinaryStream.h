#ifndef _BINARY_STREAM_H_
#define _BINARY_STREAM_H_
#include <cstdlib>

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

#endif