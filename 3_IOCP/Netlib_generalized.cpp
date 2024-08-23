
#include <iostream>

#include "Netlib_generalized.h"


cServer_TCP::cServer_TCP()
{
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);

	ZeroMemory(&_serverAddr, sizeof(SOCKADDR_IN));
	_listenSocket = INVALID_SOCKET;

	_uniqueId = 0;
	_bRun = TRUE;

	InitializeCriticalSection(&_cs_mapLock);

	_numberOfWorkerThread = 0;

	_totalAccept = 0;
	_curAccept = 0;
	_acceptTPS = 0;
	_recvTPS = 0;
	_sendTPS = 0;
	_recvBytesPerSec = 0;
	_sendBytesPerSec = 0;
	_maxRecvTPS = 0;
	_maxSendTPS = 0;
}
cServer_TCP::~cServer_TCP()
{
	DeleteCriticalSection(&_cs_mapLock);
	closesocket(_listenSocket);
	WSACleanup();
}

cServer_TCP::stServerParam::stServerParam() 
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	_numberOfWorkerThread = sysInfo.dwNumberOfProcessors / 2;
	_numberOfRunningThread = sysInfo.dwNumberOfProcessors / 4;

	_bNoDelay = false;
}
cServer_TCP::stServerParam::stServerParam(short openPort, DWORD maxClient)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	_port = openPort;
	_numberOfWorkerThread = sysInfo.dwNumberOfProcessors / 2;
	_numberOfRunningThread = sysInfo.dwNumberOfProcessors / 4;

	// 프로세서 4개 이하 환경 대비 코드
	if (1 >= _numberOfWorkerThread)
	{
		_numberOfWorkerThread = 4;
	}
	if (1 >= _numberOfRunningThread)
	{
		_numberOfRunningThread = 2;
	}

	_bNoDelay = false;

	_maxClient = maxClient;
}
cServer_TCP::stServerParam::~stServerParam() {}

bool cServer_TCP::Start(const stServerParam& infoParam)
{
	if (0 >= infoParam._maxClient || 0 >= infoParam._numberOfRunningThread || 0 >= infoParam._numberOfWorkerThread)
	{
		OnError(NetlibErrCode::INVALID_SERVERPARAM, L"Invalid stServerParam");
		return false;
	}

	if (!InitializeListenSocket(infoParam._ipAddr, infoParam._port, infoParam._bNoDelay))
	{
		return false;
	}
	wprintf(L"Initializing Listen Socket Success\n");

	_hIOCP = CreateNewCompletionPort(infoParam._numberOfRunningThread);
	if (NULL == _hIOCP)
	{
		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"create IO Completion Port error");
		return false;
	}
	wprintf(L"Creating CompletionPort Success\n");

	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, (LPVOID)this, 0, NULL);
	if (NULL == _hAcceptThread)
	{
		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"_beginthreadex_accept error");
		return false;
	}
	_numberOfWorkerThread = infoParam._numberOfWorkerThread;
	_hIOCP_workerThread = new HANDLE[_numberOfWorkerThread];
	for (DWORD i = 0; i < _numberOfWorkerThread; ++i)
	{
		_hIOCP_workerThread[i] = (HANDLE)_beginthreadex(NULL, 0, IOCP_WorkerThread, (LPVOID)this, 0, NULL);
		if (NULL == _hIOCP_workerThread[i])
		{
			OnError(NetlibErrCode::DAMAGED_PARAMETER, L"_beginthreadex_worker error");
			return false;
		}
	}
	_hMonitorResetThread = (HANDLE)_beginthreadex(NULL, 0, MonitorResetThread, (LPVOID)this, 0, NULL);
	if (NULL == _hMonitorResetThread)
	{
		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"_beginthreadex_monitor error");
		return false;
	}

	return true;
}
void cServer_TCP::Stop()
{
	_bRun = FALSE;
	closesocket(_listenSocket);
	PostQueuedCompletionStatus(_hIOCP, NULL, NULL, NULL);

	DWORD waitRet;
	waitRet = WaitForSingleObject(_hAcceptThread, INFINITE);
	if (WAIT_FAILED == waitRet)
	{
		int waitForSingleErr = GetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"Accept Thread Termination Failed.");
	}
	else
	{
		wprintf(L"Accept Thread Terminated...\n");
	}

	waitRet = WaitForMultipleObjects(_numberOfWorkerThread, _hIOCP_workerThread, TRUE, INFINITE);
	if (WAIT_FAILED == waitRet)
	{
		int waitForMultiErr = GetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"Worker Thread Termination Failed.");
	}
	else
	{
		wprintf(L"Worker Thread Terminated...\n");
	}

	stSession* pDeleteSession;
	unordered_map<DWORD64, stSession*>::iterator mapIter = _sessionMap.begin();
	for (mapIter; mapIter != _sessionMap.end();)
	{
		pDeleteSession = mapIter->second;
		mapIter = _sessionMap.erase(mapIter);
		closesocket(pDeleteSession->_sock);
		delete pDeleteSession;
	}
}

