
//#define ACTIVATE

// �׽�Ʈ 3-2
// ���� �����尡 n���� ��ü�� �����Ͽ� ������ �����ϰų�
// ��Ƽ �����尡 n���� ��ü�� �����Ͽ� �����͸� �����ϴ� ��Ȳ
// ���� ������ ��Ƽ ������ ȯ�濡�� �迭�� ����Ʈ ����� ���ǹ��� ���̸� ���������� Ȯ���ϴ� ��
// �׽�Ʈ 3-1���� �������� ĳ�� ���� �ȿ� �� ���� ��ü�� �� �� �ִ� ũ��

#ifdef ACTIVATE

long g_readyCount;

#pragma comment(lib, "winmm")

#include <locale.h>
#include <process.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>

#include "variousMempool.h"

using std::wstringstream;
using std::to_wstring;
using std::vector;
using std::swap;
using namespace variousMemoryPool;

HANDLE g_hFile;
LARGE_INTEGER freq;

constexpr int WARMUP_LOOP = 10;
constexpr int LOGIC_LOOP = 500000;
constexpr int ALLOC_COUNT = 1000;

constexpr int MINMAX_COUNT = 5;

struct stTest32
{
	char _size[32];
};

float g_singleThreadResult;
float g_multiThreadResult;

void IsBig(float time, float* pMaxArr);
void IsSmall(float time, float* pMinArr);

HANDLE hJobReadyEvent1;
HANDLE hJobReadyEvent2;
HANDLE hJobReadyEvent3;
HANDLE hJobReadyEvent4;
HANDLE hJobStartEvent;

