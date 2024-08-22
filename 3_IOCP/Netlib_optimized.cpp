
#include <iostream>
#include <new>

#include "Netlib_optimized.h"


cServer_TCP::cServer_TCP()
{
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);

	ZeroMemory(&_serverAddr, sizeof(SOCKADDR_IN));
	_listenSocket = INVALID_SOCKET;

	_uniqueId = 1;
	_bRun = TRUE;

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

	_pMessagePool = nullptr;
	_pValidIdxPool = nullptr;
}
cServer_TCP::~cServer_TCP()
{
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


	_pMessagePool = new cPoolList_stack<cMessage>(5000, false);

	int maxSession = infoParam._maxClient;
	_pValidIdxPool = new cPoolArr_stack<DWORD>(maxSession, false);
	_sessionArr.resize(maxSession);

	// 인덱스를 가져올 때 최초 할당을 0번 인덱스부터 주도록 설정
	DWORD** pTmpArr = new DWORD*[maxSession];
	for (int i = 0; i < maxSession; ++i)
	{
		DWORD* pTmpVal = _pValidIdxPool->Alloc();
		*pTmpVal = i;
		pTmpArr[i] = pTmpVal;
	}
	for (int i = 0; i < maxSession; ++i)
	{
		_pValidIdxPool->Free(pTmpArr[maxSession - 1 - i]);
	}
	delete[] pTmpArr;
	
	for (int i = 0; i < maxSession; ++i)
	{
		//stSession* pNewSession = new stSession;
		stSession* pNewSession = (stSession*)_aligned_malloc(sizeof(stSession), 64);
		if (nullptr == pNewSession)
		{
			abort();
		}
		new(pNewSession) stSession;		// alignd_malloc 사용했으므로 생성자 호출 유도
		
		pNewSession->_id._libId._arrIdx = i;
		_sessionArr[i] = pNewSession;
	}

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

	vector<stSession*>::iterator iter = _sessionArr.begin();
	for (iter; iter != _sessionArr.end(); ++iter)
	{
		closesocket((*iter)->_sock);
		stSession* pDel = (*iter);
		//delete pDel;
		_aligned_free(pDel);			//_aligned_mlloc을 사용했으므로 _aligend_free
	}
}

bool cServer_TCP::DisconnectSession(DWORD64 sessionId)
{
	stSession* pSession = FindSessionById(sessionId);
	if (nullptr == pSession)
	{
		return false;
	}

	if (0 == InterlockedDecrement(&pSession->_ioCount))
	{
		FreeSession(pSession);
		InterlockedDecrement(&_curAccept);
	}
	return true;
}
bool cServer_TCP::TransmitMessage(DWORD64 sessionId, cMessage* pMsg)
{
	stSession* pSession = FindSessionById(sessionId);
	if (nullptr == pSession)
	{
		return false;
	}

	AcquireSRWLockExclusive(&pSession->_srw_sessionLock);
	if (!pSession->_sendBuffer.Enqueue(pMsg->GetReadPos(), pMsg->GetDataSize()))
	{
		ReleaseSRWLockExclusive(&pSession->_srw_sessionLock);
		OnError(NetlibErrCode::BUFFER_ERROR, L"Enqueue OverFlow");
		DebugBreak();
		return false;
	}
	ReleaseSRWLockExclusive(&pSession->_srw_sessionLock);

	SendPost(pSession);
	return true;
}


