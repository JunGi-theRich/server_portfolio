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

	double _gValue;		// 출발점부터 현재 위치까지의 이동 거리
	double _hValue;		// 현재 위치에서 목적지까지의 절대적 거리(장애물 무시)
	double _fValue;		// g + h 의 값(가중치는 #define을 통해 조절 가능)

	int _nodeDir;
};

class cPathFinderBase
{
public:
	virtual bool StartPathFind() = 0;

	// 출발지, 도착지, 벽은 그대로 두고, 길 찾기를 위해 생성된 노드들을 지움
	void ClearPath();

	// 모든 노드 정보를 삭제함
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

	// _openList와 _closeList를 순회하여 해당 좌표값을 갖는 노드가 존재한다면 노드 리턴, 없으면 nullptr 리턴
	stTileNode* GetTilePtrInfo(int xPos, int yPos) const;

protected:
	virtual stTileNode* MakeNode(int xPos, int yPos, stTileNode* pParentTile, eNODE_DIRECTION dir = DIR_NONE) = 0;

protected:
	list<stTileNode*> _openList;
	list<stTileNode*> _closeList;
};