unsigned WINAPI Test_SingleThread(LPVOID lpParam)
{
	if (!SetThreadAffinityMask(GetCurrentThread(), 2))
	{
		wprintf(L"SetThreadAffinityMask Failed due to %d\n", GetLastError());
		abort();
	}
	LPVOID* paramArr = (LPVOID*)lpParam;

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	g_singleThreadResult = 0;
	float elapsedTotal = 0;
	float minTime[MINMAX_COUNT] = { 0, };
	float maxTime[MINMAX_COUNT] = { 0, };
	vector<stTest32*> addrVec(ALLOC_COUNT * 4);

	if (0 == paramArr[0])
	{
		cPoolArr_stack<stTest32>* pAlloc = (cPoolArr_stack<stTest32>*)paramArr[1];
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest32* pNew = pAlloc->Alloc();
			pAlloc->Free(pNew);
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				stTest32* pNew = pAlloc->Alloc();
				addrVec[allocCnt] = pNew;
			}
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				addrVec[allocCnt]->_size[0] = (char)0xaa;
				addrVec[allocCnt]->_size[16] = (char)0xaa;
			}
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				pAlloc->Free(addrVec[allocCnt]);
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
			elapsedTotal += elapsedTime;
			IsBig(elapsedTime, maxTime);
			IsSmall(elapsedTime, minTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			elapsedTotal = elapsedTotal - minTime[i] - maxTime[i];
		}
		g_singleThreadResult = (elapsedTotal / (LOGIC_LOOP - MINMAX_COUNT * 2));
	}
	else
	{
		cPoolList_stack<stTest32>* pAlloc = (cPoolList_stack<stTest32>*)paramArr[1];
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest32* pNew = pAlloc->Alloc();
			pAlloc->Free(pNew);
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				stTest32* pNew = pAlloc->Alloc();
				addrVec[allocCnt] = pNew;
			}
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				addrVec[allocCnt]->_size[0] = (char)0xaa;
				addrVec[allocCnt]->_size[16] = (char)0xaa;
			}
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT * 4; ++allocCnt)
			{
				pAlloc->Free(addrVec[allocCnt]);
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
			elapsedTotal += elapsedTime;
			IsBig(elapsedTime, maxTime);
			IsSmall(elapsedTime, minTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			elapsedTotal = elapsedTotal - minTime[i] - maxTime[i];
		}
		g_singleThreadResult = (elapsedTotal / (LOGIC_LOOP - MINMAX_COUNT * 2));
	}
	return 0;
}
unsigned WINAPI Test_MultiThread(LPVOID lpParam)
{
	LPVOID* paramArr = (LPVOID*)lpParam;
	vector<stTest32*> addrVec(ALLOC_COUNT);

	if (0 == paramArr[0])
	{
		cPoolArr_stack<stTest32>* pAlloc = (cPoolArr_stack<stTest32>*)paramArr[2];
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest32* pNew = pAlloc->Alloc();
			pAlloc->Free(pNew);
		}

		if (0 == paramArr[1])
		{
			// �����ϴ� �����常 ����
			LARGE_INTEGER startTime;
			LARGE_INTEGER endTime;
			g_multiThreadResult = 0;
			float elapsedTotal = 0;
			float minTime[MINMAX_COUNT] = { 0, };
			float maxTime[MINMAX_COUNT] = { 0, };

			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				ResetEvent(hJobStartEvent);
				SetEvent(hJobReadyEvent1);
				DWORD waitRet = WaitForSingleObject(hJobStartEvent, INFINITE);
				if (WAIT_FAILED == waitRet)
				{
					wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
					abort();
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = pAlloc->Alloc();
					addrVec[allocCnt] = pNew;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					addrVec[allocCnt]->_size[0] = (char)0xaa;
					addrVec[allocCnt]->_size[16] = (char)0xaa;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					pAlloc->Free(addrVec[allocCnt]);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
				elapsedTotal += elapsedTime;
				IsBig(elapsedTime, maxTime);
				IsSmall(elapsedTime, minTime);

				InterlockedIncrement(&g_readyCount);
				while (g_readyCount)
				{
					InterlockedCompareExchange(&g_readyCount, 0, 4);
				}
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				elapsedTotal = elapsedTotal - minTime[i] - maxTime[i];
			}
			g_multiThreadResult = (elapsedTotal / (LOGIC_LOOP - MINMAX_COUNT * 2));
		}
		else
		{
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				ResetEvent(hJobStartEvent);
				if ((LPVOID)1 == paramArr[1])
				{
					SetEvent(hJobReadyEvent2);
				}
				else if ((LPVOID)2 == paramArr[1])
				{
					SetEvent(hJobReadyEvent3);
				}
				else
				{
					SetEvent(hJobReadyEvent4);
				}

				DWORD waitRet = WaitForSingleObject(hJobStartEvent, INFINITE);
				if (WAIT_FAILED == waitRet)
				{
					wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
					abort();
				}

				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = pAlloc->Alloc();
					addrVec[allocCnt] = pNew;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					addrVec[allocCnt]->_size[0] = (char)0xaa;
					addrVec[allocCnt]->_size[16] = (char)0xaa;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					pAlloc->Free(addrVec[allocCnt]);
				}

				InterlockedIncrement(&g_readyCount);
				while (g_readyCount)
				{
					InterlockedCompareExchange(&g_readyCount, 0, 4);
				}
			}
		}
	}
	else
	{
		cPoolList_stack<stTest32>* pAlloc = (cPoolList_stack<stTest32>*)paramArr[2];
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest32* pNew = pAlloc->Alloc();
			pAlloc->Free(pNew);
		}

		if (0 == paramArr[1])
		{
			// �����ϴ� �����常 ����
			LARGE_INTEGER startTime;
			LARGE_INTEGER endTime;
			g_multiThreadResult = 0;
			float elapsedTotal = 0;
			float minTime[MINMAX_COUNT] = { 0, };
			float maxTime[MINMAX_COUNT] = { 0, };

			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				ResetEvent(hJobStartEvent);
				SetEvent(hJobReadyEvent1);
				DWORD waitRet = WaitForSingleObject(hJobStartEvent, INFINITE);
				if (WAIT_FAILED == waitRet)
				{
					wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
					abort();
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = pAlloc->Alloc();
					addrVec[allocCnt] = pNew;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					addrVec[allocCnt]->_size[0] = (char)0xaa;
					addrVec[allocCnt]->_size[16] = (char)0xaa;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					pAlloc->Free(addrVec[allocCnt]);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
				elapsedTotal += elapsedTime;
				IsBig(elapsedTime, maxTime);
				IsSmall(elapsedTime, minTime);

				InterlockedIncrement(&g_readyCount);
				while (g_readyCount)
				{
					InterlockedCompareExchange(&g_readyCount, 0, 4);
				}
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				elapsedTotal = elapsedTotal - minTime[i] - maxTime[i];
			}
			g_multiThreadResult = (elapsedTotal / (LOGIC_LOOP - MINMAX_COUNT * 2));
		}
		else
		{
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				ResetEvent(hJobStartEvent);
				if ((LPVOID)1 == paramArr[1])
				{
					SetEvent(hJobReadyEvent2);
				}
				else if ((LPVOID)2 == paramArr[1])
				{
					SetEvent(hJobReadyEvent3);
				}
				else
				{
					SetEvent(hJobReadyEvent4);
				}

				DWORD waitRet = WaitForSingleObject(hJobStartEvent, INFINITE);
				if (WAIT_FAILED == waitRet)
				{
					wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
					abort();
				}

				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = pAlloc->Alloc();
					addrVec[allocCnt] = pNew;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					addrVec[allocCnt]->_size[0] = (char)0xaa;
					addrVec[allocCnt]->_size[16] = (char)0xaa;
				}
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					pAlloc->Free(addrVec[allocCnt]);
				}

				InterlockedIncrement(&g_readyCount);
				while (g_readyCount)
				{
					InterlockedCompareExchange(&g_readyCount, 0, 4);
				}
			}
		}
	}
	return 0;
}

