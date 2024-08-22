
//#define ACTIVATE

// 테스트 2
// 객체를 n회 할당 후에 랜덤으로 삭제하는 상황
// 실험 목적은 단일 스레드 서버에서 발생할 법한 환경을 구성해보는 것

#ifdef ACTIVATE



#pragma comment(lib, "winmm")

#include <locale.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>

#include "variousMempool.h"

using std::wstringstream;
using std::to_wstring;
using std::vector;
using std::swap;
using std::shuffle;
using std::random_device;
using std::mt19937;
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

PVOID g_addrContainer[ALLOC_COUNT];			// 주소 값을 저장하는 배열
vector<int> g_idxVec(ALLOC_COUNT);			// 인덱스를 저장하는 배열

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
	wprintf(L"테스트2 시작...\n");

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	float totalTime = 0;
	float finalAvg = 0;

	g_hFile = CreateFile(L"Test2_Result.txt", GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == g_hFile)
	{
		wprintf(L"CreateFile Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}

	// 0~ALLOC_COUNT 값들을 idxVec에 저장하고 순서를 섞어줌
	// 이후 idxVec에 들어있는 값으로 addrContainer 인덱스 접근하여 메모리 해제
	for (int i = 0; i < ALLOC_COUNT; ++i)
	{
		g_idxVec[i] = i;
	}
	random_device rd;
	mt19937 rng(rd());
	shuffle(g_idxVec.begin(), g_idxVec.end(), rng);

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
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest32* pNew = new stTest32;
				g_addrContainer[allocCnt] = pNew;
			}

			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
			{
				// 4000 개의 주소 값 중에서 무작위 2000 개의 주소값을 삭제하는데 걸린 시간 측정
				delete g_addrContainer[g_idxVec[allocCnt]];
			}
			QueryPerformanceCounter(&endTime);

			for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				// 지워지지 않은 나머지 주소 삭제
				delete g_addrContainer[g_idxVec[allocCnt]];
			}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_stack<stTest32> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = arrStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrStack.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					// 지워지지 않은 나머지 주소 삭제
					arrStack.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_queue<stTest32> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = arrQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrQueue.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrQueue.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}				

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_stack<stTest32> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = listStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listStack.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listStack.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}				

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_queue<stTest32> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest32* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest32* pNew = listQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listQueue.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listQueue.Free((stTest32*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest64* pNew = new stTest64;
				g_addrContainer[allocCnt] = pNew;
			}

			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}
			QueryPerformanceCounter(&endTime);

			for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_stack<stTest64> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = arrStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrStack.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrStack.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_queue<stTest64> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = arrQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrQueue.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrQueue.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_stack<stTest64> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = listStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listStack.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listStack.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_queue<stTest64> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest64* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest64* pNew = listQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listQueue.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listQueue.Free((stTest64*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest128* pNew = new stTest128;
				g_addrContainer[allocCnt] = pNew;
			}

			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}
			QueryPerformanceCounter(&endTime);

			for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_stack<stTest128> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = arrStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrStack.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrStack.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_queue<stTest128> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = arrQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrQueue.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrQueue.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_stack<stTest128> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = listStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listStack.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listStack.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_queue<stTest128> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest128* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest128* pNew = listQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listQueue.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listQueue.Free((stTest128*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest256* pNew = new stTest256;
				g_addrContainer[allocCnt] = pNew;
			}

			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}
			QueryPerformanceCounter(&endTime);

			for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_stack<stTest256> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = arrStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrStack.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrStack.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_queue<stTest256> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = arrQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrQueue.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrQueue.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_stack<stTest256> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = listStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listStack.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listStack.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_queue<stTest256> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest256* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest256* pNew = listQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listQueue.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listQueue.Free((stTest256*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				stTest512* pNew = new stTest512;
				g_addrContainer[allocCnt] = pNew;
			}

			QueryPerformanceCounter(&startTime);
			for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}
			QueryPerformanceCounter(&endTime);

			for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
			{
				delete g_addrContainer[g_idxVec[allocCnt]];
			}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_stack<stTest512> arrStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = arrStack.Alloc();
				arrStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = arrStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrStack.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrStack.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolArr_queue<stTest512> arrQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = arrQueue.Alloc();
				arrQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = arrQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					arrQueue.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					arrQueue.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_stack<stTest512> listStack(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = listStack.Alloc();
				listStack.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = listStack.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listStack.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listStack.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
		memset(g_addrContainer, 0, sizeof(g_addrContainer));
		{
			cPoolList_queue<stTest512> listQueue(ALLOC_COUNT);
			for (int loopCnt = 0; loopCnt < WARMUP_LOOP; ++loopCnt)
			{
				stTest512* pNew = listQueue.Alloc();
				listQueue.Free(pNew);
			}
			for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
			{
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					stTest512* pNew = listQueue.Alloc();
					g_addrContainer[allocCnt] = pNew;
				}

				QueryPerformanceCounter(&startTime);
				for (int allocCnt = 0; allocCnt < ALLOC_COUNT / 2; ++allocCnt)
				{
					listQueue.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}
				QueryPerformanceCounter(&endTime);

				for (int allocCnt = ALLOC_COUNT / 2; allocCnt < ALLOC_COUNT; ++allocCnt)
				{
					listQueue.Free((stTest512*)g_addrContainer[g_idxVec[allocCnt]]);
				}

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