bool cServer_TCP::DisconnectSession(DWORD64 sessionId)
{
	stSession* pSession;
	unordered_map<DWORD64, stSession*>::iterator mapIter;

	EnterCriticalSection(&_cs_mapLock);

	mapIter = _sessionMap.find(sessionId);
	if (_sessionMap.end() == mapIter)
	{
		OnError(NetlibErrCode::ID_NOT_FOUND, L"ID couldn't found");
		LeaveCriticalSection(&_cs_mapLock);
		return false;
	}
	pSession = mapIter->second;
	if (0 == InterlockedDecrement(&pSession->_ioCount))
	{
		PostQueuedCompletionStatus(_hIOCP, 0, (ULONG_PTR)pSession, (LPOVERLAPPED)DELETE_SESSION_PROC);
	}

	LeaveCriticalSection(&_cs_mapLock);

	return true;
}
bool cServer_TCP::TransmitMessage(DWORD64 sessionId, cMessage* pMsg)
{
	stSession* pSession;
	unordered_map<DWORD64, stSession*>::iterator mapIter;

	EnterCriticalSection(&_cs_mapLock);

	mapIter = _sessionMap.find(sessionId);
	if (_sessionMap.end() == mapIter)
	{
		OnError(NetlibErrCode::ID_NOT_FOUND, L"ID couldn't find");
		LeaveCriticalSection(&_cs_mapLock);
		return false;
	}
	pSession = mapIter->second;
	EnterCriticalSection(&pSession->_cs_sessionLock);

	if (!pSession->_sendBuffer.Enqueue(pMsg->GetReadPos(), pMsg->GetDataSize()))
	{
		LeaveCriticalSection(&pSession->_cs_sessionLock);
		OnError(NetlibErrCode::BUFFER_ERROR, L"Enqueue OverFlow");
		DebugBreak();
		return false;
	}

	SendPost(pSession);

	LeaveCriticalSection(&pSession->_cs_sessionLock);

	LeaveCriticalSection(&_cs_mapLock);
	return true;
}

cServer_TCP::stSession::stSession()
{
#ifndef NDEBUG
	_sock = INVALID_SOCKET;
	_id = -1;
	memset(_ipAddr, 0, sizeof(_ipAddr));
	ZeroMemory(&_recvOverlapped, sizeof(OVERLAPPED));
	ZeroMemory(&_sendOverlapped, sizeof(OVERLAPPED));
#endif // !NDEBUG

	_ioCount = 0;
	_sendFlag = SEND_FLAG_OFF;
	InitializeCriticalSection(&_cs_sessionLock);
}
cServer_TCP::stSession::~stSession()
{
	DeleteCriticalSection(&_cs_sessionLock);
}

