
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <Windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "ringbuffer.h"
#include "messagebuffer.h"

using namespace std;
using namespace messagebuffer;

constexpr short PAYLOAD_LEN = 8;
constexpr int serverPort = 6000;

struct stHeader
{
	short _len = PAYLOAD_LEN;
};

struct stDummyClient
{
	stDummyClient()
	{
		_bErase = false;
		_bConnect = false;
		_bLoginPacket = false;
		_sock = INVALID_SOCKET;
		_sendData = 0;
		_sendCount = 0;
		_connectTime = 0;
		_lastSendTime = 0;
	}
	
	bool _bErase;				// 오류 등의 이유로 켜진 클라이언트를 루프 마지막에 삭제하기 위한 변수
	bool _bConnect;				// 클라이언트 연결 성공 여부
	bool _bLoginPacket;			// 로그인 패킷 수신 여부
	SOCKET _sock;				// 클라이언트 소켓
	DWORD64 _sendData;			// 송신하는 에코 데이터
	DWORD _sendCount;			// desiredCount와 비교해야함
	DWORD _connectTime;			// login 패킷 수신여부 위한 변수
	DWORD _lastSendTime;		// delay를 위한 변수

	ringbuffer::cRingBuffer _recvQ;
	messagebuffer::cMessage _sendBuf;


	// list / map 등으로 송신한 데이터를 보관해두고 수신시 송신한 데이터가 도착했는지 확인하는 로직 가능
	// 그러나 많은 데이터를 송신하는 경우, 자료 구조 삽입 및 탐색에 걸리는 시간 고려해야 함
};


wstring serverIP = L"127.0.0.1";
SOCKADDR_IN serverAddr;
bool bSend = true;
bool bRun = true;

bool bDisconnect = false;
unsigned g_desiredClient;
unsigned g_desiredSend;
unsigned long g_sendDelay;

long g_totalConnet;
long g_curConnect;
long g_sendTPS;
long g_recvTPS;

long g_loginPacketNotRecv;
long g_duplicatedLogicPacket;

unsigned long g_maxLatency;
long g_connectFail;
long g_echoNotRecv;
long g_invalidPacket;
long g_disconnet;

bool RecvProc(stDummyClient* pClient);
bool SendProc(stDummyClient* pClient);

char loginPacket[8];

//==============================

HANDLE hTimerEvent;
HANDLE hMonitorThread;
HANDLE hResetEvent;

unsigned WINAPI MonitorThread(LPVOID lpParam)
{
	DWORD checkTime = timeGetTime();
	while (bRun)
	{
		Sleep(999);

		wprintf(L"s: Stop Echo / Resume Echo\n");
		wprintf(L"q: Quit\n\n");

		wprintf(L"Desired Client: %d\t", g_desiredClient);
		wprintf(L"Desired Send: %d\n", g_desiredSend);
		wprintf(L"Send Delay: %d\n\n", g_sendDelay);

		wprintf(L"Total Connect: %d\n", g_totalConnet);
		wprintf(L"Current Connect: %d\n", g_curConnect);
		wprintf(L"Message Send TPS: %d\n", g_sendTPS);
		wprintf(L"Message Recv TPS: %d\n\n", g_recvTPS);

		wprintf(L"Login Packet Not Recv: %d\n", g_loginPacketNotRecv);
		wprintf(L"Duplicated Login Packet: %d\n\n", g_duplicatedLogicPacket);

		wprintf(L"Max Latency(ms): %d\n", g_maxLatency);
		wprintf(L"Error - Connect Failed: %d\n", g_connectFail);
		wprintf(L"Error - Echo Not Recv(1sec.): %d\n", g_echoNotRecv);
		wprintf(L"Error - Invalid Packet Recv: %d\n", g_invalidPacket);
		wprintf(L"Error - Disconnected From Server: %d\n", g_disconnet);
		wprintf(L"\n--------------------------------------------------\n\n");

		if (0 == g_sendTPS && 0 == g_recvTPS)
		{
			continue;
		}

		g_sendTPS = 0;
		g_recvTPS = 0;
	}
	return 0;
}

