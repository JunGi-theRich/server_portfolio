
#include <stdlib.h>
#include <memory.h>

#include "ringbuffer.h"
using namespace ringbuffer;

cRingBuffer::cRingBuffer()
{
	_bufferSize = DEFAULT_BUFFER_SIZE;
	_usedSize = 0;

	_pStart = (char*)malloc(_bufferSize);
	_pEnd = _pStart + _bufferSize;

	_pReadPos = _pStart;
	_pWritePos = _pStart;
}
cRingBuffer::cRingBuffer(const unsigned long bufferSize)
{
	_bufferSize = bufferSize;
	_usedSize = 0;

	_pStart = (char*)malloc(_bufferSize);
	_pEnd = _pStart + _bufferSize;

	_pReadPos = _pStart;
	_pWritePos = _pStart;
}
cRingBuffer::~cRingBuffer()
{
	free(_pStart);
}

int cRingBuffer::MoveReadPos(const unsigned long size)
{
	// if(size > GetUsedSize()) return 0;

	if (_pEnd - _pReadPos < size)
	{
		__int64 extra = size - (_pEnd - _pReadPos);
		_pReadPos = _pStart + extra;
	}
	else
	{
		_pReadPos += size;
	}

	//_usedSize -= size;
	InterlockedExchangeSubtract(&_usedSize, size);
	return size;
}
int cRingBuffer::MoveWritePos(const unsigned long size)
{
	// if(size > GetFreeSize()) return 0;

	if (_pEnd - _pWritePos < size)
	{
		__int64 extra = size - (_pEnd - _pWritePos);
		_pWritePos = _pStart + extra;
	}
	else
	{
		_pWritePos += size;
	}

	//_usedSize += size;
	InterlockedExchangeAdd(&_usedSize, size);
	return size;
}

char* cRingBuffer::GetWriteBufferPtr() const
{
	if (_pWritePos == _pEnd)
	{
		return _pStart;
	}
	else
	{
		return _pWritePos;
	}
}
char* cRingBuffer::GetReadBufferPtr() const
{
	if (_pReadPos == _pEnd)
	{
		return _pStart;
	}
	else
	{
		return _pReadPos;
	}
}

bool cRingBuffer::Empty()
{
	// if(_usedSize == 0) 과 동일
	if (_pReadPos == _pWritePos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void cRingBuffer::Clear()
{
	_usedSize = 0;

	_pReadPos = _pStart;
	_pWritePos = _pStart;
}

UINT_PTR cRingBuffer::DirectEnqueueSize() const
{
	if (_pWritePos < _pReadPos)
	{
		return _pReadPos - _pWritePos;
	}
	else if (_pWritePos > _pReadPos)
	{
		return _pEnd - _pWritePos;
	}
	else
	{
		if (_pWritePos != _pEnd)
		{
			return _pEnd - _pWritePos;
		}
		else
		{
			return 0;
		}
	}
}
UINT_PTR cRingBuffer::DirectDequeueSize() const
{
	if (_pReadPos < _pWritePos)
	{
		return _pWritePos - _pReadPos;
	}
	else if (_pReadPos > _pWritePos)
	{
		return _pEnd - _pReadPos;
	}
	else
	{
		return 0;
	}
}

UINT_PTR cRingBuffer::LeftOverEnqueueSize() const
{
	if (_pWritePos < _pReadPos)
	{
		return 0;
	}
	else if (_pWritePos > _pReadPos)
	{
		return _pReadPos - _pStart;
	}
	else
	{
		if (_pWritePos != _pEnd)
		{
			return _pWritePos - _pStart;
		}
		else
		{
			return _pEnd - _pStart;
		}
	}
}
UINT_PTR cRingBuffer::LeftOverDequeuSize() const
{
	if (_pReadPos < _pWritePos)
	{
		return 0;
	}
	else if (_pReadPos > _pWritePos)
	{
		return _pWritePos - _pStart;
	}
	else
	{
		return 0;
	}
}

bool cRingBuffer::Enqueue(const char* pData, const unsigned long size)
{
	// 넣으려는 버퍼의 크기가 현재 큐의 남은 크기보다 큰 경우
	if (GetFreeSize() < size)
	{
		return false;
	}

	// 가장 끝 빈공간의 길이가 size보다 작은 경우
	if (_pEnd - _pWritePos < size)
	{
		__int64 extra = size - (_pEnd - _pWritePos);
		memcpy(_pWritePos, pData, _pEnd - _pWritePos);
		memcpy(_pStart, pData + (_pEnd - _pWritePos), extra);
		MoveWritePos(size);
	}
	else
	{
		memcpy(_pWritePos, pData, size);
		MoveWritePos(size);
	}

	return true;
}
bool cRingBuffer::Dequeue(char* pDest, const unsigned long size)
{
	// 빈 버퍼 또는 남은 크기보다 크게 빼려는 경우
	if (Empty() || _usedSize < size)
	{
		return false;
	}

	// 가장 끝 빈공간의 길이가 size보다 작은 경우
	if (_pEnd - _pReadPos < size)
	{
		__int64 extra = size - (_pEnd - _pReadPos);
		memcpy(pDest, _pReadPos, _pEnd - _pReadPos);
		memcpy(pDest + (_pEnd - _pReadPos), _pStart, extra);
		MoveReadPos(size);
	}
	else
	{
		memcpy(pDest, _pReadPos, size);
		MoveReadPos(size);
	}

	return true;
}
bool cRingBuffer::Peek(char* pDest, const unsigned long size)
{
	// 빈 버퍼 또는 남은 크기보다 크게 빼려는 경우
	//if (IsEmpty() || _usedSize < size)
	if (size > GetUsedSize() || size < 0 || Empty())
	{
		return false;
	}

	// 가장 끝 빈공간의 길이가 size보다 작은 경우
	if (_pEnd - _pReadPos < size)
	{
		__int64 extra = size - (_pEnd - _pReadPos);
		memcpy(pDest, _pReadPos, _pEnd - _pReadPos);
		memcpy(pDest + (_pEnd - _pReadPos), _pStart, extra);
	}
	else
	{
		memcpy(pDest, _pReadPos, size);
	}

	return true;
}