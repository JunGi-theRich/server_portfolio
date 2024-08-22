
#pragma once

#include <Windows.h>

namespace ringbuffer
{
	constexpr int DEFAULT_BUFFER_SIZE = 64000;

	class cRingBuffer
	{
	public:
		cRingBuffer();
		cRingBuffer(const unsigned long bufferSize);
		~cRingBuffer();

	public:
		/* !! 조건 체크 없이 size 값 만큼 무조건 움직이므로 사용 주의 !! */
		int MoveReadPos(const unsigned long size);
		/* !! 조건 체크 없이 size 값 만큼 무조건 움직이므로 사용 주의 !! */
		int MoveWritePos(const unsigned long size);

		unsigned long GetBufferSize() const { return _bufferSize; }
		unsigned long GetUsedSize() const { return _usedSize; }
		unsigned long GetFreeSize() const { return _bufferSize - _usedSize; }

		char* GetHeadPos() const { return _pStart; }
		char* GetTailPos() const { return _pEnd; }

		char* GetReadPos() const { return _pStart; }
		char* GetWritePos() const { return _pEnd; }

		char* GetWriteBufferPtr() const;
		char* GetReadBufferPtr() const;

		bool Empty();

		/* 포인터만 이동시켜 초기화하며, 버퍼는 그대로 두어 디버깅 목적으로 남김 */
		void Clear();

		UINT_PTR DirectEnqueueSize() const;
		UINT_PTR DirectDequeueSize() const;

		UINT_PTR LeftOverEnqueueSize() const;
		UINT_PTR LeftOverDequeuSize() const;

		bool Enqueue(const char* pData, const unsigned long size);
		bool Dequeue(char* pDest, const unsigned long size);
		bool Peek(char* pDest, const unsigned long size);

	private:
		unsigned long _bufferSize;		// 기본적인 버퍼 전체 크기
		unsigned long _usedSize;		// 현재 사용중인 버퍼 크기

		char* _pStart;			// 버퍼의 시작 주소
		char* _pEnd;			// 버퍼의 끝 주소

		char* _pReadPos;		// 버퍼 읽기 위치
		char* _pWritePos;		// 버퍼 쓰기 위치
	};
}