HANDLE hWorkerThread[5];
unsigned WINAPI WorkerThread(LPVOID lpParam)
{
	// 더미 클라이언트가 한 개인 경우 2, 3, 4, 5번 스레드는 동작하지 않음
	if (1 == g_desiredClient && ((LPVOID)2 == lpParam || (LPVOID)3 == lpParam || (LPVOID)4 == lpParam || (LPVOID)5 == lpParam))
	{
		return 0;
	}

	// 더미 클라이언트가 두 개인 경우 3, 4, 5번 스레드는 동작하지 않음
	else if (2 == g_desiredClient && ((LPVOID)3 == lpParam || (LPVOID)4 == lpParam || (LPVOID)5 == lpParam))
	{
		return 0;
	}


	int retVal;
	int desiredClinetbyThread;				// 각 스레드들이 관리해야할 클라이언트 수
	if (1 == g_desiredClient || 2 == g_desiredClient)
	{
		desiredClinetbyThread = 1;
	}
	else
	{
		desiredClinetbyThread = g_desiredClient / 5;
	}
	int curClientbyThread = 0;
	list<stDummyClient*> clientList;			// 이 리스트를 순회하며 접속한 세션들에 대해서 select

	//DWORD connectTime = 0;
	DWORD disconnectTime = timeGetTime();
	while (bRun)
	{
		SOCKET sock;

		//if (desiredClinetbyThread > curClientbyThread && 100 < timeGetTime() - connectTime)		// 0.1 초마다 세션 등록
		if(desiredClinetbyThread > curClientbyThread)
		{
			//connectTime = timeGetTime();
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (INVALID_SOCKET == sock)
			{
				wprintf(L"socket error: %d\n", WSAGetLastError());
				DebugBreak();
				return 1;
			}

			u_long on = 1;
			retVal = ioctlsocket(sock, FIONBIO, &on);
			if (SOCKET_ERROR == retVal)
			{
				wprintf(L"ioctlsocket error: %d\n", WSAGetLastError());
				DebugBreak();
				return 1;
			}
			
			retVal = connect(sock, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
			if (SOCKET_ERROR == retVal)
			{
				if (WSAEWOULDBLOCK != WSAGetLastError())
				{
					InterlockedIncrement(&g_connectFail);
					wprintf(L"connect error: %d\n", WSAGetLastError());
					closesocket(sock);
					continue;
				}
			}

			// 클라이언트 리스트에 우선 등록 후에 select 과정에서 결과 확인
			stDummyClient* pNewClient = new stDummyClient;
			pNewClient->_sock = sock;
			clientList.push_back(pNewClient);
			++curClientbyThread;
		}

		FD_SET readSet;
		FD_SET writeSet;
		FD_SET exceptSet;

		stDummyClient* pClientArr[FD_SETSIZE];
		list<stDummyClient*>::iterator iter = clientList.begin();
		for (iter; iter != clientList.end();)
		{
			memset(pClientArr, 0, sizeof(pClientArr));
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);
			FD_ZERO(&exceptSet);

			for (int setCnt = 0; setCnt < FD_SETSIZE; ++setCnt)
			{
				if (iter == clientList.end())
				{
					break;
				}

				FD_SET((*iter)->_sock, &readSet);
				FD_SET((*iter)->_sock, &writeSet);
				if (false == (*iter)->_bConnect)
				{
					FD_SET((*iter)->_sock, &exceptSet);
				}
				pClientArr[setCnt] = *iter;
				++iter;
			}

			timeval timeOut = { 0, 0 };
			int selectResult = select(0, &readSet, &writeSet, &exceptSet, &timeOut);
			for (int selectCnt = 0; selectCnt < FD_SETSIZE; ++selectCnt)
			{
				stDummyClient* pClient = pClientArr[selectCnt];
				if (nullptr == pClient)
				{
					break;
				}

				if (0 < selectResult)
				{
					if (FD_ISSET(pClient->_sock, &readSet))
					{
						if (!RecvProc(pClient))
						{
							InterlockedIncrement(&g_disconnet);
							InterlockedDecrement(&g_curConnect);
							--curClientbyThread;

							pClient->_bErase = true;
							closesocket(pClient->_sock);
							continue;
						}
					}
					if (FD_ISSET(pClient->_sock, &writeSet))
					{
						if (false == pClient->_bConnect)
						{
							InterlockedIncrement(&g_totalConnet);
							InterlockedIncrement(&g_curConnect);
							pClient->_connectTime = timeGetTime();
							pClient->_bConnect = true;
						}

						// 송신 중단 체크
						if (bSend)
						{
							// 송신 딜레이 체크
							if (g_sendDelay <= timeGetTime() - pClient->_lastSendTime)
							{
								if (!SendProc(pClient))
								{
									InterlockedIncrement(&g_disconnet);
									InterlockedDecrement(&g_curConnect);
									--curClientbyThread;

									pClient->_bErase = true;
									closesocket(pClient->_sock);
									continue;
								}
							}
						}
					}
					if (FD_ISSET(pClient->_sock, &exceptSet))
					{
						// 예외셋 ... connect 실패
						InterlockedIncrement(&g_connectFail);
						--curClientbyThread;

						pClient->_bErase = true;
						closesocket(pClient->_sock);
						continue;
					}


					if (true == pClient->_bConnect && false == pClient->_bLoginPacket && 3000 < timeGetTime() - pClient->_connectTime)
					{
						InterlockedIncrement(&g_loginPacketNotRecv);
						pClient->_connectTime = timeGetTime();
					}
					if (true == pClient->_bConnect && 0 != pClient->_sendCount && 1000 < timeGetTime() - pClient->_lastSendTime)
					{
						InterlockedIncrement(&g_echoNotRecv);
						pClient->_lastSendTime = timeGetTime();
					}
				}
				else if (SOCKET_ERROR == selectResult)
				{
					// select 함수의 경우 fd_set 멤버가 잘못되었음
					wprintf(L"select error: %d\n", WSAGetLastError());
					system("pause");
					DebugBreak();
				}
			}
		}

		// 입출력 과정에서 오류가 발생한 스레드는 이 루프에서 한 번에 제거함
		list<stDummyClient*>::iterator eraseIter = clientList.begin();
		for (eraseIter; eraseIter != clientList.end();)
		{
			if ((*eraseIter)->_bErase)
			{
				stDummyClient* pDeleteClient = *eraseIter;
				eraseIter = clientList.erase(eraseIter);
				delete pDeleteClient;
			}
			else
			{
				++eraseIter;
			}

			// bDisconnect 플래그가 켜졌을 때 0.5초마다 리스트 가장 앞 클라이언트 제거
			if (!clientList.empty() && bDisconnect && 500 < timeGetTime() - disconnectTime)
			{
				InterlockedDecrement(&g_curConnect);
				--curClientbyThread;
				disconnectTime = timeGetTime();

				stDummyClient* pDeleteClient = *clientList.begin();
				clientList.pop_front();
				closesocket(pDeleteClient->_sock);
				delete pDeleteClient;
			}
		}
	}
	return 0;
}

