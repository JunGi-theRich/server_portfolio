#pragma once

#include <Windows.h>
#include <list>
using std::list;

#include "define.h"

enum eNODE_DIRECTION
{
	DIR_NONE = 4,
	DIR_LL,
	DIR_LU,
	DIR_UU,
	DIR_RU,
	DIR_RR,
	DIR_RD,
	DIR_DD,
	DIR_LD,
};

struct stTileNode
{
	stTileNode();
	stTileNode(int xPos, int yPos, stTileNode* pParentNode, eNODE_DIRECTION dir = DIR_NONE);
	~stTileNode();

	int _xPos;
	int _yPos;

	stTileNode* _pParent;

	double _gValue;		// ��������� ���� ��ġ������ �̵� �Ÿ�
	double _hValue;		// ���� ��ġ���� ������������ ������ �Ÿ�(��ֹ� ����)
	double _fValue;		// g + h �� ��(����ġ�� #define�� ���� ���� ����)

	int _nodeDir;
};

class cPathFinderBase
{
public:
	virtual bool StartPathFind() = 0;

	// �����, ������, ���� �״�� �ΰ�, �� ã�⸦ ���� ������ ������ ����
	void ClearPath();

	// ��� ��� ������ ������
	void ClearAll();

protected:
	inline double GetGvalue(const stTileNode* pParentTile, const stTileNode* pCurTile)
	{
		return pParentTile->_gValue +
			sqrt(abs(pCurTile->_xPos - pParentTile->_xPos) * abs(pCurTile->_xPos - pParentTile->_xPos) +
				abs(pCurTile->_yPos - pParentTile->_yPos) * abs(pCurTile->_yPos - pParentTile->_yPos));
	}
	inline double GetHvalue(const stTileNode* pDestTile, const stTileNode* pCurTile)
	{
		return abs(pCurTile->_xPos - pDestTile->_xPos) + abs(pCurTile->_yPos - pDestTile->_yPos);
	}
	inline double GetFvalue(const stTileNode* pCurTile) { return pCurTile->_gValue * G_WEIGHT + pCurTile->_hValue * H_WEIGHT; }

	bool IsTile(int xPos, int yPos)const;
	bool IsPath(int xPos, int yPos)const;

	// _openList�� _closeList�� ��ȸ�Ͽ� �ش� ��ǥ���� ���� ��尡 �����Ѵٸ� ��� ����, ������ nullptr ����
	stTileNode* GetTilePtrInfo(int xPos, int yPos) const;

protected:
	virtual stTileNode* MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir = DIR_NONE) = 0;

protected:
	list<stTileNode*> _openList;
	list<stTileNode*> _closeList;
};