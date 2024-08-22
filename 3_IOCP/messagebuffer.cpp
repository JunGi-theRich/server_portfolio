
#include <stdlib.h>
#include <string>

#include "messagebuffer.h"
using namespace messagebuffer;

cMessage::cMessage()
{
	_bufferSize = DEFAULT_BUFFER_SIZE;
	_dataSize = 0;

	_pStart = (char*)malloc(_bufferSize);
	_pEnd = _pStart + _bufferSize;

	_pReadPos = _pStart;
	_pWritePos = _pStart;
}
cMessage::cMessage(int bufferSize)
{
	_bufferSize = bufferSize;
	_dataSize = 0;

	_pStart = (char*)malloc(_bufferSize);
	_pEnd = _pStart + _bufferSize;

	_pReadPos = _pStart;
	_pWritePos = _pStart;
}
cMessage::~cMessage()
{
	free(_pStart);
}

bool cMessage::Empty() const
{
	if (0 == _dataSize)
		return true;

	else
		return false;
}
void cMessage::Clear()
{
	// _pReadPos와 _pWritePos의 위치만 옮겨 재사용함
	_pReadPos = _pStart;
	_pWritePos = _pStart;

	_dataSize = 0;
}

int cMessage::MoveWritePos(const int size)
{
	if (0 > size || _pWritePos + size > _pEnd)
		return 0;

	_pWritePos += size;
	_dataSize += size;
	return size;
}
int cMessage::MoveReadPos(const int size)
{
	if (0 > size || _pReadPos + size > _pEnd)
		return 0;

	_pReadPos += size;
	_dataSize -= size;
	return size;
}

// 인자로 받은 버퍼의 데이터를 직렬화 버퍼에 넣어주는 함수
int cMessage::GetData(const char* pSrc, const int size)
{
	if (size > GetFreeSize())
		return 0;

	memcpy(_pWritePos, pSrc, size);
	_pWritePos += size;
	_dataSize += size;

	return size;
}

// 직렬화 버퍼에 있는 데이터를 꺼내어 목표하는 버퍼에 넣어줌
int cMessage::PutData(char* pDest, const int size)
{
	if (size > GetDataSize())
		return 0;

	memcpy(pDest, _pReadPos, size);
	_pReadPos += size;
	_dataSize -= size;

	return size;
}


cMessage& cMessage::operator <<(unsigned char src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(char src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}

cMessage& cMessage::operator <<(const char* src)
{
	GetData(src, (int)strlen(src));
	return *this;
}
cMessage& cMessage::operator <<(const wchar_t* src)
{
	GetData((char*)src, 2 * (int)wcslen(src));
	return *this;
}

cMessage& cMessage::operator <<(unsigned short src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(short src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}

cMessage& cMessage::operator <<(unsigned int src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(int src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}

cMessage& cMessage::operator <<(unsigned long src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(long src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}

cMessage& cMessage::operator <<(unsigned long long src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(long long src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}

cMessage& cMessage::operator <<(float src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(double src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}
cMessage& cMessage::operator <<(long double src)
{
	GetData((char*)&src, sizeof(src));
	return *this;
}


cMessage& cMessage::operator >>(unsigned char& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(char& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}

cMessage& cMessage::operator >>(char* dest)
{
	PutData(dest, (int)strlen(dest));
	return *this;
}
cMessage& cMessage::operator >>(wchar_t* dest)
{
	PutData((char*)dest, 2 * (int)wcslen(dest));
	return *this;
}

cMessage& cMessage::operator >>(unsigned short& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(short& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}

cMessage& cMessage::operator >>(unsigned int& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(int& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}

cMessage& cMessage::operator >>(unsigned long& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(long& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}

cMessage& cMessage::operator >>(unsigned long long& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(long long& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}

cMessage& cMessage::operator >>(float& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(double& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}
cMessage& cMessage::operator >>(long double& dest)
{
	PutData((char*)&dest, sizeof(dest));
	return *this;
}