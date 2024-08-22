#pragma once

#include "define.h"
#include "PathFinderBase.h"

class cAstar : public cPathFinderBase
{
public:

public:
	virtual bool StartPathFind();

protected:
	// Ÿ�� �����͸� ����, Ÿ���� ���ٸ� ���� �Ŀ� ������ ����
	stTileNode* MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir = DIR_NONE);
};