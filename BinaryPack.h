#ifndef _BINARY_PACK_H_
#define _BINARY_PACK_H_

#include "BinaryStream.h"

class BinaryPack
{
public:
	BinaryPack(): _pos(0) {}
	BinaryPack(size_t cap): _pos(0), _streamData(cap) {}
	BinaryPack(const void* data, size_t size): _pos(0), _streamData(data, size) {}

	BinaryPack& operator<< (uint8_t);
	BinaryPack& operator<< (uint16_t);
	BinaryPack& operator<< (uint32_t);
	BinaryPack& operator<< (uint64_t);
	BinaryPack& operator<< (const std::string&);
	template<typename T>
	BinaryPack& operator<< (const std::vector<T> vec);

	BinaryPack& operator>> (uint8_t&);
	BinaryPack& operator>> (uint16_t&);
	BinaryPack& operator>> (uint32_t&);
	BinaryPack& operator>> (uint64_t&);
	BinaryPack& operator>> (std::string&);
	template<typename T>
	BinaryPack& operator>> (std::vector<T>&);

	void* GetData() { return _streamData.data(); }
	size_t GetDataSize() { return _streamData.size(); }
private:
	BinaryStream _streamData;
	size_t _pos;
};

inline BinaryPack& BinaryPack::operator<<(uint8_t data)
{
	_streamData.push(&data, 1);
	return *this;
}

inline BinaryPack& BinaryPack::operator<<(uint16_t data)
{
	_streamData.push(&data, 2);
	return *this;
}

inline BinaryPack& BinaryPack::operator<<(uint32_t data)
{
	_streamData.push(&data, 4);
	return *this;
}

inline BinaryPack& BinaryPack::operator<<(uint64_t data)
{
	_streamData.push(&data, 8);
	return *this;
}

inline BinaryPack& BinaryPack::operator<<(const std::string& data)
{
	uint16_t strLen = data.length();
	*this << strLen;
	for (size_t i = 0; i < strLen; ++i)
	{
		*this << (uint8_t)data[i];
	}

	return *this;
}

template<typename T>
inline BinaryPack& BinaryPack::operator<<(const std::vector<T> vec)
{
	*this << (uint16_t)vec.size();

	std::vector<T>::const_iterator cur, end = vec.end();
	for (cur = vec.begin(); cur != end; ++cur)
	{
		*this << *cur;
	}

	return *this;
}

inline BinaryPack& BinaryPack::operator>>(uint8_t& data)
{
	_streamData.pop(&data, 1);
	return *this;
}

inline BinaryPack& BinaryPack::operator>>(uint16_t& data)
{
	_streamData.pop(&data, 2);
	return *this;
}

inline BinaryPack& BinaryPack::operator>>(uint32_t& data)
{
	_streamData.pop(&data, 4);
	return *this;
}

inline BinaryPack& BinaryPack::operator>>(uint64_t& data)
{
	_streamData.pop(&data, 8);
	return *this;
}

inline BinaryPack& BinaryPack::operator>>(std::string& data)
{
	uint16_t strLen;
	*this >> strLen;

	data.clear();
	uint8_t ch;
	for (size_t i = 0; i < strLen; ++i)
	{
		*this >> ch;
		data += ch;
	}

	return *this;
}

template<typename T>
inline BinaryPack& BinaryPack::operator>> (std::vector<T>& vec)
{
	vec.clear();
	uint16_t vecSize;
	*this >> vecSize;

	for (uint16_t i = 1; i <= vecSize; ++i)
	{
		T blockDesp;
		*this >> blockDesp;
		vec.push_back(blockDesp);
	}

	return *this;
}

#endif