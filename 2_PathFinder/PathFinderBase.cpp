
#include "PathFinderBase.h"
extern char g_tile[][GRID_WIDTH];
extern stTileNode g_startTile;
extern stTileNode g_destTile;

stTileNode::stTileNode()
{
	_xPos = 0;
	_yPos = 0;

	_pParent = nullptr;

	_gValue = 0;
	_hValue = 0;
	_fValue = 0;

	_nodeDir = eNODE_DIRECTION::DIR_NONE;
}
stTileNode::stTileNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir)
{
	_xPos = xPos;
	_yPos = yPos;

	_pParent = pParentTile;

	_gValue = 0;
	_hValue = 0;
	_fValue = 0;

	_nodeDir = dir;
}
stTileNode::~stTileNode() {}

bool cPathFinderBase::IsTile(int xPos, int yPos) const
{
	if (xPos < 0 || xPos > GRID_WIDTH - 1 || yPos < 0 || yPos > GRID_HEIGHT - 1)
	{
		return false;
	}

	return true;
}
bool cPathFinderBase::IsPath(int xPos, int yPos) const
{
	if (g_tile[yPos][xPos] == TILE_WALL)
	{
		return false;
	}

	return true;
}
stTileNode* cPathFinderBase::GetTilePtrInfo(int xPos, int yPos) const
{
	// OpenList에 인자로 들어온 좌표값과 동일한 노드가 존재하는지 확인
	if (!_openList.empty())
	{
		list<stTileNode*>::const_iterator openIter = _openList.begin();
		for (openIter; openIter != _openList.end(); ++openIter)
		{
			if ((*openIter)->_xPos == xPos && (*openIter)->_yPos == yPos)
			{
				return *openIter;
			}
		}
	}

	// CloseList에 인자로 들어온 좌표값과 동일한 노드가 존재하는지 확인
	if (!_closeList.empty())
	{
		list<stTileNode*>::const_iterator closeIter = _closeList.begin();
		for (closeIter; closeIter != _closeList.end(); ++closeIter)
		{
			if ((*closeIter)->_xPos == xPos && (*closeIter)->_yPos == yPos)
			{
				return *closeIter;
			}
		}
	}
	return nullptr;
}

void cPathFinderBase::ClearPath()
{
	for (int iCntW = 0; iCntW < GRID_WIDTH; ++iCntW)
	{
		for (int iCntH = 0; iCntH < GRID_HEIGHT; ++iCntH)
		{
			if (g_tile[iCntH][iCntW] == TILE_START || g_tile[iCntH][iCntW] == TILE_DEST || g_tile[iCntH][iCntW] == TILE_WALL)
			{
				continue;
			}

			g_tile[iCntH][iCntW] = TILE_PATH;
		}
	}

	list<stTileNode*>::iterator openIter = _openList.begin();
	list<stTileNode*>::iterator closeIter = _closeList.begin();

	for (openIter; openIter != _openList.end(); ++openIter)
	{
		delete* openIter;
	}
	for (closeIter; closeIter != _closeList.end(); ++closeIter)
	{
		if (*closeIter == &g_startTile || *closeIter == &g_destTile)
		{
			continue;
		}

		delete* closeIter;
	}

	_openList.clear();
	_closeList.clear();
}
void cPathFinderBase::ClearAll()
{
	for (int iCntW = 0; iCntW < GRID_WIDTH; ++iCntW)
	{
		for (int iCntH = 0; iCntH < GRID_HEIGHT; ++iCntH)
		{
			g_tile[iCntH][iCntW] = TILE_PATH;
		}
	}

	//g_startPos, g_destPos, g_bSetStart, g_bSetDest, drawtile, drawline 같은 정보들은 메인에서 수정해야 함
	// 여기서는 노드들만 삭제함
	

	list<stTileNode*>::iterator openIter = _openList.begin();
	list<stTileNode*>::iterator closeIter = _closeList.begin();

	for (openIter; openIter != _openList.end(); ++openIter)
	{
		delete* openIter;
	}
	for (closeIter; closeIter != _closeList.end(); ++closeIter)
	{
		delete* closeIter;
	}

	_openList.clear();
	_closeList.clear();
}