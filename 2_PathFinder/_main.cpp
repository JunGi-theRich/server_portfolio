
#pragma comment(lib, "Winmm")

#include <Windows.h>
#include <windowsx.h>

#include <iostream>
#include <string>
#include <list>

#include "define.h"
#include "Astar.h"
#include "JPS.h"

HBRUSH g_hPathBrush;
HBRUSH g_hWallBrush;
HBRUSH g_hStartBrush;
HBRUSH g_hDestBrush;
HBRUSH g_hNodeBrush;

HPEN g_hGridPen;
HPEN g_hPathPen;
HPEN g_hArrowPen;

HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;
HDC g_hMemDC;
RECT g_clientRect;

stTileNode g_startTile;
stTileNode g_destTile;

char g_tile[GRID_HEIGHT][GRID_WIDTH];
int g_curMode = 0;							// ���� ��ã�� ���

bool g_bSetStart = false;					// ����� ���� ���õǾ��ִ��� ����
bool g_bSetDest = false;					// ������ ���� ���õǾ��ִ��� ����
bool g_bDrawTile = false;					// Ÿ�� �׸��⸦ �����ߴ����� ���� ����
bool g_bDrawLine = false;					// ���� ���� ���� ��尡 ������ �� ���� �׸��� ����

bool g_bDragWall = false;					// ���� �巡�׸� ���� ���� ���� �� ������ �� �ֵ��� ����
bool g_bEraseWall = false;					// ���� �̹� �ִ� ���� �׸��⸦ �ϸ� ����� ���

void SetWNDCLASSEX(WNDCLASSEX* wcex);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void RenderGrid(HDC hdc);
void RenderTile(HDC hdc);
void RenderLine(HDC hdc);
void RenderTile_Arrow(HDC hdc, int xPos, int yPos, int dir);

cAstar astarFinder;
cJPS jpsFinder;

constexpr int WARMUP_LOOP = 10;
constexpr int LOGIC_LOOP = 100000;

constexpr int MINMAX_COUNT = 5;

LARGE_INTEGER freq;
LARGE_INTEGER startTime;
LARGE_INTEGER endTime;

float minTime[MINMAX_COUNT];
float maxTime[MINMAX_COUNT];
void IsBig(float time);
void IsSmall(float time);

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

	g_curMode = eCurMode::PATH_FIND_ASTAR;

	MSG msg;
	HWND hWnd;
	WNDCLASSEX wcex;
	SetWNDCLASSEX(&wcex);

	if (!RegisterClassEx(&wcex))
	{
		wprintf(L"RegisterClassEx Error: %d\n", GetLastError());
		return 1;
	}

	hWnd = CreateWindow(L"PathFinder", L"PathFinder", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900, NULL, NULL, NULL, NULL);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;

	return 0;
}

