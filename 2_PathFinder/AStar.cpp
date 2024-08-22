
#include "AStar.h"
extern char g_tile[][GRID_WIDTH];
extern stTileNode g_startTile;
extern stTileNode g_destTile;

bool cAstar::StartPathFind()
{
	// A*는 현재 노드 기준으로 주변 8방향에 노드 생성을 시도함
	// 1. 직선 방향은 생성 가능 여부만 확인 (벽이 존재하지 않고, 맵에 끝이 아닌 경우 이동 가능 영역)
	// 2. 대각선 방향은 수직 및 수평 방향 모두 벽이 있는 경우도 추가로 확인함 (둘 중 한 곳이라도 벽이 아니라면 이동 가능 영역)
	// 3. 생성하려는 노드가 리스트에 없는 새로운 노드일 경우 생성하고 리스트에 넣어줌
	// 4. 생성하려는 노드가 이미 존재하고, 동시에 새로운 노드의 G값이 기존의 G값보다 작다면 G값을 변경해줌

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
	_openList.push_back(pCurNode);

	while (1)
	{
		list<stTileNode*>::iterator findIter = _openList.begin();
		list<stTileNode*>::iterator eraseIter = findIter;
		if (_openList.empty())
		{
			return false;
		}

		double fVal = (*findIter)->_fValue;

		// _openList 전체를 순회하며 도착값과 동일한 좌표를 갖는 노드가 있다면 반복문 종료
		// 없다면 전체 노드 중에서 가장 작은 f값을 갖는 노드를 추출하여 해당 좌표 주변에 노드 생성
		for (findIter; findIter != _openList.end(); ++findIter)
		{
			if ((*findIter)->_xPos == xEnd && (*findIter)->_yPos == yEnd)
			{
				// 전역 도착점 노드의 부모 포인터 갱신 및 타일 속성 유지를 위해 변경 후 함수 종료

				pCurNode = *findIter;
				g_destTile._gValue = GetGvalue(pCurNode->_pParent, pCurNode);
				g_destTile._hValue = 0;
				g_destTile._fValue = GetFvalue(&g_destTile);
				g_destTile._pParent = pCurNode->_pParent;

				g_tile[yEnd][xEnd] = TILE_DEST;
				return true;
			}

			if (fVal > (*findIter)->_fValue)
			{
				// 더 작은 f값이 있으므로 갱신
				fVal = (*findIter)->_fValue;
				eraseIter = findIter;
			}
		}

		// 여기까지 왔다면 도착 노드 정보가 없는 상태이므로 pCurNode를 갱신하여 주변 노드 생성
		pCurNode = *eraseIter;

		// LL 타일 확인
		if (IsTile(pCurNode->_xPos - 1, pCurNode->_yPos) && IsPath(pCurNode->_xPos - 1, pCurNode->_yPos))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// LU 타일 확인
		if (IsTile(pCurNode->_xPos - 1, pCurNode->_yPos - 1) && IsPath(pCurNode->_xPos - 1, pCurNode->_yPos - 1) &&
			(IsPath(pCurNode->_xPos - 1, pCurNode->_yPos) || IsPath(pCurNode->_xPos, pCurNode->_yPos - 1)))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// UU 타일 확인
		if (IsTile(pCurNode->_xPos, pCurNode->_yPos - 1) && IsPath(pCurNode->_xPos, pCurNode->_yPos - 1))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos, pCurNode->_yPos - 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// RU 타일 확인
		if (IsTile(pCurNode->_xPos + 1, pCurNode->_yPos - 1) && IsPath(pCurNode->_xPos + 1, pCurNode->_yPos - 1)
			&& (IsPath(pCurNode->_xPos + 1, pCurNode->_yPos) || IsPath(pCurNode->_xPos, pCurNode->_yPos - 1)))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// RR 타일 확인
		if (IsTile(pCurNode->_xPos + 1, pCurNode->_yPos) && IsPath(pCurNode->_xPos + 1, pCurNode->_yPos))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// RD 타일 확인
		if (IsTile(pCurNode->_xPos + 1, pCurNode->_yPos + 1) && IsPath(pCurNode->_xPos + 1, pCurNode->_yPos + 1)
			&& (IsPath(pCurNode->_xPos + 1, pCurNode->_yPos) || IsPath(pCurNode->_xPos, pCurNode->_yPos + 1)))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// DD 타일 확인
		if (IsTile(pCurNode->_xPos, pCurNode->_yPos + 1) && IsPath(pCurNode->_xPos, pCurNode->_yPos + 1))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos, pCurNode->_yPos + 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// LD 타일 확인
		if (IsTile(pCurNode->_xPos - 1, pCurNode->_yPos + 1) && IsPath(pCurNode->_xPos - 1, pCurNode->_yPos + 1)
			&& (IsPath(pCurNode->_xPos - 1, pCurNode->_yPos) || IsPath(pCurNode->_xPos, pCurNode->_yPos + 1)))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}
		_closeList.push_back(pCurNode);
		_openList.erase(eraseIter);
	}
}

stTileNode* cAstar::MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir)										
{
	stTileNode* pNewTile = GetTilePtrInfo(xPos, yPos);
	if (nullptr == pNewTile)
	{
		pNewTile = new stTileNode(xPos, yPos, pParentTile);
		pNewTile->_gValue = GetGvalue(pParentTile, pNewTile);
		pNewTile->_hValue = GetHvalue(&g_destTile, pNewTile);
		pNewTile->_fValue = GetFvalue(pNewTile);

		g_tile[pNewTile->_yPos][pNewTile->_xPos] = TILE_NODE_NORMAL;
		_openList.push_back(pNewTile);
	}
	return pNewTile;
}