//==============================


int main()
{

	//-----------------------------
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);

	DWORD64 loginNumber = 0x7fffffffffffffff;
	memcpy(loginPacket, &loginNumber, 8);

	wcout << L"Server IP Address: ";
	getline(wcin, serverIP);

	DWORD tmpFlag;
	wcout << L"Disconnet Flag - 0.5sec. (1 = YES / 2 = NO): ";
	wcin >> tmpFlag;
	if (wcin.fail()) return 1;
	if (1 == tmpFlag) bDisconnect = true;
	else bDisconnect = false;

	wcout << L"Max Client (1 - 1 / 2 - 2 / 3 - 50 / 4 - 100): "; 
	wcin >> g_desiredClient;
	if (wcin.fail() || 0 >= g_desiredClient || 4 < g_desiredClient) return 1;
	if (1 == g_desiredClient) g_desiredClient = 1;
	else if (2 == g_desiredClient) g_desiredClient = 2;
	else if (3 == g_desiredClient) g_desiredClient = 50;
	else g_desiredClient = 100;

	wcout << L"Send Packet Count (1 - 1 / 2 - 10 / 3 - 100 / 4 - 200): ";
	wcin >> g_desiredSend;
	if (wcin.fail() || 0 == g_desiredSend || 4 < g_desiredSend) return 1;
	if (1 == g_desiredSend) g_desiredSend = 1;
	else if (2 == g_desiredSend) g_desiredSend = 10;
	else if (3 == g_desiredSend) g_desiredSend = 100;
	else g_desiredSend = 200;

	wcout << L"Send Dealy: ";
	wcin >> g_sendDelay;
	if (wcin.fail()) return 1;

	//-----------------------------

	int retVal;
	WSADATA wsa;

	retVal = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (0 != retVal)
	{
		wprintf(L"WSAStartup error: %d\n", retVal);
		return 1;
	}

	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	InetPtonW(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
	serverAddr.sin_port = htons(serverPort);

	hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, NULL, 0, NULL);
	hWorkerThread[0] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (PVOID)1, 0, NULL);
	hWorkerThread[1] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (PVOID)2, 0, NULL);
	hWorkerThread[2] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (PVOID)3, 0, NULL);
	hWorkerThread[3] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (PVOID)4, 0, NULL);
	hWorkerThread[4] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (PVOID)5, 0, NULL);

	while (1)
	{
		Sleep(1000);
		if (GetAsyncKeyState(0x53))
		{
			if (false == bSend)
			{
				bSend = true;
			}
			else
			{
				bSend = false;
			}
		}
		if (GetAsyncKeyState(0x51))
		{
			bRun = false;
			break;
		}
	}

	if (WAIT_FAILED == WaitForSingleObject(hMonitorThread, INFINITE))
	{
		wprintf(L"WaitForSingleObject error: %d\n", GetLastError());
		system("pause");
		return 1;
	}
	wprintf(L"MonitorThread Terminated!\n");

	if (WAIT_FAILED == WaitForMultipleObjects(5, hWorkerThread, TRUE, INFINITE))
	{
		wprintf(L"WaitForMultipleObjects error: %d\n", GetLastError());
		system("pause");
		return 1;
	}
	wprintf(L"All WorkerThread Terminated!\n");

	WSACleanup();
	system("pause");
	return 0;
}


