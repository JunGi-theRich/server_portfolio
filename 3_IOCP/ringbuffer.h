
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
		/* !! ���� üũ ���� size �� ��ŭ ������ �����̹Ƿ� ��� ���� !! */
		int MoveReadPos(const unsigned long size);
		/* !! ���� üũ ���� size �� ��ŭ ������ �����̹Ƿ� ��� ���� !! */
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

		/* �����͸� �̵����� �ʱ�ȭ�ϸ�, ���۴� �״�� �ξ� ����� �������� ���� */
		void Clear();

		UINT_PTR DirectEnqueueSize() const;
		UINT_PTR DirectDequeueSize() const;

		UINT_PTR LeftOverEnqueueSize() const;
		UINT_PTR LeftOverDequeuSize() const;

		bool Enqueue(const char* pData, const unsigned long size);
		bool Dequeue(char* pDest, const unsigned long size);
		bool Peek(char* pDest, const unsigned long size);

	private:
		unsigned long _bufferSize;		// �⺻���� ���� ��ü ũ��
		unsigned long _usedSize;		// ���� ������� ���� ũ��

		char* _pStart;			// ������ ���� �ּ�
		char* _pEnd;			// ������ �� �ּ�

		char* _pReadPos;		// ���� �б� ��ġ
		char* _pWritePos;		// ���� ���� ��ġ
	};
}