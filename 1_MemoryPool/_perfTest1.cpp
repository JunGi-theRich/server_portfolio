
//#define ACTIVATE

// 테스트 1
// 객체 1회 할당 이후 바로 삭제하는 상황
// 실험 목적은 캐시 히트 및 syscall의 영향을 확인하는 것

#ifdef ACTIVATE



#pragma comment(lib, "winmm")

#include <locale.h>
#include <iostream>
#include <sstream>

#include "variousMempool.h"

using std::wstringstream;
using std::to_wstring;
using std::swap;
using namespace variousMemoryPool;


HANDLE g_hFile;
LARGE_INTEGER freq;

constexpr int WARMUP_LOOP = 10;
constexpr int LOGIC_LOOP = 500000;
constexpr int ALLOC_COUNT = 4000;

constexpr int MINMAX_COUNT = 5;

struct stTest32
{
	char _size[32];
};
struct stTest64
{
	char _size[64];
};
struct stTest128
{
	char _size[128];
};
struct stTest256
{
	char _size[256];
};
struct stTest512
{
	char _size[512];
};

float minTime[MINMAX_COUNT];
float maxTime[MINMAX_COUNT];
void IsBig(float time);
void IsSmall(float time);

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
	if (!SetProcessAffinityMask(GetCurrentProcess(), 2))
	{
		wprintf(L"SetProcessAffinityMask Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}
	wprintf(L"테스트1 시작...\n");
	
	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	float totalTime = 0;
	float finalAvg = 0;

	g_hFile = CreateFile(L"Test1_Result.txt", GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == g_hFile)
	{
		wprintf(L"CreateFile Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}

	wstringstream wss;

	wprintf(L"32 바이트 구조체 할당 시작\n");
	{
		wss << L"\n\n\n====================\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"## 32 바이트 구조체 할당 테스트 ##\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");

		wss << L"\n*** [new delete] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest32* pNew = new stTest32;
			delete pNew;
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest32* pNew = new stTest32;
				delete pNew;
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

			totalTime += elapsedTime;
			IsSmall(elapsedTime);
			IsBig(elapsedTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			totalTime = totalTime - minTime[i] - maxTime[i];
		}
		finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
		wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [array stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_stack<stTest32> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = arrStack.Alloc();
					arrStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [array queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_queue<stTest32> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = arrQueue.Alloc();
					arrQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_stack<stTest32> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = listStack.Alloc();
					listStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_queue<stTest32> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = listQueue.Alloc();
					listQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}
	}

	wprintf(L"64 바이트 구조체 할당 시작\n");
	{
		wss << L"\n\n\n====================\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"## 64 바이트 구조체 할당 테스트 ##\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");

		wss << L"\n*** [new delete] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest64* pNew = new stTest64;
			delete pNew;
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest64* pNew = new stTest64;
				delete pNew;
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

			totalTime += elapsedTime;
			IsSmall(elapsedTime);
			IsBig(elapsedTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			totalTime = totalTime - minTime[i] - maxTime[i];
		}
		finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
		wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [array stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_stack<stTest64> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = arrStack.Alloc();
					arrStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [array queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_queue<stTest64> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = arrQueue.Alloc();
					arrQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_stack<stTest64> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = listStack.Alloc();
					listStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_queue<stTest64> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = listQueue.Alloc();
					listQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}
	}



	wprintf(L"128 바이트 구조체 할당 시작\n");
	{
		wss << L"\n\n\n====================\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"## 128 바이트 구조체 할당 테스트 ##\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [new delete] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest128* pNew = new stTest128;
			delete pNew;
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest128* pNew = new stTest128;
				delete pNew;
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

			totalTime += elapsedTime;
			IsSmall(elapsedTime);
			IsBig(elapsedTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			totalTime = totalTime - minTime[i] - maxTime[i];
		}
		finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
		wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [array stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_stack<stTest128> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = arrStack.Alloc();
					arrStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [array queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_queue<stTest128> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = arrQueue.Alloc();
					arrQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_stack<stTest128> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = listStack.Alloc();
					listStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_queue<stTest128> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = listQueue.Alloc();
					listQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}
	}



	wprintf(L"256 바이트 구조체 할당 시작\n");
	{
		wss << L"\n\n\n====================\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"## 256 바이트 구조체 할당 테스트 ##\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [new delete] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest256* pNew = new stTest256;
			delete pNew;
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest256* pNew = new stTest256;
				delete pNew;
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

			totalTime += elapsedTime;
			IsSmall(elapsedTime);
			IsBig(elapsedTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			totalTime = totalTime - minTime[i] - maxTime[i];
		}
		finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
		wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [array stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_stack<stTest256> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = arrStack.Alloc();
					arrStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [array queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_queue<stTest256> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = arrQueue.Alloc();
					arrQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_stack<stTest256> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = listStack.Alloc();
					listStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_queue<stTest256> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = listQueue.Alloc();
					listQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}
	}



	wprintf(L"512 바이트 구조체 할당 시작\n");
	{
		wss << L"\n\n\n====================\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		wss << L"## 512 바이트 구조체 할당 테스트 ##\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [new delete] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
		{
			stTest512* pNew = new stTest512;
			delete pNew;
		}
		for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
		{
			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest512* pNew = new stTest512;
				delete pNew;
			}
			QueryPerformanceCounter(&endTime);

			float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

			totalTime += elapsedTime;
			IsSmall(elapsedTime);
			IsBig(elapsedTime);
		}
		for (int i = 0; i < MINMAX_COUNT; ++i)
		{
			totalTime = totalTime - minTime[i] - maxTime[i];
		}
		finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
		wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");


		wss << L"\n*** [array stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_stack<stTest512> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = arrStack.Alloc();
					arrStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [array queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolArr_queue<stTest512> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = arrQueue.Alloc();
					arrQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list stack] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_stack<stTest512> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = listStack.Alloc();
					listStack.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}


		wss << L"\n*** [list queue] ***\n";
		WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		totalTime = 0;
		finalAvg = 0;
		memset(minTime, 0, sizeof(minTime));
		memset(minTime, 0, sizeof(maxTime));
		{
			cPoolList_queue<stTest512> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = listQueue.Alloc();
					listQueue.Free(pNew);
				}
				QueryPerformanceCounter(&endTime);

				float elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;

				totalTime += elapsedTime;
				IsSmall(elapsedTime);
				IsBig(elapsedTime);
			}
			for (int i = 0; i < MINMAX_COUNT; ++i)
			{
				totalTime = totalTime - minTime[i] - maxTime[i];
			}
			finalAvg = totalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
			wss << L"[평균 소요 시간: " << to_wstring(finalAvg) << L" us]\n";
			WriteFile(g_hFile, wss.str().c_str(), (DWORD)wss.str().size() * 2, NULL, NULL); wss.str(L"");
		}
	}


	WriteFile(g_hFile, L"\n\n\n", 6, NULL, NULL);
	CloseHandle(g_hFile);

	wprintf(L"측정 완료\n");
	system("pause");
	return 0;
}


void IsBig(float time)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == maxTime[i] || time > maxTime[i])
		{
			swap(maxTime[i], time);
		}
	}
}
void IsSmall(float time)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == minTime[i] || time < minTime[i])
		{
			swap(minTime[i], time);
		}
	}
}



#endif // ACTIVATE