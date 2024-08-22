
#pragma comment(lib, "winmm")

#include <Windows.h>

#include <random>

#include "myList.h"
#include "myBST.h"
#include "myRBT.h"

using std::vector;
using std::swap;
using std::shuffle;
using std::random_device;
using std::mt19937;

vector<int> inputVec;
vector<int> eraseVec;

constexpr int LOGIC_LOOP = 500000;
constexpr int INPUT_SIZE = 2000;

constexpr int MINMAX_COUNT = 5;

LARGE_INTEGER freq;

float listTotalTime;
float listAvgTime;
float listMinArr[MINMAX_COUNT];
float listMaxArr[MINMAX_COUNT];

float bstTotalTime;
float bstAvgTime;
float bstMinArr[MINMAX_COUNT];
float bstMaxArr[MINMAX_COUNT];

float rbtTotalTime;
float rbtAvgTime;
float rbtMinArr[MINMAX_COUNT];
float rbtMaxArr[MINMAX_COUNT];

void IsBig(float time, float* pMaxArr);
void IsSmall(float time, float* pMinArr);

cList<int> myList;
cBST<int> myBST;
cRBT<int> myRBT;

void ResetContainer();

int main()
{
	_wsetlocale(LC_ALL, L"KOR");
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&freq);
	if (!SetProcessAffinityMask(GetCurrentProcess(), 2))
	{
		wprintf(L"SetProcessAffinityMask Failed due to: %d\n", GetLastError());
		system("pause");
		return 1;
	}

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;

	// 데이터 랜덤 삽입 및 삭제를 위한 준비 작업
	inputVec.resize(INPUT_SIZE);
	eraseVec.resize(INPUT_SIZE);
	for (int i = 0; i < INPUT_SIZE; ++i)
	{
		inputVec[i] = i + 1;
		eraseVec[i] = i + 1;
	}
	random_device rd;
	mt19937 rng(rd());

	wprintf(L"\n삽입 및 삭제 데이터 수: %d\n", INPUT_SIZE);
	wprintf(L"삽입 및 삭제 로직 시행 횟수: %d\n", LOGIC_LOOP);


	wprintf(L"\n====================\n");
	wprintf(L"데이터 삽입 소요 시간 측정\n");
	ResetContainer();
	for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
	{
		float elapsedTime;
		shuffle(inputVec.begin(), inputVec.end(), rng);

		QueryPerformanceCounter(&startTime);
		for (int inputCnt = 0; inputCnt < INPUT_SIZE; ++inputCnt)
		{
			myList.push_back(inputVec[inputCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		listTotalTime += elapsedTime;
		IsSmall(elapsedTime, listMinArr);
		IsBig(elapsedTime, listMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int inputCnt = 0; inputCnt < INPUT_SIZE; ++inputCnt)
		{
			myBST.insert(inputVec[inputCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		bstTotalTime += elapsedTime;
		IsSmall(elapsedTime, bstMinArr);
		IsBig(elapsedTime, bstMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int inputCnt = 0; inputCnt < INPUT_SIZE; ++inputCnt)
		{
			myRBT.insert(inputVec[inputCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		rbtTotalTime += elapsedTime;
		IsSmall(elapsedTime, rbtMinArr);
		IsBig(elapsedTime, rbtMaxArr);

		myList.clear();
		myBST.clear();
		myRBT.clear();
	}
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		listTotalTime = listTotalTime - listMinArr[i] - listMaxArr[i];
		bstTotalTime = bstTotalTime - bstMinArr[i] - bstMaxArr[i];
		rbtTotalTime = rbtTotalTime - rbtMinArr[i] - rbtMaxArr[i];
	}
	listAvgTime = listTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	bstAvgTime = bstTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	rbtAvgTime = rbtTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	wprintf(L"list 삽입 평균 소요 시간: %f us\n", listAvgTime);
	wprintf(L"BST 삽입 평균 소요 시간: %f us\n", bstAvgTime);
	wprintf(L"RBT 삽입 평균 소요 시간: %f us\n", rbtAvgTime);


	wprintf(L"\n\n====================\n");
	wprintf(L"무작위 데이터 100개 탐색 소요 시간 측정\n");
	ResetContainer();
	for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
	{
		float elapsedTime;
		shuffle(inputVec.begin(), inputVec.end(), rng);
		shuffle(eraseVec.begin(), eraseVec.end(), rng);

		for (int inputCnt = 0; inputCnt < INPUT_SIZE; ++inputCnt)
		{
			myList.push_back(inputVec[inputCnt]);
			myBST.insert(inputVec[inputCnt]);
			myRBT.insert(inputVec[inputCnt]);
		}

		QueryPerformanceCounter(&startTime);
		for (int searchCnt = 0; searchCnt < 100; ++searchCnt)
		{
			myList.find(eraseVec[searchCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		listTotalTime += elapsedTime;
		IsSmall(elapsedTime, listMinArr);
		IsBig(elapsedTime, listMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int searchCnt = 0; searchCnt < 100; ++searchCnt)
		{
			myBST.find(eraseVec[searchCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		bstTotalTime += elapsedTime;
		IsSmall(elapsedTime, bstMinArr);
		IsBig(elapsedTime, bstMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int searchCnt = 0; searchCnt < 100; ++searchCnt)
		{
			myRBT.find(eraseVec[searchCnt]);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		rbtTotalTime += elapsedTime;
		IsSmall(elapsedTime, rbtMinArr);
		IsBig(elapsedTime, rbtMaxArr);

		myList.clear();
		myBST.clear();
		myRBT.clear();
	}
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		listTotalTime = listTotalTime - listMinArr[i] - listMaxArr[i];
		bstTotalTime = bstTotalTime - bstMinArr[i] - bstMaxArr[i];
		rbtTotalTime = rbtTotalTime - rbtMinArr[i] - rbtMaxArr[i];
	}
	listAvgTime = listTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	bstAvgTime = bstTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	rbtAvgTime = rbtTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	wprintf(L"list 탐색 평균 소요 시간: %f us\n", listAvgTime);
	wprintf(L"BST 탐색 평균 소요 시간: %f us\n", bstAvgTime);
	wprintf(L"RBT 탐색 평균 소요 시간: %f us\n", rbtAvgTime);


	wprintf(L"\n\n====================\n");
	wprintf(L"데이터 삭제 소요 시간 측정\n");
	ResetContainer();
	for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
	{
		float elapsedTime;
		shuffle(inputVec.begin(), inputVec.end(), rng);
		shuffle(eraseVec.begin(), eraseVec.end(), rng);

		for (int inputCnt = 0; inputCnt < INPUT_SIZE; ++inputCnt)
		{
			myList.push_back(inputVec[inputCnt]);
			myBST.insert(inputVec[inputCnt]);
			myRBT.insert(inputVec[inputCnt]);
		}

		QueryPerformanceCounter(&startTime);
		for (int eraseCnt = 0; eraseCnt < 100; ++eraseCnt)
		{
			cList<int>::iterator iter = myList.begin();
			for (iter; iter != myList.end();)
			{
				if (eraseVec[eraseCnt] == *iter)
				{
					iter = myList.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		listTotalTime += elapsedTime;
		IsSmall(elapsedTime, listMinArr);
		IsBig(elapsedTime, listMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int eraseCnt = 0; eraseCnt < 100; ++eraseCnt)
		{
			cBST<int>::iterator iter;
			iter = myBST.find(eraseVec[eraseCnt]);
			myBST.erase(iter);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		bstTotalTime += elapsedTime;
		IsSmall(elapsedTime, bstMinArr);
		IsBig(elapsedTime, bstMaxArr);

		QueryPerformanceCounter(&startTime);
		for (int eraseCnt = 0; eraseCnt < 100; ++eraseCnt)
		{
			cRBT<int>::iterator iter;
			iter = myRBT.find(eraseVec[eraseCnt]);
			myRBT.erase(iter);
		}
		QueryPerformanceCounter(&endTime);
		elapsedTime = (float)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * 1000000;
		rbtTotalTime += elapsedTime;
		IsSmall(elapsedTime, rbtMinArr);
		IsBig(elapsedTime, rbtMaxArr);

		myList.clear();
		myBST.clear();
		myRBT.clear();
	}
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		listTotalTime = listTotalTime - listMinArr[i] - listMaxArr[i];
		bstTotalTime = bstTotalTime - bstMinArr[i] - bstMaxArr[i];
		rbtTotalTime = rbtTotalTime - rbtMinArr[i] - rbtMaxArr[i];
	}
	listAvgTime = listTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	bstAvgTime = bstTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	rbtAvgTime = rbtTotalTime / (LOGIC_LOOP - MINMAX_COUNT * 2);
	wprintf(L"list 삭제 평균 소요 시간: %f us\n", listAvgTime);
	wprintf(L"BST 삭제 평균 소요 시간: %f us\n", bstAvgTime);
	wprintf(L"RBT 삭제 평균 소요 시간: %f us\n", rbtAvgTime);
	

	system("pause");
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

void ResetContainer()
{
	listTotalTime = 0;
	listAvgTime = 0;
	memset(listMinArr, 0, sizeof(listMinArr));
	memset(listMaxArr, 0, sizeof(listMaxArr));

	bstTotalTime = 0;
	bstAvgTime = 0;
	memset(bstMinArr, 0, sizeof(bstMinArr));
	memset(bstMaxArr, 0, sizeof(bstMaxArr));

	rbtTotalTime = 0;
	rbtAvgTime = 0;
	memset(rbtMinArr, 0, sizeof(rbtMinArr));
	memset(rbtMaxArr, 0, sizeof(rbtMaxArr));
}