unsigned WINAPI cServer_TCP::AcceptThread(LPVOID lpParam)
{
	cServer_TCP* pThis = (cServer_TCP*)lpParam;
	while (pThis->TryAcceptSession())
	{}

	return 0;
}
unsigned WINAPI cServer_TCP::IOCP_WorkerThread(LPVOID lpParam)
{
	cServer_TCP* pThis = (cServer_TCP*)lpParam;

	while (pThis->_bRun)
	{
		int retVal;
		DWORD transferred = NULL;
		stSession* pSessionKey = nullptr;
		OVERLAPPED* pOverlappedInfo = nullptr;
		retVal = GetQueuedCompletionStatus(pThis->_hIOCP, &transferred, (PULONG_PTR)&pSessionKey, &pOverlappedInfo, INFINITE);
		if (nullptr == pOverlappedInfo)
		{
			if (TRUE == retVal)
			{
				pThis->_bRun = FALSE;
				closesocket(pThis->_listenSocket);
				PostQueuedCompletionStatus(pThis->_hIOCP, NULL, NULL, NULL);
				return 0;
			}
			else
			{
				pThis->OnError(NetlibErrCode::DAMAGED_PARAMETER, L"GQCS(IOCP) error");
				pThis->_bRun = FALSE;
				closesocket(pThis->_listenSocket);
				PostQueuedCompletionStatus(pThis->_hIOCP, NULL, NULL, NULL);
				return 1;
			}
		}
		else
		{
			if ((LPOVERLAPPED)DELETE_SESSION_PROC == pOverlappedInfo)
			{
				// PQCS로 받은 세션 삭제 명령
				pThis->RemoveSession(pSessionKey);
				continue;
			}
			else if (0 == transferred)
			{
				if (0 == InterlockedDecrement(&pSessionKey->_ioCount))
				{
					pThis->RemoveSession(pSessionKey);
					continue;
				}
			}
			else
			{
				if (pOverlappedInfo == &pSessionKey->_recvOverlapped)
				{
					pThis->RecvProc(pSessionKey, transferred);
				}
				else if (pOverlappedInfo == &pSessionKey->_sendOverlapped)
				{
					pThis->SendProc(pSessionKey, transferred);
				}

				if (0 == InterlockedDecrement(&pSessionKey->_ioCount))
				{
					pThis->RemoveSession(pSessionKey);
				}
			}
		}
	}
	return 0;
}
unsigned WINAPI cServer_TCP::MonitorResetThread(LPVOID lpParam)
{
	cServer_TCP* pThis = (cServer_TCP*)lpParam;
	while (pThis->_bRun)
	{
		Sleep(999);

		if (pThis->_maxRecvTPS < pThis->_recvTPS)
		{
			pThis->_maxRecvTPS = pThis->_recvTPS;
		}
		if (pThis->_maxSendTPS < pThis->_sendTPS)
		{
			pThis->_maxSendTPS = pThis->_sendTPS;
		}

		if (0 == pThis->_recvTPS && 0 == pThis->_sendTPS)
		{
			continue;
		}

		pThis->_acceptTPS = 0;
		pThis->_recvTPS = 0;
		pThis->_sendTPS = 0;
		pThis->_recvBytesPerSec = 0;
		pThis->_sendBytesPerSec = 0;
	}

	return 0;
}

bool cServer_TCP::InitializeListenSocket(const wstring& ip, short port, bool bNoDelay)
{
	int retVal;
	retVal = WSAStartup(MAKEWORD(2, 2), &_wsa);
	if (0 != retVal)
	{
		int WSAStartupErr = retVal; DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"WSAStartup error");
		return false;
	}

	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _listenSocket)
	{
		int socketErr = WSAGetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"socket(listenSoccket) error");
		return false;
	}

	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	retVal = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(LINGER));
	if (SOCKET_ERROR == retVal)
	{
		int setsockoptErr = WSAGetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"setsockopt(linger) error");
		return false;
	}

	// NODELAY == NAGLE OFF
	if (bNoDelay)
	{
		BOOL noDealy = 1;
		retVal = setsockopt(_listenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&noDealy, sizeof(BOOL));
		if (SOCKET_ERROR == retVal)
		{
			int setsockoptErr = WSAGetLastError(); DebugBreak();

			OnError(NetlibErrCode::DAMAGED_PARAMETER, L"setsockopt(nagle) error");
			return false;
		}
	}

	_serverAddr.sin_family = AF_INET;
	InetPtonW(AF_INET, ip.c_str(), &_serverAddr.sin_addr);
	_serverAddr.sin_port = htons(port);
	retVal = bind(_listenSocket, (SOCKADDR*)&_serverAddr, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		int bindErr = WSAGetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"bind error");
		return false;
	}

	retVal = listen(_listenSocket, SOMAXCONN);
	if (SOCKET_ERROR == retVal)
	{
		int listenErr = WSAGetLastError(); DebugBreak();

		OnError(NetlibErrCode::DAMAGED_PARAMETER, L"listen error");
		return false;
	}

	return true;
}
HANDLE cServer_TCP::CreateNewCompletionPort(DWORD numberOfConcurrentThreads)
{
	return (CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, numberOfConcurrentThreads));
}
bool cServer_TCP::AssociateSocketWithCompletionPort(HANDLE completionPort, HANDLE sock, ULONG_PTR completionKey)
{
	HANDLE hRet = CreateIoCompletionPort(sock, completionPort, completionKey, 0);
	return (completionPort == hRet);
}

