
#include "JPS.h"
extern char g_tile[][GRID_WIDTH];
extern stTileNode g_startTile;
extern stTileNode g_destTile;

bool cJPS::StartPathFind()
{
	// ��� Ž�� �⺻ ����
	// 1. ��� ������ ������ ��� ������ ������ ���⼺(unique direction)�� ������
	// 2. �� ������ �θ� ��忡 ���� Ž���� ����� ������ ������ ��θ� Ž����
	// 3. ��� Ž�� �� �ڳ�(���� �̿�)�� ������ �� ���, �ش� ������ �θ� ��忡 ���� Ž������ �ʾҴٰ� ��
	// 
	// 4. '���� ���⼺'�� ���� ����� Ž��
	// 4-1. ���� ���� ���� ���� ��ǥ(���� ���, ���� Ž���� ��� ���� ��ġ���� '���� �Ʒ�')�� ���� �ִ� ���,
	// �׸��� ���ÿ� �밢�� �������� �̵��� ������ ��쿡 �ڳʷ� �Ǵ���
	// 4-2. �ڳʸ� �Ǵ��� ������ ��带 ������
	// �� ����� �θ� ���� Ž���� ������ ����̸�, ���⼺ ���� �״�� ��������
	// 4-3. �� ������ �ڳʸ� �߰��ϰų� �̵��� �Ұ����� ������ �ڽ��� �������� �� ĭ�� �̵��ϸ� ����
	// 
	// 5. '�밢�� ���⼺'�� ���� ����� Ž��
	// 5-1. ���� ���� ���� 135�� ��ǥ(���� ���, ���� Ž���� ��� ���� ��ġ���� '���ʰ� �Ʒ�')�� ���� �ִ� ���,
	// �׸��� ���ÿ� ���� �������� �̵��� ������ ��� �ڳʷ� �Ǵ���
	// 5-2. �ڳʸ� ã�� ���ߴٸ� ���� �� ���� ������ Ž���� ������(���� ��� ���� Ž���� ��� ���� ��ġ���� '�����ʰ� ��')
	// Ž�� �������� �ڳʸ� �߰��Ͽ��ٸ� ���� �� ���� ���� Ž���� ������ ������ ��带 ������
	// 5-3. �� ������ �ڳʸ� �߰��ϰų� �̵��� �Ұ����� ������ �ڽ��� �������� �� ĭ�� �̵��ϸ� ����

	stTileNode* pCurNode;
	int xStart = g_startTile._xPos;
	int yStart = g_startTile._yPos;
	int xEnd = g_destTile._xPos;
	int yEnd = g_destTile._yPos;

	// gValue�� ��Ŭ����, hValue�� ����ư ������� ����
	g_startTile._gValue = 0;
	g_startTile._hValue = (abs(xEnd - xStart) + (abs(yEnd - yStart)));
	g_startTile._fValue = g_startTile._gValue * G_WEIGHT + g_startTile._hValue * H_WEIGHT;

	pCurNode = &g_startTile;
	_closeList.push_back(pCurNode);


	// ���� �������� 8���� ��� Ž��
	PathCheckLL(xStart, yStart, pCurNode);
	PathCheckUU(xStart, yStart, pCurNode);
	PathCheckRR(xStart, yStart, pCurNode);
	PathCheckDD(xStart, yStart, pCurNode);
	PathCheckLU(xStart, yStart, pCurNode);
	PathCheckRU(xStart, yStart, pCurNode);
	PathCheckRD(xStart, yStart, pCurNode);
	PathCheckLD(xStart, yStart, pCurNode);

	// _openList ��ȸ�ϸ� ���� ���� f���� ���� ���� ã��, �� ����� ���⼺�� �������� ��� Ž��
	while (1)
	{
		list<stTileNode*>::iterator openIter = _openList.begin();
		list<stTileNode*>::iterator eraseIter = openIter;
		if (_openList.empty())
		{
			// ��� ���� ������ �޽���
			return false;
		}

		double fVal = (*openIter)->_fValue;

		for (openIter; openIter != _openList.end(); ++openIter)
		{
			if ((*openIter)->_xPos == xEnd && (*openIter)->_yPos == yEnd)
			{
				// ���� ������ ����� �θ� ������ ���� �� Ÿ�� �Ӽ� ������ ���� ���� �� �Լ� ����

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
				// �� ���� f���� �����Ƿ� ����
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
			DebugBreak();		// �̰��� ���� �ȵ�
			break;

		case DIR_LL:
			PathCheckLL(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// �Ʒ��� �� üũ
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

			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1 		// �Ʒ��� �� üũ
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1		// ������ �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			break;

		case DIR_UU:
			PathCheckUU(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1		// ������ �� üũ
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

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// �Ʒ��� �� üũ
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] == TILE_WALL
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1								// ���� �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			break;

		case DIR_RR:
			PathCheckRR(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// �Ʒ��� �� üũ
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

			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && 0 <= pCurNode->_yPos - 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos - 1, pCurNode, DIR_RU);
			}
			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			break;

		case DIR_DD:
			PathCheckDD(pCurNode->_xPos, pCurNode->_yPos, pCurNode);

			if (0 <= pCurNode->_xPos - 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1					// ���� �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos + 1, pCurNode, DIR_LD);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// ������ �� üũ
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

			if (0 <= pCurNode->_xPos - 1 && 0 <= pCurNode->_yPos - 1								// ���� �� üũ
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos] == TILE_WALL 
				&& g_tile[pCurNode->_yPos - 1][pCurNode->_xPos - 1] != TILE_WALL && g_tile[pCurNode->_yPos][pCurNode->_xPos - 1] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos - 1, pCurNode, DIR_LU);
			}
			if (GRID_WIDTH - 1 >= pCurNode->_xPos + 1 && GRID_HEIGHT - 1 >= pCurNode->_yPos + 1		// ������ �� üũ
				&& g_tile[pCurNode->_yPos][pCurNode->_xPos + 1] == TILE_WALL 
				&& g_tile[pCurNode->_yPos + 1][pCurNode->_xPos + 1] != TILE_WALL && g_tile[pCurNode->_yPos + 1][pCurNode->_xPos] != TILE_WALL)
			{
				MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos + 1, pCurNode, DIR_RD);
			}
			break;

		default:
			DebugBreak();		// �̰��� ���� �ȵ�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// ���� �� ���� Ž��
		if (PathCheckLL(xPos, yPos, pParentTile, false) || PathCheckUU(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_LU);
			return true;
		}

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// ���� �� ���� Ž��
		if (PathCheckUU(xPos, yPos, pParentTile, false) || PathCheckRR(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_RU);
			return true;
		}

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// ���� �� ���� Ž��
		if (PathCheckRR(xPos, yPos, pParentTile, false) || PathCheckDD(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_RD);
			return true;
		}

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
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
			// �������̹Ƿ� ���⼺ NONE
			if (bMakeNode)
			{
				MakeNode(xPos, yPos, pParentTile, DIR_NONE);
			}
			return true;
		}

		// �ڳ� Ž��(��尡 ������ �� �ڳ� �̵� ���� ���� ���� Ȯ��)
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

		// ���� �� ���� Ž��
		if (PathCheckLL(xPos, yPos, pParentTile, false) || PathCheckDD(xPos, yPos, pParentTile, false))
		{
			MakeNode(xPos, yPos, pParentTile, DIR_LD);
			return true;
		}

		// �ڳʸ� ã�� �������Ƿ� ��ǥ �̵�
		--xPos;
		++yPos;
	}

	return false;
}