cServer_TCP::stSession::stSession()
{
	_sessionStatus = SESSION_NOT_USED;
	_sock = INVALID_SOCKET;
	_id._contentId = 0;
	_pIdxFree = nullptr;
	memset(_ipAddr, 0, sizeof(_ipAddr));
	ZeroMemory(&_recvOverlapped, sizeof(OVERLAPPED));
	ZeroMemory(&_sendOverlapped, sizeof(OVERLAPPED));

	_ioCount = 0;
	_sendFlag = SEND_FLAG_OFF;
	InitializeSRWLock(&_srw_sessionLock);
}
cServer_TCP::stSession::~stSession()
{}


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
			if (0 == transferred)
			{
				if (0 == InterlockedDecrement(&pSessionKey->_ioCount))
				{
					pThis->FreeSession(pSessionKey);
					InterlockedDecrement(&pThis->_curAccept);
				}
				continue;
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
					pThis->FreeSession(pSessionKey);
					InterlockedDecrement(&pThis->_curAccept);
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

	stSession* pNewSession = AllocSession();
	if (nullptr == pNewSession)
	{
		OnError(NetlibErrCode::MAX_SESSION_OVERFLOW, L"tried to over-accept max session");
		return true;
	}
	pNewSession->_sock = clientSocket;
	inet_ntop(AF_INET, &clientAddr.sin_addr, pNewSession->_ipAddr, sizeof(pNewSession->_ipAddr));

	if (!OnConnectionRequest(clientAddr))
	{
		
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

	InterlockedIncrement(&pNewSession->_ioCount);

	++_acceptTPS;
	++_totalAccept;
	InterlockedIncrement(&_curAccept);

	OnEnter(pNewSession->_id._contentId);

	RecvPost(pNewSession);

	if (0 == InterlockedDecrement(&pNewSession->_ioCount))
	{
		FreeSession(pNewSession);
		InterlockedDecrement(&_curAccept);
		return true;
	}
	return true;
}

bool cServer_TCP::RecvProc(stSession* pSession, DWORD transferred)
{
	InterlockedIncrement(&_recvTPS);
	InterlockedAdd(&_recvBytesPerSec, transferred);

	pSession->_recvBuffer.MoveWritePos(transferred);

	cMessage* pRecvMsg = _pMessagePool->Alloc();
	pRecvMsg->Clear();
	if (!pSession->_recvBuffer.Peek(pRecvMsg->GetWritePos(), pSession->_recvBuffer.GetUsedSize()))
	{
		OnError(NetlibErrCode::BUFFER_ERROR, L"Peek Overflow");
		DebugBreak();
	}
	pRecvMsg->MoveWritePos(pSession->_recvBuffer.GetUsedSize());

	OnRecv(pSession->_id._contentId, pRecvMsg, pSession->_recvBuffer.GetUsedSize());
	pSession->_recvBuffer.MoveReadPos(pRecvMsg->GetBufferSize() - pRecvMsg->GetFreeSize());

	RecvPost(pSession);
	_pMessagePool->Free(pRecvMsg);
	return true;
}
bool cServer_TCP::SendProc(stSession* pSession, DWORD transferred)
{
	InterlockedIncrement(&_sendTPS);
	InterlockedAdd(&_sendBytesPerSec, transferred);

	AcquireSRWLockExclusive(&pSession->_srw_sessionLock);
	pSession->_sendBuffer.MoveReadPos(transferred);
	pSession->_sendFlag = SEND_FLAG_OFF;
	ReleaseSRWLockExclusive(&pSession->_srw_sessionLock);
	OnSend(pSession->_id._contentId, transferred);

	if (pSession->_sendBuffer.GetUsedSize() > 0)
	{
		if (!SendPost(pSession))
		{
			return false;
		}
	}

	return true;
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
				FreeSession(pSession);
				InterlockedDecrement(&_curAccept);
			}
			return false;
		}
	}
	return true;
}
bool cServer_TCP::SendPost(stSession* pSession)
{
	if (SEND_FLAG_OFF == InterlockedExchange(&pSession->_sendFlag, SEND_FLAG_ON))
	{
		ZeroMemory(&pSession->_sendOverlapped, sizeof(OVERLAPPED));
		WSABUF dataBuf[2] = { 0, };

		if (0 == pSession->_sendBuffer.GetUsedSize())
		{
			InterlockedExchange(&pSession->_sendFlag, SEND_FLAG_OFF);
			return true;
		}

		dataBuf[0].buf = pSession->_sendBuffer.GetReadBufferPtr();
		dataBuf[0].len = (ULONG)pSession->_sendBuffer.DirectDequeueSize();
		dataBuf[1].buf = pSession->_sendBuffer.GetHeadPos();
		dataBuf[1].len = (ULONG)pSession->_sendBuffer.LeftOverDequeuSize();

		InterlockedIncrement(&pSession->_ioCount);
		int retVal = WSASend(pSession->_sock, dataBuf, 2, NULL, NULL, &pSession->_sendOverlapped, NULL);
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
				FreeSession(pSession);
				InterlockedDecrement(&_curAccept);
			}
			return false;
		}
	}
	return true;
}

cServer_TCP::stSession* cServer_TCP::AllocSession()
{
	DWORD* pIdx = _pValidIdxPool->Alloc();
	stSession* pRetSession = _sessionArr[*pIdx];
	assert(pRetSession->_sessionStatus == SESSION_NOT_USED);
	assert(pRetSession->_ioCount == 0);

	pRetSession->_recvBuffer.Clear();
	pRetSession->_sendBuffer.Clear();

	pRetSession->_sessionStatus = SESSION_CUR_USED;
	pRetSession->_pIdxFree = pIdx;
	pRetSession->_id._libId._arrId = _uniqueId++;
	return pRetSession;
}
void cServer_TCP::FreeSession(stSession* pFree)
{
	assert(pFree->_sessionStatus == SESSION_CUR_USED);
	assert(pFree->_ioCount == 0);

	closesocket(pFree->_sock);
	pFree->_sessionStatus = SESSION_NOT_USED;
	pFree->_sendFlag = SEND_FLAG_OFF;
	DWORD* pIdx = pFree->_pIdxFree;
	DWORD64 contentId = pFree->_id._contentId;
	pFree->_pIdxFree = nullptr;
	_pValidIdxPool->Free(pIdx);
	OnLeave(contentId);
}

cServer_TCP::stSession* cServer_TCP::FindSessionById(DWORD64 contentId)
{
	cServer_TCP::stSession::uId* pUId = (cServer_TCP::stSession::uId*)&contentId;
	int idx = pUId->_libId._arrIdx;
	if (SESSION_CUR_USED == _sessionArr[idx]->_sessionStatus
		&& pUId->_libId._arrId == _sessionArr[idx]->_id._libId._arrId)
	{
		return _sessionArr[idx];
	}
	else
	{
		OnError(NetlibErrCode::ID_NOT_FOUND, L"ID couldn't found");
		return nullptr;
	}
}