bool cServer_TCP::TryAcceptSession()
{
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);

	clientSocket = accept(_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
	if (INVALID_SOCKET == clientSocket)
	{
		int errCode = WSAGetLastError();
		if (WSAENOTSOCK != errCode && WSAEINTR != errCode)
		{
			int accpetErr = WSAGetLastError(); DebugBreak();

			OnError(NetlibErrCode::DAMAGED_SOCKET, L"accept error");
		}		
		return false;
	}

	stSession* pNewSession = new stSession;
	pNewSession->_sock = clientSocket;
	inet_ntop(AF_INET, &clientAddr.sin_addr, pNewSession->_ipAddr, sizeof(pNewSession->_ipAddr));
	pNewSession->_id = ++_uniqueId;

	if (!OnConnectionRequest(clientAddr))
	{
		closesocket(clientSocket);
		delete pNewSession;
		return true;
	}

	if (!AssociateSocketWithCompletionPort(_hIOCP, (HANDLE)pNewSession->_sock, (ULONG_PTR)pNewSession))
	{
		int iocpErr = GetLastError(); DebugBreak();
		OnError(NetlibErrCode::DAMAGED_SOCKET, L"Associate Socket with IO Completion Port error");
		closesocket(clientSocket);
		delete pNewSession;
		return false;
	}

	EnterCriticalSection(&_cs_mapLock);
	_sessionMap.insert(std::pair<DWORD64, stSession*>(pNewSession->_id, pNewSession));
	LeaveCriticalSection(&_cs_mapLock);	

	InterlockedIncrement(&pNewSession->_ioCount);

	++_acceptTPS;
	++_totalAccept;
	InterlockedIncrement(&_curAccept);

	OnEnter(pNewSession->_id);

	RecvPost(pNewSession);

	if (0 == InterlockedDecrement(&pNewSession->_ioCount))
	{
		PostQueuedCompletionStatus(_hIOCP, 0xdddddddd, (ULONG_PTR)pNewSession, (LPOVERLAPPED)DELETE_SESSION_PROC);
		//RemoveSession(pNewSession);
	}

	// 인터락 증감이 필요한 이유
	// OnEnter 내부에 TransmitMessage가 존재하고 그 안에는 SendPost 존재함
	// 세션의 기본 ioCount는 최초 RecvPost 적용 전까지 0인 상태
	// 이 때 위의 OnEnter->TransmitMessage->SendPost가 실패하면 ioCount는 0이 되고
	// 최초 RecvPost 수행 전에 세션 자체가 지워질 수 있음
	// 따라서 의도적으로 1회의 ioCount 증감을 진행함
	// ++) 세션 생성자에서 최초 ioCount값을 1로 지정하여도 됨

	return true;
}
void cServer_TCP::RemoveSession(stSession* pSession)
{
	EnterCriticalSection(&_cs_mapLock);

	EnterCriticalSection(&pSession->_cs_sessionLock);
	LeaveCriticalSection(&pSession->_cs_sessionLock);

	DWORD64 leaveId = pSession->_id;
	size_t ret = _sessionMap.erase(leaveId);
	if (0 == ret)
	{
		LeaveCriticalSection(&_cs_mapLock);
		return;
	}
	closesocket(pSession->_sock);
	delete pSession;

	LeaveCriticalSection(&_cs_mapLock);

	InterlockedDecrement(&_curAccept);

	OnLeave(leaveId);
}

void cServer_TCP::RecvProc(stSession* pSession, DWORD transferred)
{
	InterlockedIncrement(&_recvTPS);
	InterlockedAdd(&_recvBytesPerSec, transferred);

	pSession->_recvBuffer.MoveWritePos(transferred);

	cMessage recvMsg(pSession->_recvBuffer.GetUsedSize());
	if (!pSession->_recvBuffer.Peek(recvMsg.GetWritePos(), pSession->_recvBuffer.GetUsedSize()))
	{
		OnError(NetlibErrCode::BUFFER_ERROR, L"Peek Overflow");
		abort();
	}
	recvMsg.MoveWritePos(pSession->_recvBuffer.GetUsedSize());

	OnRecv(pSession->_id, &recvMsg, pSession->_recvBuffer.GetUsedSize());
	pSession->_recvBuffer.MoveReadPos(recvMsg.GetBufferSize() - recvMsg.GetFreeSize());
	
	RecvPost(pSession);
}
void cServer_TCP::SendProc(stSession* pSession, DWORD transferred)
{
	InterlockedIncrement(&_sendTPS);
	InterlockedAdd(&_sendBytesPerSec, transferred);

	EnterCriticalSection(&pSession->_cs_sessionLock);

	pSession->_sendBuffer.MoveReadPos(transferred);
	OnSend(pSession->_id, transferred);
	pSession->_sendFlag = SEND_FLAG_OFF;

	if (pSession->_sendBuffer.GetUsedSize() > 0)
	{
		SendPost(pSession);
	}
	LeaveCriticalSection(&pSession->_cs_sessionLock);
}

