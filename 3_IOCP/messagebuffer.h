
#pragma once

namespace messagebuffer
{
	constexpr int DEFAULT_BUFFER_SIZE = 20000;

	class cMessage
	{
	public:

		cMessage();
		cMessage(int bufferSize);
		virtual ~cMessage();

	public:
		int GetBufferSize() const { return _bufferSize; }
		int GetDataSize() const { return _dataSize; }
		int GetFreeSize() const { return (int)(_pEnd - _pWritePos); }
		char* GetWritePos() const { return _pWritePos; }
		char* GetReadPos() const { return _pReadPos; }

		bool Empty() const;
		void Clear();
		int MoveWritePos(int size);
		int MoveReadPos(int size);

		int GetData(const char* pSrc, int size);
		int PutData(char* pDest, int size);

	public:
		// src�� �����͸� ���ۿ� �־���

		cMessage& operator <<(unsigned char src);
		cMessage& operator <<(char src);

		cMessage& operator <<(const char* src);
		cMessage& operator <<(const wchar_t* src);

		cMessage& operator <<(unsigned short src);
		cMessage& operator <<(short src);

		cMessage& operator <<(unsigned int src);
		cMessage& operator <<(int src);

		cMessage& operator <<(unsigned long src);
		cMessage& operator <<(long src);

		cMessage& operator <<(unsigned long long src);
		cMessage& operator <<(long long src);

		cMessage& operator <<(float src);
		cMessage& operator <<(double src);
		cMessage& operator <<(long double src);


		// ������ �����͸� dest�� �־���

		cMessage& operator >>(unsigned char& dest);
		cMessage& operator >>(char& dest);

		cMessage& operator >>(char* dest);
		cMessage& operator >>(wchar_t* dest);

		cMessage& operator >>(unsigned short& dest);
		cMessage& operator >>(short& dest);

		cMessage& operator >>(unsigned int& dest);
		cMessage& operator >>(int& dest);

		cMessage& operator >>(unsigned long& dest);
		cMessage& operator >>(long& dest);

		cMessage& operator >>(unsigned long long& dest);
		cMessage& operator >>(long long& dest);

		cMessage& operator >>(float& dest);
		cMessage& operator >>(double& dest);
		cMessage& operator >>(long double& dest);


	protected:
		int _bufferSize;		// ��ü ���� ũ��
		int _dataSize;			// ������� ���� ũ��

		char* _pStart;			// ���� ���� ����
		char* _pEnd;			// ���� �� ����

		char* _pReadPos;
		char* _pWritePos;
	};
}