int main()
{
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&freq);

	if (LOGIC_LOOP < MINMAX_COUNT)
	{
		wprintf(L"LOGIC_LOOP should bigger than MINMAX_COUNT!\n");
		system("pause");
		return 1;
	}
	wprintf(L"�׽�Ʈ3-2 ����...\n");

	g_hFile = CreateFile(L"Test3-2_Result.txt", GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == g_hFile)
	{
		wprintf(L"CreateFile Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}

	wstringstream wss;


	// ���� �����尡 ALLOC_COUNT ��ŭ ��ü �Ҵ� ���� �� ����
	//wprintf(L"������ �迭 �Ҵ� �۾� ���� - �̱� ������\n");
	//{
	//	cPoolArr_stack<stTest32> arrStack(ALLOC_COUNT * 4);
	//	LPVOID paramArr[2] = { (LPVOID)0, &arrStack };
	//	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Test_SingleThread, paramArr, 0, NULL);
	//	if (0 == hThread)
	//	{
	//		wprintf(L"_beginthreadex Failed due to %d\n", errno);
	//		abort();
	//	}
	//	DWORD waitRet = WaitForSingleObject(hThread, INFINITE);
	//	if (WAIT_FAILED == waitRet)
	//	{
	//		wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
	//		abort();
	//	}

	//	wss << L"\n*** [array stack - Single Thread 32 bytes] ***\n";
	//	WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
	//	wss << L"[��� �ҿ� �ð�: " << to_wstring(g_singleThreadResult) << L" us]\n";
	//	WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
	//	CloseHandle(hThread);
	//}
	//wprintf(L"������ ����Ʈ �Ҵ� �۾� ���� - �̱� ������\n");
	//{
	//	cPoolList_stack<stTest32> listStack(ALLOC_COUNT * 4);
	//	LPVOID paramArr[2] = { (LPVOID)1, &listStack };
	//	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Test_SingleThread, paramArr, 0, NULL);
	//	if (0 == hThread)
	//	{
	//		wprintf(L"_beginthreadex Failed due to %d\n", errno);
	//		abort();
	//	}
	//	DWORD waitRet = WaitForSingleObject(hThread, INFINITE);
	//	if (WAIT_FAILED == waitRet)
	//	{
	//		wprintf(L"WaitForSingle Failed due to %d\n", GetLastError());
	//		abort();
	//	}

	//	wss << L"\n*** [list stack - Single Thread 32 bytes] ***\n";
	//	WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
	//	wss << L"[��� �ҿ� �ð�: " << to_wstring(g_singleThreadResult) << L" us]\n";
	//	WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
	//	CloseHandle(hThread);
	//}


	// ��Ƽ �����尡 ALLOC_COUNT ��ŭ ��ü �Ҵ� ���� �� ����
	wprintf(L"������ �迭 32 bytes �Ҵ� �۾� ���� - ��Ƽ ������\n");
	{
		hJobReadyEvent1 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent3 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent4 = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (NULL == hJobReadyEvent1 || NULL == hJobReadyEvent2 || NULL == hJobReadyEvent3 || NULL == hJobReadyEvent4)
		{
			wprintf(L"CreateEvent Failed due to %d\n", GetLastError());
			abort();
		}
		hJobStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == hJobStartEvent)
		{
			wprintf(L"CreateEvent Failed due to %d\n", GetLastError());
			abort();
		}

		cPoolArr_stack<stTest32> arrStack(ALLOC_COUNT * 4);
		LPVOID paramArr1[3] = { (LPVOID)0, (LPVOID)0, &arrStack };		// �� ��° ���ڰ� 0�� ������� �ð� ������ ����
		LPVOID paramArr2[3] = { (LPVOID)0, (LPVOID)1, &arrStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		LPVOID paramArr3[3] = { (LPVOID)0, (LPVOID)2, &arrStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		LPVOID paramArr4[3] = { (LPVOID)0, (LPVOID)3, &arrStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		HANDLE hThreadArr[4];
		hThreadArr[0] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr1, 0, NULL);
		hThreadArr[1] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr2, 0, NULL);
		hThreadArr[2] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr3, 0, NULL);
		hThreadArr[3] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr4, 0, NULL);
		if (0 == hThreadArr[0] || 0 == hThreadArr[1] || 0 == hThreadArr[2] || 0 == hThreadArr[3])
		{
			abort();
		}

		DWORD waitRet;
		HANDLE hJobReadyArr[4] = { hJobReadyEvent1, hJobReadyEvent2, hJobReadyEvent3, hJobReadyEvent4 };
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			waitRet = WaitForMultipleObjects(4, hJobReadyArr, TRUE, INFINITE);
			if (WAIT_FAILED == waitRet)
			{
				wprintf(L"WaitForMultiple Failed due to %d\n", GetLastError());
				abort();
			}
			SetEvent(hJobStartEvent);
		}

		waitRet = WaitForMultipleObjects(4, hThreadArr, TRUE, INFINITE);
		if (WAIT_FAILED == waitRet)
		{
			wprintf(L"WaitForMultiple Failed due to %d\n", GetLastError());
			abort();
		}

		wss << L"\n*** [array stack - Multi Thread 32 bytes] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"[��� �ҿ� �ð�: " << to_wstring(g_multiThreadResult) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int i = 0; i < 4; ++i)
		{
			CloseHandle(hJobReadyArr[i]);
			CloseHandle(hThreadArr[i]);
		}
		CloseHandle(hJobStartEvent);
	}
	wprintf(L"������ ����Ʈ 32 bytes �Ҵ� �۾� ���� - ��Ƽ ������\n");
	{
		hJobReadyEvent1 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent3 = CreateEvent(NULL, FALSE, FALSE, NULL);
		hJobReadyEvent4 = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (NULL == hJobReadyEvent1 || NULL == hJobReadyEvent2 || NULL == hJobReadyEvent3 || NULL == hJobReadyEvent4)
		{
			wprintf(L"CreateEvent Failed due to %d\n", GetLastError());
			abort();
		}
		hJobStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == hJobStartEvent)
		{
			wprintf(L"CreateEvent Failed due to %d\n", GetLastError());
			abort();
		}

		cPoolList_stack<stTest32> listStack(ALLOC_COUNT * 4);
		LPVOID paramArr1[3] = { (LPVOID)1, (LPVOID)0, &listStack };		// �� ��° ���ڰ� 0�� ������� �ð� ������ ����
		LPVOID paramArr2[3] = { (LPVOID)1, (LPVOID)1, &listStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		LPVOID paramArr3[3] = { (LPVOID)1, (LPVOID)2, &listStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		LPVOID paramArr4[3] = { (LPVOID)1, (LPVOID)3, &listStack };		// �� ��° ���ڰ� 0�� �ƴ� ������� �Ҵ縸 ������ �� ����
		HANDLE hThreadArr[4];
		hThreadArr[0] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr1, 0, NULL);
		hThreadArr[1] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr2, 0, NULL);
		hThreadArr[2] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr3, 0, NULL);
		hThreadArr[3] = (HANDLE)_beginthreadex(NULL, 0, Test_MultiThread, paramArr4, 0, NULL);
		if (0 == hThreadArr[0] || 0 == hThreadArr[1] || 0 == hThreadArr[2] || 0 == hThreadArr[3])
		{
			abort();
		}

		DWORD waitRet;
		HANDLE hJobReadyArr[4] = { hJobReadyEvent1, hJobReadyEvent2, hJobReadyEvent3, hJobReadyEvent4 };
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			waitRet = WaitForMultipleObjects(4, hJobReadyArr, TRUE, INFINITE);
			if (WAIT_FAILED == waitRet)
			{
				wprintf(L"WaitForMultiple Failed due to %d\n", GetLastError());
				abort();
			}
			SetEvent(hJobStartEvent);
		}

		waitRet = WaitForMultipleObjects(4, hThreadArr, TRUE, INFINITE);
		if (WAIT_FAILED == waitRet)
		{
			wprintf(L"WaitForMultiple Failed due to %d\n", GetLastError());
			abort();
		}
		wss << L"\n*** [list stack - Multi Thread 32 bytes] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"[��� �ҿ� �ð�: " << to_wstring(g_multiThreadResult) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int i = 0; i < 4; ++i)
		{
			CloseHandle(hJobReadyArr[i]);
			CloseHandle(hThreadArr[i]);
		}
		CloseHandle(hJobStartEvent);
	}

	CloseHandle(g_hFile);
	return 0;
}


void IsBig(float time, float* pMaxArr)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == pMaxArr[i] || time > pMaxArr[i])
		{
			swap(pMaxArr[i], time);
		}
	}
}
void IsSmall(float time, float* pMinArr)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == pMinArr[i] || time < pMinArr[i])
		{
			swap(pMinArr[i], time);
		}
	}
}



#endif // ACTIVATE
