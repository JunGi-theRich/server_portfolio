
#pragma comment(lib, "winmm")
#pragma comment(lib, "Pdh")

#include <ctime>
#include <iostream>
#include <list>

//#include "Netlib_generalized.h"
#include "Netlib_optimized.h"

#include "txtParser.h"
txtParser::cTxtParser prs;

#include <Windows.h>
#include <Psapi.h>
#include <Pdh.h>

HANDLE g_hFile;

PROCESS_MEMORY_COUNTERS_EX pmc;
PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;

CRITICAL_SECTION cs_idList;
std::list<DWORD64> idList;		// 필요에 따라 콘텐츠는 id를 보관하여 사용함

constexpr short PAYLOAD_LEN = 8;
struct stHeader
{
	short _len = PAYLOAD_LEN;
};
class cServer : public cServer_TCP
{
	void OnError(int errCode, const wchar_t* str) 
	{
		wprintf(L"code: %d\t%s\n", errCode, str);
	}
	bool OnConnectionRequest(const SOCKADDR_IN& addr) { return true; }

	void OnEnter(DWORD64 id) 
	{
		EnterCriticalSection(&cs_idList);
		idList.push_back(id);
		LeaveCriticalSection(&cs_idList);

		// 1. 동적 할당
		//cMessage sendMsg(10);
		//sendMsg << PAYLOAD_LEN << 0x7fffffffffffffff;
		//TransmitMessage(id, &sendMsg);

		// 2. 라이브러리의 메모리풀 활용
		cMessage* pSendMsg = _pMessagePool->Alloc();
		pSendMsg->Clear();//!!반드시 clear 호출할 것...
		*pSendMsg << PAYLOAD_LEN << 0x7fffffffffffffff;
		TransmitMessage(id, pSendMsg);
		_pMessagePool->Free(pSendMsg);
	}
	void OnLeave(DWORD64 id)
	{
		EnterCriticalSection(&cs_idList);
		std::list<DWORD64>::iterator eraseIter = std::find(idList.begin(), idList.end(), id);
		if (eraseIter != idList.end())
		{
			idList.erase(eraseIter);
		}
		LeaveCriticalSection(&cs_idList);
	}
	void OnRecv(DWORD64 id, cMessage* pMsg, DWORD len)
	{
		while (1)
		{
			if (sizeof(stHeader) >= pMsg->GetDataSize() || 
				PAYLOAD_LEN != ((stHeader*)pMsg->GetReadPos())->_len ||
				(PAYLOAD_LEN > pMsg->GetDataSize() - sizeof(stHeader)))
			{
				break;
			}
			pMsg->MoveReadPos(sizeof(stHeader));
			__int64 recvData;
			*pMsg >> recvData;

			//cMessage* pSendMsg = new cMessage;
			//*pSendMsg << PAYLOAD_LEN;
			//*pSendMsg << recvData;
			//TransmitMessage(id, pSendMsg);
			//delete pSendMsg;

			cMessage* pSendMsg = _pMessagePool->Alloc();
			pSendMsg->Clear();//!!반드시 clear 호출할 것...
			*pSendMsg << PAYLOAD_LEN << recvData;
			TransmitMessage(id, pSendMsg);
			_pMessagePool->Free(pSendMsg);
		}
	}
	void OnSend(DWORD64 id, DWORD len) {}
};
cServer myServer;
cServer::stServerParam serverParam;

bool g_bRunMonitor = true;
HANDLE hMonitorThread;
unsigned WINAPI ServerMonitor(LPVOID lpParam);

