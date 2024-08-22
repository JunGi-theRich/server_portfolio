
#include "AStar.h"
extern char g_tile[][GRID_WIDTH];
extern stTileNode g_startTile;
extern stTileNode g_destTile;

bool cAstar::StartPathFind()
{
	// A*�� ���� ��� �������� �ֺ� 8���⿡ ��� ������ �õ���
	// 1. ���� ������ ���� ���� ���θ� Ȯ�� (���� �������� �ʰ�, �ʿ� ���� �ƴ� ��� �̵� ���� ����)
	// 2. �밢�� ������ ���� �� ���� ���� ��� ���� �ִ� ��쵵 �߰��� Ȯ���� (�� �� �� ���̶� ���� �ƴ϶�� �̵� ���� ����)
	// 3. �����Ϸ��� ��尡 ����Ʈ�� ���� ���ο� ����� ��� �����ϰ� ����Ʈ�� �־���
	// 4. �����Ϸ��� ��尡 �̹� �����ϰ�, ���ÿ� ���ο� ����� G���� ������ G������ �۴ٸ� G���� ��������

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

		// _openList ��ü�� ��ȸ�ϸ� �������� ������ ��ǥ�� ���� ��尡 �ִٸ� �ݺ��� ����
		// ���ٸ� ��ü ��� �߿��� ���� ���� f���� ���� ��带 �����Ͽ� �ش� ��ǥ �ֺ��� ��� ����
		for (findIter; findIter != _openList.end(); ++findIter)
		{
			if ((*findIter)->_xPos == xEnd && (*findIter)->_yPos == yEnd)
			{
				// ���� ������ ����� �θ� ������ ���� �� Ÿ�� �Ӽ� ������ ���� ���� �� �Լ� ����

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
				// �� ���� f���� �����Ƿ� ����
				fVal = (*findIter)->_fValue;
				eraseIter = findIter;
			}
		}

		// ������� �Դٸ� ���� ��� ������ ���� �����̹Ƿ� pCurNode�� �����Ͽ� �ֺ� ��� ����
		pCurNode = *eraseIter;

		// LL Ÿ�� Ȯ��
		if (IsTile(pCurNode->_xPos - 1, pCurNode->_yPos) && IsPath(pCurNode->_xPos - 1, pCurNode->_yPos))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos - 1, pCurNode->_yPos, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// LU Ÿ�� Ȯ��
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

		// UU Ÿ�� Ȯ��
		if (IsTile(pCurNode->_xPos, pCurNode->_yPos - 1) && IsPath(pCurNode->_xPos, pCurNode->_yPos - 1))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos, pCurNode->_yPos - 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// RU Ÿ�� Ȯ��
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

		// RR Ÿ�� Ȯ��
		if (IsTile(pCurNode->_xPos + 1, pCurNode->_yPos) && IsPath(pCurNode->_xPos + 1, pCurNode->_yPos))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos + 1, pCurNode->_yPos, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// RD Ÿ�� Ȯ��
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

		// DD Ÿ�� Ȯ��
		if (IsTile(pCurNode->_xPos, pCurNode->_yPos + 1) && IsPath(pCurNode->_xPos, pCurNode->_yPos + 1))
		{
			stTileNode* pTile = MakeNode(pCurNode->_xPos, pCurNode->_yPos + 1, pCurNode);
			if (pTile->_gValue > GetGvalue(pCurNode, pTile))
			{
				pTile->_gValue = GetGvalue(pCurNode, pTile);
				pTile->_pParent = pCurNode;
			}
		}

		// LD Ÿ�� Ȯ��
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