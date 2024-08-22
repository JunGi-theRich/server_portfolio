
#include "JPS.h"
extern char g_tile[][GRID_WIDTH];
extern stTileNode g_startTile;
extern stTileNode g_destTile;

bool cJPS::StartPathFind()
{
	// 경로 탐색 기본 로직
	// 1. 출발 지점을 제외한 모든 노드들은 고유의 방향성(unique direction)을 보유함
	// 2. 각 노드들은 부모 노드에 의해 탐색이 수행된 구간을 제외한 경로를 탐색함
	// 3. 경로 탐색 중 코너(강제 이웃)를 만나게 된 경우, 해당 구간은 부모 노드에 의해 탐색되지 않았다고 봄
	// 
	// 4. '직선 방향성'을 갖는 노드의 탐색
	// 4-1. 진행 방향 기준 수직 좌표(예를 들어, 우측 탐색일 경우 현재 위치에서 '위와 아래')에 벽이 있는 경우,
	// 그리고 동시에 대각선 방향으로 이동이 가능한 경우에 코너로 판단함
	// 4-2. 코너를 판단한 지점에 노드를 생성함
	// 이 노드의 부모 노드는 탐색을 시작한 노드이며, 방향성 또한 그대로 물려받음
	// 4-3. 이 과정을 코너를 발견하거나 이동이 불가능할 때까지 자신의 방향으로 한 칸씩 이동하며 진행
	// 
	// 5. '대각선 방향성'을 갖는 노드의 탐색
	// 5-1. 진행 방향 기준 135도 좌표(예를 들어, 우상단 탐색일 경우 현재 위치에서 '왼쪽과 아래')에 벽이 있는 경우,
	// 그리고 동시에 수직 방향으로 이동이 가능한 경우 코너로 판단함
	// 5-2. 코너를 찾지 못했다면 수직 및 수평 방향의 탐색을 수행함(예를 들어 우상단 탐색일 경우 현재 위치에서 '오른쪽과 위')
	// 탐색 과정에서 코너를 발견하였다면 수직 및 수평 방향 탐색을 시작한 지점에 노드를 생성함
	// 5-3. 이 과정을 코너를 발견하거나 이동이 불가능할 때까지 자신의 방향으로 한 칸씩 이동하며 진행

	stTileNode* pCurNode;
	int xStart = g_startTile._xPos;
	int yStart = g_startTile._yPos;
	int xEnd = g_destTile._xPos;
	int yEnd = g_destTile._yPos;

	// gValue는 유클리드, hValue는 맨해튼 방식으로 진행
	g_startTile._gValue = 0;
	g_startTile._hValue = (abs(xEnd - xStart) + (abs(yEnd - yStart)));
	g_startTile._fValue = g_startTile._gValue * G_WEIGHT + g_startTile._hValue * H_WEIGHT;

	pCurNode = &g_startTile;
	_closeList.push_back(pCurNode);


	// 시작 지점에서 8방향 경로 탐색
	PathCheckLL(xStart, yStart, pCurNode);
	PathCheckUU(xStart, yStart, pCurNode);
	PathCheckRR(xStart, yStart, pCurNode);
	PathCheckDD(xStart, yStart, pCurNode);
	PathCheckLU(xStart, yStart, pCurNode);
	PathCheckRU(xStart, yStart, pCurNode);
	PathCheckRD(xStart, yStart, pCurNode);
	PathCheckLD(xStart, yStart, pCurNode);

	// _openList 순회하며 가장 작은 f값을 갖는 도를 찾고, 그 노드의 방향성을 기준으로 경로 탐색
	while (1)
	{
		list<stTileNode*>::iterator openIter = _openList.begin();
		list<stTileNode*>::iterator eraseIter = openIter;
		if (_openList.empty())
		{
			// 경로 없음 윈도우 메시지
			return false;
		}

		double fVal = (*openIter)->_fValue;

		for (openIter; openIter != _openList.end(); ++openIter)
		{
			if ((*openIter)->_xPos == xEnd && (*openIter)->_yPos == yEnd)
			{
				// 전역 도착점 노드의 부모 포인터 갱신 및 타일 속성 유지를 위해 변경 후 함수 종료

				pCurNode = *openIter;
				g_destTile._gValue = GetGvalue(pCurNode->_pParent, pCurNode);
				g_destTile._hValue = 0;
				g_destTile._fValue = GetFvalue(pCurNode);
				g_destTile._pParent = pCurNode->_pParent;

				g_tile[yEnd][xEnd] = TILE_DEST;
				return true;
			}

			if (fVal > (*openIter)->_fValue)
			{
				// 더 작은 f값이 있으므로 갱신
				fVal = (*openIter)->_fValue;
				eraseIter = openIter;
			}
		}

		pCurNode = *eraseIter;
		_closeList.push_back(pCurNode);
		_openList.erase(eraseIter);

		switch (pCurNode->_nodeDir)
		{
		case DIR_NONE:
			DebugBreak();		// 이곳에 오면 안됨
			break;

		case DIR_LL:
			PathCheckLL(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1					// 위쪽 벽 체크
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// 아래쪽 벽 체크
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			break;

		case DIR_LU:
			PathCheckLU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckLL(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckUU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1 		// 아래쪽 벽 체크
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1		// 오른쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			break;

		case DIR_UU:
			PathCheckUU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1					// 왼쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1		// 오른쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			break;

		case DIR_RU:
			PathCheckRU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckRR(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckUU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// 아래쪽 벽 체크
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1								// 왼쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			break;

		case DIR_RR:
			PathCheckRR(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1					// 위쪽 벽 체크
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// 아래쪽 벽 체크
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			break;

		case DIR_RD:
			PathCheckRD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckRR(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckDD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1					// 위쪽 벽 체크
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1					// 왼쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			break;

		case DIR_DD:
			PathCheckDD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1					// 왼쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// 오른쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			break;

		case DIR_LD:
			PathCheckLD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckLL(pCurNode->_xPos, pCurNode->_yPos, pCurNode);
			PathCheckDD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1								// 위쪽 벽 체크
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// 오른쪽 벽 체크
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			break;

		default:
			DebugBreak();		// 이곳에 오면 안됨
			break;
		}
	}

	return true;
}

stTileNode* cJPS::MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir)
{
	stTileNode* pNewTile = GetTilePtrInfo(xPos, yPos);
	if (nullptr == pNewTile)
	{
		pNewTile = new stTileNode(xPos, yPos, pParentTile, dir);
		pNewTile->_gValue = GetGvalue(pParentTile, pNewTile);
		pNewTile->_hValue = GetHvalue(&g_destTile, pNewTile);
		pNewTile->_fValue = GetFvalue(pNewTile);

		g_tile[yPos][xPos] = dir;
		_openList.push_back(pNewTile);
	}
	return pNewTile;
}

bool cJPS::PathCheckLL(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	bool bUpCorner = false;
	bool bDownCorner = false;

	int xPos = curXPos - 1;
	int yPos = curYPos;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (0 <= xPos - 1 && IsPath(xPos - 1, yPos))
		{
			if (0 <= yPos - 1 && g_tile[yPos - 1][xPos] == TILE_WALL && g_tile[yPos - 1][xPos - 1] != TILE_WALL)
			{
				bUpCorner = true;
			}
			if (GRID_HEIGHT - 1 >= yPos + 1 && g_tile[yPos + 1][xPos] == TILE_WALL && g_tile[yPos + 1][xPos - 1] != TILE_WALL)
			{
				bDownCorner = true;
			}
		}
		if (bUpCorner || bDownCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_LL);
			}
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		--xPos;
	}

	return false;
}
bool cJPS::PathCheckLU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	if ((IsTile(curXPos - 1, curYPos) && !IsPath(curXPos - 1, curYPos)) &&
		(IsTile(curXPos, curYPos - 1) && !IsPath(curXPos, curYPos - 1)))  return false;

	bool bDownCorner = false;
	bool bRightCorner = false;

	int xPos = curXPos - 1;
	int yPos = curYPos - 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if ((IsTile(xPos - 1, yPos) && !IsPath(xPos - 1, yPos)) &&
			(IsTile(xPos, yPos - 1) && !IsPath(xPos, yPos - 1))) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (0 <= xPos - 1 && IsPath(xPos - 1, yPos))
		{
			if (GRID_HEIGHT - 1 >= yPos + 1 && g_tile[yPos + 1][xPos] == TILE_WALL && g_tile[yPos + 1][xPos - 1] != TILE_WALL)
			{
				bDownCorner = true;
			}
		}
		if (0 <= yPos - 1 && IsPath(xPos, yPos - 1))
		{
			if (GRID_WIDTH - 1 >= xPos + 1 && g_tile[yPos][xPos + 1] == TILE_WALL && g_tile[yPos - 1][xPos + 1] != TILE_WALL)
			{
				bRightCorner = true;
			}
		}
		if (bDownCorner || bRightCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_LU);
			}
			return true;
		}

		// 수직 및 수평 탐색
		if (PathCheckLL(xPos, yPos, pParentTile, false) || PathCheckUU(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_LU);
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		--xPos;
		--yPos;
	}

	return false;
}
bool cJPS::PathCheckUU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	bool bLeftCorner = false;
	bool bRightCorner = false;

	int xPos = curXPos;
	int yPos = curYPos - 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (0 <= yPos - 1 && IsPath(xPos, yPos - 1))
		{
			if (0 <= xPos - 1 && g_tile[yPos][xPos - 1] == TILE_WALL && g_tile[yPos - 1][xPos - 1] != TILE_WALL)
			{
				bLeftCorner = true;
			}
			if (GRID_WIDTH - 1 >= xPos + 1 && g_tile[yPos][xPos + 1] == TILE_WALL && g_tile[yPos - 1][xPos + 1] != TILE_WALL)
			{
				bRightCorner = true;
			}
		}
		if (bLeftCorner || bRightCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_UU);
			}
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		--yPos;
	}

	return false;
}
bool cJPS::PathCheckRU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	if ((IsTile(curXPos + 1, curYPos) && !IsPath(curXPos + 1, curYPos)) &&
		(IsTile(curXPos, curYPos - 1) && !IsPath(curXPos, curYPos - 1)))  return false;

	bool bDownCorner = false;
	bool bLeftCorner = false;

	int xPos = curXPos + 1;
	int yPos = curYPos - 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if ((IsTile(xPos + 1, yPos) && !IsPath(xPos + 1, yPos)) &&
			(IsTile(xPos, yPos - 1) && !IsPath(xPos, yPos - 1))) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (GRID_WIDTH - 1 >= xPos + 1 && IsPath(xPos + 1, yPos))
		{
			if (GRID_HEIGHT - 1 >= yPos + 1 && g_tile[yPos + 1][xPos] == TILE_WALL && g_tile[yPos + 1][xPos + 1] != TILE_WALL)
			{
				bDownCorner = true;
			}
		}
		if (0 <= yPos - 1 && IsPath(xPos, yPos - 1))
		{
			if (0 <= xPos - 1 && g_tile[yPos][xPos - 1] == TILE_WALL && g_tile[yPos - 1][xPos - 1] != TILE_WALL)
			{
				bLeftCorner = true;
			}
		}
		if (bDownCorner || bLeftCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_RU);
			}
			return true;
		}

		// 수직 및 수평 탐색
		if (PathCheckUU(xPos, yPos, pParentTile, false) || PathCheckRR(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_RU);
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		++xPos;
		--yPos;
	}

	return false;
}
bool cJPS::PathCheckRR(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	bool bUpCorner = false;
	bool bDownCorner = false;

	int xPos = curXPos + 1;
	int yPos = curYPos;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (GRID_WIDTH - 1 >= xPos + 1 && IsPath(xPos + 1, yPos))
		{
			if (0 <= yPos - 1 && g_tile[yPos - 1][xPos] == TILE_WALL && g_tile[yPos - 1][xPos + 1] != TILE_WALL)
			{
				bUpCorner = true;
			}
			if (GRID_HEIGHT - 1 >= yPos + 1 && g_tile[yPos + 1][xPos] == TILE_WALL && g_tile[yPos + 1][xPos + 1] != TILE_WALL)
			{
				bDownCorner = true;
			}
		}
		if (bUpCorner || bDownCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_RR);
			}
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		++xPos;
	}

	return false;
}
bool cJPS::PathCheckRD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	if ((IsTile(curXPos + 1, curYPos) && !IsPath(curXPos + 1, curYPos)) &&
		(IsTile(curXPos, curYPos + 1) && !IsPath(curXPos, curYPos + 1)))  return false;

	bool bUpCorner = false;
	bool bLeftCorner = false;

	int xPos = curXPos + 1;
	int yPos = curYPos + 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if ((IsTile(xPos + 1, yPos) && !IsPath(xPos + 1, yPos)) &&
			(IsTile(xPos, yPos + 1) && !IsPath(xPos, yPos + 1))) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (GRID_WIDTH - 1 >= xPos + 1 && IsPath(xPos + 1, yPos))
		{
			if (0 <= yPos - 1 && g_tile[yPos - 1][xPos] == TILE_WALL && g_tile[yPos - 1][xPos + 1] != TILE_WALL)
			{
				bUpCorner = true;
			}
		}
		if (GRID_HEIGHT - 1 >= yPos + 1 && IsPath(xPos, yPos + 1))
		{
			if (0 <= xPos - 1 && g_tile[yPos][xPos - 1] == TILE_WALL && g_tile[yPos + 1][xPos - 1] != TILE_WALL)
			{
				bLeftCorner = true;
			}
		}
		if (bUpCorner || bLeftCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_RD);
			}
			return true;
		}

		// 수직 및 수평 탐색
		if (PathCheckRR(xPos, yPos, pParentTile, false) || PathCheckDD(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_RD);
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		++xPos;
		++yPos;
	}

	return false;
}
bool cJPS::PathCheckDD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	bool bLeftCorner = false;
	bool bRightCorner = false;

	int xPos = curXPos;
	int yPos = curYPos + 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (GRID_HEIGHT - 1 >= yPos + 1 && IsPath(xPos, yPos + 1))
		{
			if (0 <= xPos - 1 && g_tile[yPos][xPos - 1] == TILE_WALL && g_tile[yPos + 1][xPos - 1] != TILE_WALL)
			{
				bLeftCorner = true;
			}
			if (GRID_WIDTH - 1 >= xPos + 1 && g_tile[yPos][xPos + 1] == TILE_WALL && g_tile[yPos + 1][xPos + 1] != TILE_WALL)
			{
				bRightCorner = true;
			}
		}
		if (bLeftCorner || bRightCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_DD);
			}
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		++yPos;
	}

	return false;
}
bool cJPS::PathCheckLD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode)
{
	if ((IsTile(curXPos - 1, curYPos) && !IsPath(curXPos - 1, curYPos)) && 
		(IsTile(curXPos, curYPos + 1) && !IsPath(curXPos, curYPos + 1)))  return false;

	bool bUpCorner = false;
	bool bRightCorner = false;

	int xPos = curXPos - 1;
	int yPos = curYPos + 1;
	while (1)
	{
		if (!IsTile(xPos, yPos)) break;
		if (!IsPath(xPos, yPos)) break;

		if ((IsTile(xPos - 1, yPos) && !IsPath(xPos - 1, yPos)) &&
			(IsTile(xPos, yPos + 1) && !IsPath(xPos, yPos + 1))) break;

		if (g_destTile._xPos == xPos && g_destTile._yPos == yPos)
		{
			// 목적지이므로 방향성 NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// 코너 탐색(노드가 생성된 후 코너 이동 가능 여부 먼저 확인)
		if (0 <= xPos - 1 && IsPath(xPos - 1, yPos))
		{
			if (0 <= yPos - 1 && g_tile[yPos - 1][xPos] == TILE_WALL && g_tile[yPos - 1][xPos - 1] != TILE_WALL)
			{
				bUpCorner = true;
			}
		}
		if (GRID_HEIGHT - 1 >= yPos + 1 && IsPath(xPos, yPos + 1))
		{
			if (GRID_WIDTH - 1 >= xPos + 1 && g_tile[yPos][xPos + 1] == TILE_WALL && g_tile[yPos + 1][xPos + 1] != TILE_WALL)
			{
				bRightCorner = true;
			}
		}
		if (bUpCorner || bRightCorner)
		{
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_LD);
			}
			return true;
		}

		// 수직 및 수평 탐색
		if (PathCheckLL(xPos, yPos, pParentTile, false) || PathCheckDD(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_LD);
			return true;
		}

		// 코너를 찾지 못했으므로 좌표 이동
		--xPos;
		++yPos;
	}

	return false;
}