int main()
{
	_wsetlocale(LC_ALL, L"KOR");	
	timeBeginPeriod(1);

	InitializeCriticalSection(&cs_idList);

	wchar_t ipAddress[20] = { 0, };
	short serverPort;
	int workerThread;
	int runningThread;
	bool noDelay;
	int maxClient;
	if (!prs.LoadFile(L"serversetting.txt"))
	{
		wprintf(L"설정 파일 불러오기 실패\n");
		wprintf(L"기본 설정(6000포트)으로 진행...\n");

		serverParam._ipAddr = L"0.0.0.0";
		serverParam._port = 6000;
		serverParam._bNoDelay = true;
		serverParam._maxClient = 5000;
	}
	else
	{
		prs.GetString(L"ipAddress", ipAddress, 20);
		prs.GetValue(L"serverPort", &serverPort);
		prs.GetValue(L"workerThread", &workerThread);
		prs.GetValue(L"runningThread", &runningThread);
		prs.GetValue(L"noDelay", &noDelay);
		prs.GetValue(L"maxClient", &maxClient);
		prs.CloseFile();

		serverParam._ipAddr = ipAddress;
		serverParam._port = serverPort;
		serverParam._numberOfWorkerThread = workerThread;
		serverParam._numberOfRunningThread = runningThread;
		serverParam._bNoDelay = noDelay;
		serverParam._maxClient = maxClient;
	}
	Sleep(500);
	
	hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, ServerMonitor, NULL, 0, NULL);
	if (NULL == hMonitorThread)
	{
		wprintf(L"Creating MonitorThread Failed\n");
		system("pause");
		return 1;
	}

	if (!myServer.Start(serverParam))
	{
		wprintf(L"Start Server Failed\n");
		system("pause");
		return 1;
	}

	while (1)
	{
		wchar_t input = _getwch();
		if (L'q' == input)
		{
			g_bRunMonitor = false;
			myServer.Stop();
			break;
		}
		else if (L'd' == input)
		{
			__int64 deleteNum;
			if (!idList.empty())
			{
				EnterCriticalSection(&cs_idList);
				deleteNum = *idList.begin();
				LeaveCriticalSection(&cs_idList);
				myServer.DisconnectSession(deleteNum);
			}
		}
	}
	int waitRet = WaitForSingleObject(hMonitorThread, INFINITE);
	if (WAIT_FAILED == waitRet)
	{
		wprintf(L"WaitForSingle Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}

	system("pause");
	return 0;
}


unsigned WINAPI ServerMonitor(LPVOID lpParam)
{
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, L"\\Processor(*)\\% Processor Time", NULL, &cpuTotal);
	PDH_FMT_COUNTERVALUE counterVal;

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T prev_CommitMemSize = pmc.PrivateUsage;
	while (g_bRunMonitor)
	{
		PdhCollectQueryData(cpuQuery);
		Sleep(999);

		wprintf(L"==============================\n");
		wprintf(L"q: quit server\td: disconnect first session test\n\n");

		wprintf(L"Total Session Accepted: %lld\n", myServer.GetTotalAccept());
		wprintf(L"Current Session Accepted: %d\n", myServer.GetCurAccept());
		wprintf(L"Session Accept TPS: %d\n", myServer.GetAcceptTPS());
		wprintf(L"Message Recv TPS: %d\n", myServer.GetRecvTPS());
		wprintf(L"Message Send TPS: %d\n", myServer.GetSendTPS());
		wprintf(L"Recv Bytes Per Sec: %d\n", myServer.GetRecvBytesPerSec());
		wprintf(L"Send Bytes Per Sec: %d\n\n", myServer.GetSendBytesPerSec());

		wprintf(L"Maximun Recv TPS: %d\n", myServer.GetMaxRecvTPS());
		wprintf(L"Maximun Send TPS: %d\n\n", myServer.GetMaxSendTPS());

		PdhCollectQueryData(cpuQuery);
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		wprintf(L"Current Cpu Usage: %f\n", counterVal.doubleValue);

		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		wprintf(L"Committed Memory Size by this Process: %lld", pmc.PrivateUsage);

		float commitDelta = (((float)pmc.PrivateUsage - (float)prev_CommitMemSize) / (float)(prev_CommitMemSize));
		wprintf(L"(%f)\n", commitDelta * 100);
		prev_CommitMemSize = pmc.PrivateUsage;

		wprintf(L"Current Non-paged Pool Usage: %lld\n", pmc.QuotaNonPagedPoolUsage);

		// 풀을 사용하는 서버 라이브러리에서 추가되는 모니터링 변수
		wprintf(L"Message Pool Usage Rate: %d / %d\n", myServer.GetUsedMsgPoolSize(), myServer.GetTotalMsgPoolSize());

		wprintf(L"\n\n");
	}

	return 0;
}