bool cServer_TCP::RecvPost(stSession* pSession)
{
	ZeroMemory(&pSession->_recvOverlapped, sizeof(OVERLAPPED));

	DWORD retVal;
	DWORD flags = 0;
	WSABUF dataBuf[2];
	dataBuf[0].buf = pSession->_recvBuffer.GetWriteBufferPtr();
	dataBuf[0].len = (ULONG)pSession->_recvBuffer.DirectEnqueueSize();
	dataBuf[1].buf = pSession->_recvBuffer.GetHeadPos();
	dataBuf[1].len = (ULONG)pSession->_recvBuffer.LeftOverEnqueueSize();

	DWORD bytes;
	InterlockedIncrement(&pSession->_ioCount);
	retVal = WSARecv(pSession->_sock, dataBuf, 2, &bytes, &flags, &pSession->_recvOverlapped, NULL);
	if (SOCKET_ERROR == retVal)
	{
		int errCode = WSAGetLastError();
		if (ERROR_IO_PENDING == errCode)
		{
			return true;
		}
		else
		{
			if (WSAECONNABORTED != errCode && WSAECONNRESET != errCode)
			{
				int recvErr = WSAGetLastError(); DebugBreak();
				OnError(NetlibErrCode::DAMAGED_SOCKET, L"WSARecv error");
			}

			if (0 == InterlockedDecrement(&pSession->_ioCount))
			{
				//RemoveSession(pSession);
				PostQueuedCompletionStatus(_hIOCP, 0, (ULONG_PTR)pSession, (LPOVERLAPPED)DELETE_SESSION_PROC);
			}
			return false;
		}
	}
	return true;
}
bool cServer_TCP::SendPost(stSession* pSession)
{
	if (SEND_FLAG_OFF == pSession->_sendFlag)
	{
		pSession->_sendFlag = SEND_FLAG_ON;
		if (0 == pSession->_sendBuffer.GetUsedSize())
		{
			pSession->_sendFlag = SEND_FLAG_OFF;
			return true;
		}

		ZeroMemory(&pSession->_sendOverlapped, sizeof(OVERLAPPED));

		DWORD retVal;
		WSABUF dataBuf[2] = { 0, };

		dataBuf[0].buf = pSession->_sendBuffer.GetReadBufferPtr();
		dataBuf[0].len = (ULONG)pSession->_sendBuffer.DirectDequeueSize();
		dataBuf[1].buf = pSession->_sendBuffer.GetHeadPos();
		dataBuf[1].len = (ULONG)pSession->_sendBuffer.LeftOverDequeuSize();

		InterlockedIncrement(&pSession->_ioCount);
		retVal = WSASend(pSession->_sock, dataBuf, 2, NULL, NULL, &pSession->_sendOverlapped, NULL);
		if (SOCKET_ERROR == retVal)
		{
			int errCode = WSAGetLastError();
			if (ERROR_IO_PENDING == errCode)
			{
				return true;
			}

			if (WSAECONNABORTED != errCode && WSAECONNRESET != errCode)
			{
				int sendErr = WSAGetLastError(); DebugBreak();
				OnError(NetlibErrCode::DAMAGED_SOCKET, L"WSASend error");
			}
			if (0 == InterlockedDecrement(&pSession->_ioCount))
			{
				PostQueuedCompletionStatus(_hIOCP, 0, (ULONG_PTR)pSession, (LPOVERLAPPED)DELETE_SESSION_PROC);
			}
			return false;
		}
	}
	return true;
}


/*
* 동기화 객체에서 세션 삭제 방법
* 
* 1. 
*	세션맵락 
*	[
*		세션검색
*		세션맵.erase
*		세션락
*		{
*	] 
*	세션맵언락
*		}
*		세션언락
*	delete 세션
* 
* 2.
*	세션맵락
*	[
*		세션검색
*		세션락
*		{
*		}
*		세션언락
*		세션맵.erase
*		delete 세션
*	]
*	세션맵언락
*/