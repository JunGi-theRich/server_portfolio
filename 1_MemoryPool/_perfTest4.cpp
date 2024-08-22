
#define ACTIVATE

#ifdef ACTIVATE


#pragma comment(lib, "winmm")

#include <stdio.h>
#include <locale.h>
#include <Windows.h>

LARGE_INTEGER freq;
LARGE_INTEGER startTime;
LARGE_INTEGER endTime;

constexpr int LOGIC_LOOP = 10000;

struct stTest32
{
	char _size[32];
};
struct stTest128
{
	char _size[128];
};

int main()
{
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&freq);
	float elapsedTime;

	int* pInt;
	stTest32* pTest1;
	stTest128* pTest2;

	QueryPerformanceCounter(&startTime);
	for (int i = 0; i < LOGIC_LOOP; ++i)
	{
		pInt = new int;
	}
	QueryPerformanceCounter(&endTime);
	elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
	wprintf(L"int: %f\n", elapsedTime);

	QueryPerformanceCounter(&startTime);
	for (int i = 0; i < LOGIC_LOOP; ++i)
	{
		pTest1 = new stTest32;
	}
	QueryPerformanceCounter(&endTime);
	elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
	wprintf(L"stTest32: %f\n", elapsedTime);

	QueryPerformanceCounter(&startTime);
	for (int i = 0; i < LOGIC_LOOP; ++i)
	{
		pTest2 = new stTest128;
	}
	QueryPerformanceCounter(&endTime);
	elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
	wprintf(L"stTest128: %f\n", elapsedTime);

	return 0;
}

#endif // ACTIVATE