bool RecvProc(stDummyClient* pClient)
{
	int retVal;
	char* pTmpBuf = new char[5000];
	retVal = recv(pClient->_sock, pTmpBuf, 5000, 0);
	if (0 == retVal)
	{
		delete[] pTmpBuf;
		return false;
	}
	else if (SOCKET_ERROR == retVal)
	{
		if (WSAEWOULDBLOCK != GetLastError())
		{
			delete[] pTmpBuf;
			return false;
		}
	}

	pClient->_recvQ.Enqueue(pTmpBuf, retVal);
	while (pClient->_recvQ.GetUsedSize())
	{
		char header[sizeof(stHeader)];
		char payload[PAYLOAD_LEN] = { 0, };

		if (false == pClient->_recvQ.Peek(header, sizeof(stHeader)) ||
			PAYLOAD_LEN > pClient->_recvQ.GetUsedSize() - sizeof(stHeader))
		{
			// 지금 당장 처리하지 못하는 데이터
			delete[] pTmpBuf;
			return true;
		}

		InterlockedIncrement(&g_recvTPS);
		--pClient->_sendCount;							
		pClient->_recvQ.MoveReadPos(sizeof(stHeader));
		pClient->_recvQ.Dequeue(payload, PAYLOAD_LEN);
		DWORD64 data = (DWORD64)atoll(payload);
		if (0 == memcmp(loginPacket, payload, 8))
		{
			if (pClient->_bLoginPacket)
			{
				InterlockedIncrement(&g_duplicatedLogicPacket);
			}
			pClient->_bLoginPacket = true;
			++pClient->_sendCount;			// 로그인 패킷은 sendCount 값으로 계산하지 않음
		}
		else if (PAYLOAD_LEN != ((stHeader*)header)->_len || (0 > data || 0x1000 < data))
		{
			// 잘못된 데이터
			wprintf(L"Invlaid Packet Recv...(%lld)", data);
			InterlockedIncrement(&g_invalidPacket);
			delete[] pTmpBuf;
			return false;
		}
		else
		{
			// 정확한 에코를 수신했기 때문에 레이턴시를 구함

			// 더 정확한 레이턴시 값을 구해야하는 경우
			// 1. 동기화 객체를 사용 (CriticalSection / SRWLOCK)
			// 2. while문과 InterlockedCompareExchange를 사용
			//unsigned long curLatency = timeGetTime() - pClient->_lastSendTime;
			//while (1) 
			//{
			//	unsigned long tmpLatency = InterlockedCompareExchange(&g_maxLatency, 0, 0);
			//	if (tmpLatency >= curLatency) 
			//		break;
			//	InterlockedCompareExchange(&g_maxLatency, curLatency, tmpLatency);
			//}

			DWORD recvTime = timeGetTime();
			if (g_maxLatency < recvTime - pClient->_lastSendTime)
			{
				g_maxLatency = recvTime - pClient->_lastSendTime;
			}
		}
	}
	delete[] pTmpBuf;
	return true;
}

bool SendProc(stDummyClient* pClient)
{
	// 최대 송신 메시지 수 체크
	if (g_desiredSend > pClient->_sendCount)
	{
		pClient->_sendBuf.Clear();
		while (g_desiredSend > pClient->_sendCount)
		{
			InterlockedIncrement(&g_sendTPS);
			++pClient->_sendCount;
			pClient->_sendBuf << PAYLOAD_LEN;
			pClient->_sendBuf << pClient->_sendData;

			// 송신되는 데이터에는 최소 범위와 최대 범위(0 ~ 0x1000) 제한함
			if (0x1000 < ++pClient->_sendData)
			{
				pClient->_sendData = 0;
			}
		}

		int retVal = send(pClient->_sock, pClient->_sendBuf.GetReadPos(), pClient->_sendBuf.GetDataSize(), 0);
		if (SOCKET_ERROR == retVal)
		{
			if (WSAEWOULDBLOCK != GetLastError())
			{
				return false;
			}
		}

		// 최종 송신 후에 송신 타이머값 갱신
		pClient->_lastSendTime = timeGetTime();
	}
	return true;
}