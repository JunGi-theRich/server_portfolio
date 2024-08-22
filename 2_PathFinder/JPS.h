#pragma once

#include "define.h"
#include "PathFinderBase.h"

class cJPS : public cPathFinderBase
{
public:

public:
	virtual bool StartPathFind();

protected:
	stTileNode* MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir = DIR_NONE);

	bool PathCheckLL(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckLU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckUU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckRU(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckRR(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckRD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckDD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
	bool PathCheckLD(int curXPos, int curYPos, stTileNode* pParentTile, bool bMakeNode = true);
};