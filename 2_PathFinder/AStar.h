#pragma once

#include "define.h"
#include "PathFinderBase.h"

class cAstar : public cPathFinderBase
{
public:

public:
	virtual bool StartPathFind();

protected:
	// 타일 포인터를 리턴, 타일이 없다면 생성 후에 포인터 리턴
	stTileNode* MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir = DIR_NONE);
};