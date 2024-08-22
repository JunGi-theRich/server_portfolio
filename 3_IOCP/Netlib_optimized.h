#pragma once

#pragma comment(lib, "winmm")
#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>

#include <unordered_map>
#include <vector>
#include <string>

#include "messagebuffer.h"
#include "ringbuffer.h"
#include "memoryPool.h"

using namespace messagebuffer;
using namespace ringbuffer;
using namespace MemoryPool;

using std::vector;
using std::wstring;

class cServer_TCP
{
public:
	cServer_TCP();
	~cServer_TCP();

	struct stServerParam
	{
		stServerParam();
		stServerParam(short openPort, DWORD maxClient);
		~stServerParam();

		wstring _ipAddr;
		short _port;
		int _numberOfWorkerThread;
		int _numberOfRunningThread;
		bool _bNoDelay;
		int _maxClient;
	};

public:
	bool Start(const stServerParam& infoParam);
	void Stop();

	bool DisconnectSession(DWORD64 sessionId);
	bool TransmitMessage(DWORD64 sessionId, cMessage* pMsg);

	long long GetTotalAccept() const { return _totalAccept; }
	long GetCurAccept() const { return _curAccept; }
	long GetAcceptTPS() const { return _acceptTPS; }
	long GetRecvTPS() const { return _recvTPS; }
	long GetSendTPS() const { return _sendTPS; }
	long GetRecvBytesPerSec() const { return _recvBytesPerSec; }
	long GetSendBytesPerSec() const { return _sendBytesPerSec; }
	long GetMaxRecvTPS() const { return _maxRecvTPS; }
	long GetMaxSendTPS() const { return _maxSendTPS; }

	long GetTotalMsgPoolSize() { return _pMessagePool->GetTotalSize(); }
	long GetUsedMsgPoolSize() { return _pMessagePool->GetUsedSize(); }

public:
	virtual void OnError(int errCode, const wchar_t* str) = 0;					// ���ο��� ���� �߻����� �� ó���� �۾�
	virtual bool OnConnectionRequest(const SOCKADDR_IN& addr) = 0;				// ����̾�Ʈ ���� ��� ���� ����, true �� ��� �޾Ƶ���
	virtual void OnEnter(DWORD64 id) = 0;										// ����̾�Ʈ ���� �� ������ �۾�
	virtual void OnLeave(DWORD64 id) = 0;										// ����̾�Ʈ ���� ���� �� ������ �۾�
	virtual void OnRecv(DWORD64 id, cMessage* pMsg, DWORD len) = 0;				// Ŭ���̾�Ʈ�� �޽��� ���� ���� ó���� �۾�, pMsg�� ������ �������
	virtual void OnSend(DWORD64 id, DWORD len) = 0;								// Ŭ���̾�Ʈ�� �޽��� �۽� ���� ó���� �۾�


private:
	struct stSession
	{
		union uId
		{
			struct stLibId
			{
				DWORD _arrIdx;
				DWORD _arrId;
			};

			DWORD64 _contentId;
			stLibId _libId;
		};

		DWORD _sessionStatus;
		SOCKET _sock;
		uId _id;
		DWORD* _pIdxFree;
		CHAR _ipAddr[30];

		OVERLAPPED _recvOverlapped;
		OVERLAPPED _sendOverlapped;

		alignas(64) cRingBuffer _recvBuffer;
		alignas(64) cRingBuffer _sendBuffer;

		alignas(64) DWORD _ioCount;
		alignas(64) DWORD _sendFlag;

		SRWLOCK _srw_sessionLock;

		stSession();
		~stSession();
	};

private:

	static unsigned WINAPI AcceptThread(LPVOID lpParam);
	static unsigned WINAPI IOCP_WorkerThread(LPVOID lpParam);
	static unsigned WINAPI MonitorResetThread(LPVOID lpParam);

private:

	bool InitializeListenSocket(const wstring& ip, short port, bool bNoDelay);
	HANDLE CreateNewCompletionPort(DWORD numberOfConcurrentThreads);
	bool AssociateSocketWithCompletionPort(HANDLE completionPort, HANDLE sock, ULONG_PTR completionKey);

	bool TryAcceptSession();

	bool RecvProc(stSession* pSession, DWORD transferred);
	bool SendProc(stSession* pSession, DWORD transferred);

	bool RecvPost(stSession* pSession);
	bool SendPost(stSession* pSession);

	stSession* AllocSession();
	void FreeSession(stSession* pFree);

	stSession* FindSessionById(DWORD64 contentId);

private:
	HANDLE _hIOCP;
	WSADATA _wsa;
	SOCKADDR_IN _serverAddr;
	SOCKET _listenSocket;

	DWORD _uniqueId;
	BOOL _bRun;
	
	cPoolArr_stack<DWORD>* _pValidIdxPool;
	vector<stSession*> _sessionArr;

	HANDLE _hAcceptThread;
	HANDLE* _hIOCP_workerThread;
	HANDLE _hMonitorResetThread;

	DWORD _numberOfWorkerThread;

public:
	cPoolList_stack<cMessage>* _pMessagePool;

private:
	long long _totalAccept;
	long _curAccept;

	long _acceptTPS;
	long _recvTPS;
	long _sendTPS;

	long _recvBytesPerSec;
	long _sendBytesPerSec;

	long _maxRecvTPS;
	long _maxSendTPS;

private:
	static constexpr long SEND_FLAG_ON = 1;
	static constexpr long SEND_FLAG_OFF = 0;

	static constexpr long SESSION_CUR_USED = 1;
	static constexpr long SESSION_NOT_USED = 0;
};

enum NetlibErrCode
{
	SUCCESS = 0,

	DAMAGED_PARAMETER,				// ���̺귯�� ���� �Լ� ȣ���� ���� �Ű� ������ ��� ������ �ջ�Ǿ� �Լ� ȣ�� ����
	DAMAGED_SOCKET,					// �� ��Ȳ�� ������, ���� �Լ� ȣ���� ���� ������ ��� ������ �ջ��

	INVALID_SERVERPARAM,			// Start�Լ� ȣ���� ���� stServerParam�� ��� ���� ����
	BUFFER_ERROR,					// ������ ���� ���ġ�� �Ѿ�� ��ť �Ǵ� ��ũ
	ID_NOT_FOUND,					// �������� ã���� �ϴ� id�� ����

	MAX_SESSION_OVERFLOW,
};