void SetWNDCLASSEX(WNDCLASSEX* wcex)
{
	wcex->cbSize = sizeof(WNDCLASSEX);
	wcex->style = CS_HREDRAW | CS_VREDRAW;
	wcex->lpfnWndProc = WndProc;
	wcex->cbClsExtra = 0;
	wcex->cbWndExtra = 0;
	wcex->hInstance = NULL;
	wcex->hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex->hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex->lpszMenuName = NULL;
	wcex->lpszClassName = L"PathFinder";
	wcex->hIconSm = NULL;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	GetClientRect(hWnd, &g_clientRect);

	switch (message)
	{
	case WM_SIZE:
	{
		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);

		HDC hdc = GetDC(hWnd);

		//GetClientRect(hWnd, &g_clientRect);
		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_clientRect.right, g_clientRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);

		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
	}
	break;

	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	{
		// ��Ŭ�� 1. ����Ʈ Ŭ���� ��� ������ ����
		if (GetAsyncKeyState(VK_SHIFT))
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / GRID_SIZE;
			int tileY = yPos / GRID_SIZE;

			if (0 > tileX || 0 > tileY || GRID_WIDTH <= tileX || GRID_HEIGHT <= tileY)
			{
				break;
			}

			// Ŭ���� ��ǥ�� �����ϴ� ������� ������ ��� ���õ�
			if (g_bSetStart && tileX == g_startTile._xPos && tileY == g_startTile._yPos)
			{
				break;
			}

			// �ٸ� ������ ��ǥ�� �������� �ʴ� ���
			if (!g_bSetDest)
			{
				g_tile[tileY][tileX] = TILE_DEST;
				wprintf(L"���� ��ǥ ����: (%d, %d)\n", tileX, tileY);

				// ���� ���� ����
				g_destTile._xPos = tileX;
				g_destTile._yPos = tileY;

				g_bSetDest = true;
				InvalidateRect(hWnd, NULL, false);
			}

			// �̹� �ٸ� ������ ��ǥ�� �����ϴ� ��� 
			else
			{
				// ��� 1. ���� ���� ��ǥ�� ������ ���� ���, ��ǥ�� �����ֱ⸸ �ϰ� ����
				if (tileX == g_destTile._xPos && tileY == g_destTile._yPos)
				{
					g_tile[g_destTile._yPos][g_destTile._xPos] = TILE_PATH;
					wprintf(L"���� ��ǥ ����: (%d, %d)\n", g_destTile._xPos, g_destTile._yPos);

					g_bSetDest = false;
					InvalidateRect(hWnd, NULL, false);
				}

				// ��� 2. ���� ���� ��ǥ�� ������ �ٸ� ���, ���� ��ǥ ������ ����ϰ� �� ��ǥ ���
				else
				{
					g_tile[g_destTile._yPos][g_destTile._xPos] = TILE_PATH;
					wprintf(L"���� ��ǥ ����: (%d, %d)\n", g_destTile._xPos, g_destTile._yPos);

					g_tile[tileY][tileX] = TILE_DEST;
					wprintf(L"���� ��ǥ ����: (%d, %d)\n", tileX, tileY);

					// ���� ���� ����
					g_destTile._xPos = tileX;
					g_destTile._yPos = tileY;

					InvalidateRect(hWnd, NULL, false);
				}
			}
		}

		// ��Ŭ�� 2. �Ϲ� Ŭ���� ��� ��߽� ����
		else
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / GRID_SIZE;
			int tileY = yPos / GRID_SIZE;

			if (0 > tileX || 0 > tileY || GRID_WIDTH <= tileX || GRID_HEIGHT <= tileY)
			{
				break;
			}

			// Ŭ���� ��ǥ�� �����ϴ� �������� ������ ��� ���õ�
			if (g_bSetDest && tileX == g_destTile._xPos && tileY == g_destTile._yPos)
			{
				break;
			}

			// �ٸ� ����� ��ǥ�� �������� �ʴ� ���
			if (!g_bSetStart)
			{
				g_tile[tileY][tileX] = TILE_START;
				wprintf(L"��� ��ǥ ����: (%d, %d)\n", tileX, tileY);

				// ���� ���� ����
				g_startTile._xPos = tileX;
				g_startTile._yPos = tileY;

				g_bSetStart = true;
				InvalidateRect(hWnd, NULL, false);
			}

			// �̹� �ٸ� ����� ��ǥ�� �����ϴ� ���
			else
			{
				// ��� 1. ���� ���� ��ǥ�� ������ ���� ���, ��ǥ�� �����ֱ⸸ �ϰ� ����
				if (tileX == g_startTile._xPos && tileY == g_startTile._yPos)
				{
					g_tile[g_startTile._yPos][g_startTile._xPos] = TILE_PATH;
					wprintf(L"��� ��ǥ ����: (%d, %d)\n", g_startTile._xPos, g_startTile._yPos);

					g_bSetStart = false;
					InvalidateRect(hWnd, NULL, false);
				}

				// ��� 2. ���� ���� ��ǥ�� ������ �ٸ� ���, ���� ��ǥ�� ����ϰ� �� ��ǥ ���
				else
				{
					g_tile[g_startTile._yPos][g_startTile._xPos] = TILE_PATH;
					wprintf(L"��� ��ǥ ����: (%d, %d)\n", g_startTile._xPos, g_startTile._yPos);

					g_tile[tileY][tileX] = TILE_START;
					wprintf(L"��� ��ǥ ����: (%d, %d)\n", tileX, tileY);

					// ���� ���� ����
					g_startTile._xPos = tileX;
					g_startTile._yPos = tileY;

					InvalidateRect(hWnd, NULL, false);
				}
			}
		}
	}
	break;

	case WM_RBUTTONDOWN:
	{
		g_bDragWall = true;

		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int tileX = xPos / GRID_SIZE;
		int tileY = yPos / GRID_SIZE;

		if (0 > tileX || 0 > tileY || GRID_WIDTH <= tileX || GRID_HEIGHT <= tileY)
		{
			break;
		}

		// ù ���� Ÿ���� ���� ��� ���� ���, �ƴ� ��� �� ���� ���
		if (g_tile[tileY][tileX] == TILE_WALL)
		{
			g_bEraseWall = true;
		}
		else
		{
			g_bEraseWall = false;
		}
	}
	break;
	case WM_RBUTTONUP:
	{
		if (g_bDragWall)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / GRID_SIZE;
			int tileY = yPos / GRID_SIZE;

			if (0 > tileX || 0 > tileY || GRID_WIDTH <= tileX || GRID_HEIGHT <= tileY)
			{
				break;
			}

			// ���� �׸��� ���� ����� �Ǵ� �������� �ִ� ���� �� ���� ����
			if ((tileX == g_startTile._xPos && tileY == g_startTile._yPos)
				|| (tileX == g_destTile._xPos && tileY == g_destTile._yPos))
			{
				break;
			}

			// �巡�� �߻� �ø��� ȭ�� ����
			if (g_bEraseWall)
			{
				g_tile[tileY][tileX] = TILE_PATH;
			}
			else
			{
				g_tile[tileY][tileX] = TILE_WALL;
			}
			InvalidateRect(hWnd, NULL, false);
		}

		g_bDragWall = false;
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (g_bDragWall)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / GRID_SIZE;
			int tileY = yPos / GRID_SIZE;

			if (0 > tileX || 0 > tileY || GRID_WIDTH <= tileX || GRID_HEIGHT <= tileY)
			{
				break;
			}

			// ���� �׸��� ���� ����� �Ǵ� �������� �ִ� ���� �� ���� ����
			if ((tileX == g_startTile._xPos && tileY == g_startTile._yPos)
				|| (tileX == g_destTile._xPos && tileY == g_destTile._yPos))
			{
				break;
			}

			// �巡�� �߻� �ø��� ȭ�� ����
			if (g_bEraseWall)
			{
				g_tile[tileY][tileX] = TILE_PATH;
			}
			else
			{
				g_tile[tileY][tileX] = TILE_WALL;
			}
			InvalidateRect(hWnd, NULL, false);
		}
	}
	break;

	case WM_KEYDOWN:
	{
		if (0x31 == wParam)
		{
			if (g_bDrawLine)
			{
				MessageBox(hWnd, L"��θ� �켱 ������ ��", L"Error", MB_OK | MB_ICONERROR);
				break;
			}
			g_curMode = PATH_FIND_ASTAR;
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		else if (0x32 == wParam)
		{
			if (g_bDrawLine)
			{
				MessageBox(hWnd, L"��θ� �켱 ������ ��", L"Error", MB_OK | MB_ICONERROR);
				break;
			}
			g_curMode = PATH_FIND_JPS;
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		else if (0x33 == wParam)
		{
			if (g_bDrawLine)
			{
				MessageBox(hWnd, L"��θ� �켱 ������ ��", L"Error", MB_OK | MB_ICONERROR);
				break;
			}
			g_curMode = PATH_FIND_COMPARE;
			InvalidateRect(hWnd, NULL, false);
			break;
		}

		if (VK_SPACE == wParam)
		{
			// ������� ������ ������ ���� ��� ��ã�� ���� �������� ����
			if (!g_bSetStart || !g_bSetDest)
			{
				MessageBox(hWnd, L"����� �Ǵ� ������ ���� ����", L"Error", MB_OK | MB_ICONERROR);
				break;
			}

			// �̹� ��ã�⸦ �����Ͽ� ��� �� �� �׸��Ⱑ �Ϸ�� ��� ���� ��带 ������
			else if (g_bDrawTile)
			{
				g_bDrawTile = false;
				g_bDrawLine = false;
				astarFinder.ClearPath();
				jpsFinder.ClearPath();
				InvalidateRect(hWnd, NULL, false);
				break;
			}

			// ���� ��忡 ���� ��ã�� ����
			switch (g_curMode)
			{
			case PATH_FIND_ASTAR:
				if (astarFinder.StartPathFind())
				{
					g_bDrawLine = true;
				}
				else
				{
					MessageBox(hWnd, L"��� ã�� ����", L"Error", MB_OK | MB_ICONERROR);
				}
				InvalidateRect(hWnd, NULL, false);
				g_bDrawTile = true;
				break;

			case PATH_FIND_JPS:
				if (jpsFinder.StartPathFind())
				{
					g_bDrawLine = true;
				}
				else
				{
					MessageBox(hWnd, L"��� ã�� ����", L"Error", MB_OK | MB_ICONERROR);
				}
				InvalidateRect(hWnd, NULL, false);
				g_bDrawTile = true;
				break;

			case PATH_FIND_COMPARE:
			{
				// Astar�� JPS�� ��� �켱 �������� ���� üũ�ϰ�, ��� ����� Ư�� ����

				if (!astarFinder.StartPathFind())
				{
					MessageBox(hWnd, L"��� ã�� ����", L"Error", MB_OK | MB_ICONERROR);
					g_bDrawTile = true;
					break;
				}
				astarFinder.ClearPath();
				if (!jpsFinder.StartPathFind())
				{
					MessageBox(hWnd, L"��� ã�� ����", L"Error", MB_OK | MB_ICONERROR);
					g_bDrawTile = true;
					break;
				}
				jpsFinder.ClearPath();

				std::wstring printStr = L"���� ��� (���� Ƚ��: ";
				printStr += std::to_wstring(LOGIC_LOOP); printStr += L")\n";

				float totalTime = 0;
				float finalAvg = 0;
				memset(minTime, 0, sizeof(minTime));
				memset(maxTime, 0, sizeof(maxTime));
				for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
				{
					QueryPerformanceCounter(&startTime);
					astarFinder.StartPathFind();
					QueryPerformanceCounter(&endTime);

					astarFinder.ClearPath();
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
				printStr += L"A*: "; printStr += std::to_wstring(finalAvg); printStr += L"us\n";

				totalTime = 0;
				finalAvg = 0;
				memset(minTime, 0, sizeof(minTime));
				memset(maxTime, 0, sizeof(maxTime));
				for (int loopCnt = 0; loopCnt < LOGIC_LOOP; ++loopCnt)
				{
					QueryPerformanceCounter(&startTime);
					jpsFinder.StartPathFind();
					QueryPerformanceCounter(&endTime);

					jpsFinder.ClearPath();
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
				printStr += L"JPS: "; printStr += std::to_wstring(finalAvg); printStr += L"us\n";

				MessageBox(hWnd, printStr.c_str(), L"Result", MB_OK);
				break;
			}
			default:
				break;
			}
		}
	}
	break;

	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		PatBlt(g_hMemDC, 0, 0, g_clientRect.right, g_clientRect.bottom, WHITENESS);
		RenderTile(g_hMemDC);
		RenderGrid(g_hMemDC);

		if (g_bSetStart && g_bSetDest && g_bDrawLine)
		{
			RenderLine(g_hMemDC);
		}
		BitBlt(hdc, 0, 0, g_clientRect.right, g_clientRect.bottom, g_hMemDC, 0, 0, SRCCOPY);


		HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		wchar_t description1[] = L"��Ŭ��: ����� ����, Shit + ��Ŭ��: ������ ����";
		wchar_t description2[] = L"��Ŭ��: �� ���� �Ǵ� ����";
		wchar_t description3[] = L"Space: �� ã�� ����";
		wchar_t description4[] = L"1: A*, 2: JPS, 3: ���� �� ���";
		std::wstring description5(L"���� ���: ");
		if (eCurMode::PATH_FIND_ASTAR == g_curMode) description5 += L"A*";
		else if (eCurMode::PATH_FIND_JPS == g_curMode) description5 += L"JPS";
		else description5 += L"Compare";

		//!!�׸��� �ʺ�� ���� Ű��ٸ� �� ���鵵 �ٲ��� ��
		TextOut(hdc, 1300, 50, description1, (int)wcslen(description1));
		TextOut(hdc, 1300, 70, description2, (int)wcslen(description2));
		TextOut(hdc, 1300, 90, description3, (int)wcslen(description2));
		TextOut(hdc, 1300, 110, description4, (int)wcslen(description4));
		TextOut(hdc, 1300, 130, description5.c_str(), (int)description5.size());
		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_CREATE:
	{
		hdc = GetDC(hWnd);

		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_clientRect.right, g_clientRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);

		g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		g_hPathPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		g_hArrowPen = CreatePen(PS_SOLID, 2, RGB(200, 150, 150));

		g_hPathBrush = CreateSolidBrush(RGB(255, 255, 255));
		g_hDestBrush = CreateSolidBrush(RGB(0, 255, 0));
		g_hStartBrush = CreateSolidBrush(RGB(0, 0, 255));
		g_hWallBrush = CreateSolidBrush(RGB(100, 100, 100));
		g_hNodeBrush = CreateSolidBrush(RGB(255, 200, 100));
	}
	break;

	case WM_DESTROY:
	{
		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);

		PostQuitMessage(0);
	}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}


void RenderGrid(HDC hdc)
{
	int xDraw = 0;
	int yDraw = 0;
	HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

	for (int iCntW = 0; iCntW <= GRID_WIDTH; ++iCntW)
	{
		MoveToEx(hdc, xDraw, 0, NULL);
		LineTo(hdc, xDraw, GRID_HEIGHT * GRID_SIZE);
		xDraw += GRID_SIZE;
	}
	for (int iCntH = 0; iCntH <= GRID_HEIGHT; ++iCntH)
	{
		MoveToEx(hdc, 0, yDraw, NULL);
		LineTo(hdc, GRID_WIDTH * GRID_SIZE, yDraw);
		yDraw += GRID_SIZE;
	}
	SelectObject(hdc, hOldPen);
}
void RenderTile(HDC hdc)
{
	int xDraw = 0;
	int yDraw = 0;

	for (int iCntW = 0; iCntW < GRID_WIDTH; ++iCntW)
	{
		for (int iCntH = 0; iCntH < GRID_HEIGHT; ++iCntH)
		{
			switch (g_tile[iCntH][iCntW])
			{
			case TILE_PATH:
			{
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hPathBrush);
				SelectObject(hdc, GetStockObject(NULL_PEN));

				xDraw = iCntW * GRID_SIZE;
				yDraw = iCntH * GRID_SIZE;

				Rectangle(hdc, xDraw, yDraw, xDraw + GRID_SIZE + 2, yDraw + GRID_SIZE + 2);
				SelectObject(hdc, hOldBrush);
			}
			break;

			case TILE_WALL:
			{
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hWallBrush);
				SelectObject(hdc, GetStockObject(NULL_PEN));

				xDraw = iCntW * GRID_SIZE;
				yDraw = iCntH * GRID_SIZE;

				Rectangle(hdc, xDraw, yDraw, xDraw + GRID_SIZE + 2, yDraw + GRID_SIZE + 2);
				SelectObject(hdc, hOldBrush);
			}
			break;

			case TILE_START:
			{
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hStartBrush);
				SelectObject(hdc, GetStockObject(NULL_PEN));

				xDraw = iCntW * GRID_SIZE;
				yDraw = iCntH * GRID_SIZE;

				Rectangle(hdc, xDraw, yDraw, xDraw + GRID_SIZE + 2, yDraw + GRID_SIZE + 2);
				SelectObject(hdc, hOldBrush);
			}
			break;

			case TILE_DEST:
			{
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hDestBrush);
				SelectObject(hdc, GetStockObject(NULL_PEN));

				xDraw = iCntW * GRID_SIZE;
				yDraw = iCntH * GRID_SIZE;

				Rectangle(hdc, xDraw, yDraw, xDraw + GRID_SIZE + 2, yDraw + GRID_SIZE + 2);
				SelectObject(hdc, hOldBrush);
			}
			break;

			case TILE_NODE_NORMAL:
			case TILE_NODE_LL:
			case TILE_NODE_LU:
			case TILE_NODE_UU:
			case TILE_NODE_RU:
			case TILE_NODE_RR:
			case TILE_NODE_RD:
			case TILE_NODE_DD:
			case TILE_NODE_LD:
			{
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hNodeBrush);
				SelectObject(hdc, GetStockObject(NULL_PEN));

				xDraw = iCntW * GRID_SIZE;
				yDraw = iCntH * GRID_SIZE;

				Rectangle(hdc, xDraw, yDraw, xDraw + GRID_SIZE + 2, yDraw + GRID_SIZE + 2);
				RenderTile_Arrow(hdc, xDraw + GRID_SIZE / 2, yDraw + GRID_SIZE / 2, g_tile[iCntH][iCntW]);		// �׸��� ��ǥ ������ �簢���� ����

				SelectObject(hdc, hOldBrush);
			}
			break;

			default:
				abort();		// �� ���� ���� �ȵ�
				break;
			}
		}
	}
}
void RenderLine(HDC hdc)
{
	int xStartLine;
	int yStartLine;

	int xDestLine;
	int yDestLine;

	// ������ ����� �θ� ������ ������ ���󰡸� ���� �׷���
	HPEN hOldPen = (HPEN)SelectObject(hdc, g_hPathPen);
	stTileNode* pDraw = &g_destTile;

	while (1)
	{
		if (pDraw == &g_startTile)
		{
			break;
		}

		xStartLine = pDraw->_xPos * GRID_SIZE + 8;
		yStartLine = pDraw->_yPos * GRID_SIZE + 8;

		xDestLine = pDraw->_pParent->_xPos * GRID_SIZE + 8;
		yDestLine = pDraw->_pParent->_yPos * GRID_SIZE + 8;

		MoveToEx(hdc, xStartLine, yStartLine, NULL);
		LineTo(hdc, xDestLine, yDestLine);

		pDraw = pDraw->_pParent;
	}
}
void RenderTile_Arrow(HDC hdc, int xPos, int yPos, int dir)
{
	HPEN hOldPen = (HPEN)SelectObject(hdc, g_hArrowPen);
	switch (dir)
	{
	case DIR_NONE:
		break;

	case DIR_LL:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 4, yPos - GRID_SIZE / 4);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 4, yPos + GRID_SIZE / 4);
		break;

	case DIR_LU:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 3, yPos);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos, yPos + GRID_SIZE / 3);
		break;

	case DIR_UU:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 4, yPos + GRID_SIZE / 4);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 4, yPos + GRID_SIZE / 4);
		break;

	case DIR_RU:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 3, yPos);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos, yPos + GRID_SIZE / 3);
		break;

	case DIR_RR:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 4, yPos - GRID_SIZE / 4);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 4, yPos + GRID_SIZE / 4);
		break;

	case DIR_RD:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos, yPos - GRID_SIZE / 3);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 3, yPos);
		break;

	case DIR_DD:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 4, yPos - GRID_SIZE / 4);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos - GRID_SIZE / 4, yPos - GRID_SIZE / 4);
		break;

	case DIR_LD:
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos, yPos - GRID_SIZE / 3);
		MoveToEx(hdc, xPos, yPos, NULL);
		LineTo(hdc, xPos + GRID_SIZE / 3, yPos);
		break;

	default:
		abort();
		break;
	}
}

void IsBig(float time)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == maxTime[i] || time > maxTime[i])
		{
			std::swap(maxTime[i], time);
		}
	}
}
void IsSmall(float time)
{
	for (int i = 0; i < MINMAX_COUNT; ++i)
	{
		if (0 == minTime[i] || time < minTime[i])
		{
			std::swap(minTime[i], time);
		}
	}
}