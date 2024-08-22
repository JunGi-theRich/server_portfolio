
#pragma once

#include <type_traits>

template <typename DATA>
class cRBT
{
private:
	struct stNode;
public:
	class iterator;

public:
	cRBT()
	{
		_size = 0;
		_pRoot = nullptr;

		//_Nil = new stNode;
	}
	~cRBT()
	{
		delete _Nil;
	}

	bool insert(const DATA& val);

	iterator find(const DATA& val);
	iterator erase(iterator whereIter);

	iterator begin()
	{
		stNode* pNode = _pRoot;
		if (nullptr == _pRoot)
		{
			return _Nil;
		}

		while (_Nil != pNode->_pLeft)
		{
			pNode = pNode->_pLeft;
		}
		return pNode;
	}
	iterator end()
	{
		return _Nil;
	}

	int size() { return _size; }
	bool empty()
	{
		if (0 == _size)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void clear()
	{
		while (!empty())
		{
			iterator tmp = begin();
			erase(tmp);
		}
	}

	//==========
public:
	// 트리 검증용 함수로 정수 자료형에서만 동작함
	bool VerifyTree(int putSize);
private:
	bool Verify_Insequence();
	bool Verify_CountBlack(stNode* pNode, int expectedBlackCount, int* pBlackCount);
	//==========

private:
	stNode* InsertProc(const DATA& val);
	void BalanceProc_Insert(stNode* pTarget);

	stNode* DeleteProc(stNode* pTarget, bool* wasBlack);
	void BalanceProc_Delete(stNode* pTarget, bool wasBlack);

	void RotateLeft(stNode* pAxis);
	void RotateRight(stNode* pAxis);

private:
	int _size;
	stNode* _pRoot;

	static stNode* _Nil;

private:
	struct stNode
	{
		friend cRBT;
		stNode()
		{
			_pParent = nullptr;
			_black = true;
		}
		stNode(const DATA& val, stNode* pParent, bool isBlack)
		{
			_data = val;
			_pParent = pParent;
			_black = isBlack;
		}

		bool IsRoot()
		{
			if (nullptr == _pParent)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool IsLeft()
		{
			if (this == _pParent->_pLeft)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool IsRight()
		{
			if (this == _pParent->_pRight)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		DATA _data;
		bool _black;

		stNode* _pParent;
		stNode* _pLeft;
		stNode* _pRight;
	};

public:
	class iterator
	{
	public:
		friend cRBT;

		iterator(stNode* pNode = nullptr)
		{
			_pNode = pNode;
		}

		iterator& operator++();
		const iterator operator++(int);

		iterator& operator--();
		const iterator operator--(int);

		DATA& operator*()
		{
			return _pNode->_data;
		}

		bool operator==(const iterator& other)
		{
			if (_pNode == other._pNode)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator!=(const iterator& other)
		{
			return !(*this == other);
		}

	private:
		stNode* _pNode;
	};
};


// ==========================================================================================
// ==========================================================================================
// ==========================================================================================
// ==========================================================================================

template <typename DATA>
typename cRBT<DATA>::stNode* cRBT<DATA>::_Nil = new stNode;

template <typename DATA>
bool cRBT<DATA>::insert(const DATA& val)
{
	stNode* pNewNode = InsertProc(val);
	if (nullptr != pNewNode)
	{
		BalanceProc_Insert(pNewNode);

		// 모든 작업을 마친 후에 _pRoot가 false(RED)라면 true(BLACK)로 바꿈
		if (!_pRoot->_black)
		{
			_pRoot->_black = true;
		}
		return true;
	}
	else
	{
		return false;
	}
}

template <typename DATA>
typename cRBT<DATA>::iterator cRBT<DATA>::find(const DATA& val)
{
	stNode* pFindNode = _pRoot;
	while (1)
	{
		if (_Nil == pFindNode)
		{
			return nullptr;
		}

		if (val < pFindNode->_data)
		{
			pFindNode = pFindNode->_pLeft;
		}
		else if (val > pFindNode->_data)
		{
			pFindNode = pFindNode->_pRight;
		}
		else
		{
			return pFindNode;
		}
	}
}

template <typename DATA>
typename cRBT<DATA>::iterator cRBT<DATA>::erase(iterator whereIter)
{
	stNode* pDeleteNode = whereIter._pNode;
	bool bWasBlack;
	if (_Nil == pDeleteNode)
	{
		return nullptr;
	}
	++whereIter;
	pDeleteNode = DeleteProc(pDeleteNode, &bWasBlack);			// 삭제된 노드 색상 확인 및 자식 노드를 리턴(Nil 또는 자식 노드)
	BalanceProc_Delete(pDeleteNode, bWasBlack);

	return whereIter;
}

template <typename DATA>
typename cRBT<DATA>::stNode* cRBT<DATA>::InsertProc(const DATA& val)
{
	if (nullptr == _pRoot)
	{
		stNode* pNewNode = new stNode(val, nullptr, true);
		pNewNode->_pLeft = _Nil;
		pNewNode->_pRight = _Nil;
		++_size;
		_pRoot = pNewNode;
		return pNewNode;
	}
	else
	{
		stNode* pParent = _pRoot;
		while (1)
		{
			if (val < pParent->_data)
			{
				if (_Nil == pParent->_pLeft)
				{
					stNode* pNewNode = new stNode(val, pParent, false);
					pNewNode->_pLeft = _Nil;
					pNewNode->_pRight = _Nil;
					++_size;
					pParent->_pLeft = pNewNode;
					return pNewNode;
				}
				pParent = pParent->_pLeft;
			}
			else if (val > pParent->_data)
			{
				if (_Nil == pParent->_pRight)
				{
					stNode* pNewNode = new stNode(val, pParent, false);
					pNewNode->_pLeft = _Nil;
					pNewNode->_pRight = _Nil;
					++_size;
					pParent->_pRight = pNewNode;
					return pNewNode;
				}
				pParent = pParent->_pRight;
			}
			else
			{
				// 중복값 허용하지 않음
				return nullptr;
			}
		}
	}
}

template <typename DATA>
void cRBT<DATA>::BalanceProc_Insert(stNode* pTarget)
{
	// 함수 진행 조건1 - 부모 및 조부모 노드의 존재
	if (nullptr == pTarget->_pParent || nullptr == pTarget->_pParent->_pParent)
	{
		return;
	}

	// 함수 진행 조건2 - 부모 노드가 false(RED)
	if (pTarget->_pParent->_black)
	{
		return;
	}

	stNode* pParent = pTarget->_pParent;
	stNode* pGrandParent = pParent->_pParent;

	// 조부모 노드 왼쪽에 부모 노드가 있는 경우
	if (pParent->IsLeft())
	{
		// 1. 삼촌 노드가 false(RED)
		if (!pGrandParent->_pRight->_black)
		{
			// 1-(1). 조부모 노드 false(RED)로 변경
			// 1-(2). 부모와 삼촌 노드를 true(BLACK)로 변경
			// 1-(3). 이 작업을 상위 노드로 올라가며 반복

			pGrandParent->_black = false;
			pParent->_black = true;
			pGrandParent->_pRight->_black = true;
			BalanceProc_Insert(pGrandParent);
		}

		// 2. 삼촌 노드가 true(BLACK)이고, pTarget이 부모 노드의 왼쪽에 있는 경우
		else if(pTarget->IsLeft())
		{
			// 2-(1). 부모 노드를 true(RED)로 변경
			// 2-(2). 조부모 노드를 false(RED)로 변경
			// 2-(3). 조부모 노드를 기준으로 우회전

			pParent->_black = true;
			pGrandParent->_black = false;
			RotateRight(pGrandParent);
		}

		// 3. 삼촌 노드가 true(BLACK)이고, pTarget이 부모 노드의 오른쪽에 있는 경우
		else
		{
			// 3-(1). 부모 노드를 기준으로 좌회전하여 한쪽으로 노드를 몰아줌
			// 3-(2). 신규 노드의 색상을 true(BLACK)으로 변경
			// 3-(3). 신규 노드의 부모 노드(구 조부모 노드)를 false(RED)로 변경
			// 3-(4). 신규 노드의 부모 노드(구 조부모 노드)를 기준으로 우회전

			RotateLeft(pParent);
			pTarget->_black = true;
			pGrandParent->_black = false;
			RotateRight(pGrandParent);
		}
	}

	// 조부모 노드 오른쪽에 부모 노드가 있는 경우 위 경우와 반대로 진행함
	else
	{
		if (!pGrandParent->_pLeft->_black)
		{
			pGrandParent->_black = false;
			pParent->_black = true;
			pGrandParent->_pLeft->_black = true;
			BalanceProc_Insert(pGrandParent);
		}

		else if (pTarget->IsLeft())
		{
			RotateRight(pParent);
			pTarget->_black = true;
			pGrandParent->_black = false;
			RotateLeft(pGrandParent);
		}

		else
		{
			pParent->_black = true;
			pGrandParent->_black = false;
			RotateLeft(pGrandParent);
		}
	}
}

template <typename DATA>
typename cRBT<DATA>::stNode* cRBT<DATA>::DeleteProc(stNode* pTarget, bool* wasBlack)
{
	stNode* pRetNode;
	// pTarget이 자식 노드가 없는 경우
	if (_Nil == pTarget->_pLeft && _Nil == pTarget->_pRight)
	{
		if (_pRoot == pTarget)
		{
			*wasBlack = _pRoot->_black;
			_pRoot = nullptr;
			delete pTarget;
			pTarget = nullptr;
			pRetNode = _Nil;
		}
		else
		{
			if (pTarget->IsLeft())
			{
				pTarget->_pParent->_pLeft = _Nil;
				_Nil->_pParent = pTarget->_pParent;
			}
			else
			{
				pTarget->_pParent->_pRight = _Nil;
				_Nil->_pParent = pTarget->_pParent;
			}

			*wasBlack = pTarget->_black;
			delete pTarget;
			pTarget = nullptr;
			pRetNode = _Nil;
		}
	}

	// pTarget이 두 개의 자식을 갖는 경우
	else if (_Nil != pTarget->_pLeft && _Nil != pTarget->_pRight)
	{
		// 삭제하려는 노드의 왼쪽 자식 노드들 중에서 가장 큰 값으로 교체하고 해당 노드를 대신 삭제
		stNode* pExchange = pTarget->_pLeft;
		while (1)
		{
			if (_Nil == pExchange->_pRight)
			{
				break;
			}
			pExchange = pExchange->_pRight;
		}

		pTarget->_data = pExchange->_data;
		pTarget = pExchange;

		if (_Nil != pTarget->_pLeft)
		{
			if (pTarget->IsLeft())
			{
				pTarget->_pParent->_pLeft = pTarget->_pLeft;
			}
			else
			{
				pTarget->_pParent->_pRight = pTarget->_pLeft;
			}

			pTarget->_pLeft->_pParent = pTarget->_pParent;
			pRetNode = pTarget->_pLeft;
			*wasBlack = pTarget->_black;
			delete pTarget;
			pTarget = nullptr;
		}
		else
		{
			if (pTarget->IsLeft())
			{
				pTarget->_pParent->_pLeft = _Nil;
			}
			else
			{
				pTarget->_pParent->_pRight = _Nil;
			}

			_Nil->_pParent = pTarget->_pParent;
			pRetNode = pTarget->_pRight;		// Nil
			*wasBlack = pTarget->_black;
			delete pTarget;
			pTarget = nullptr;
		};
	}

	// pTarget이 자식 노드를 하나 갖는 경우
	else
	{
		if (_pRoot == pTarget)
		{
			if (_Nil != pTarget->_pLeft)
			{
				pRetNode = pTarget->_pLeft;
				pRetNode->_pParent = nullptr;
			}
			else
			{
				pRetNode = pTarget->_pRight;
				pRetNode->_pParent = nullptr;
			}
			_pRoot = pRetNode;
			pRetNode->_black = true;
			*wasBlack = pTarget->_black;
			delete pTarget;
			pTarget = nullptr;
		}
		else
		{
			if (_Nil != pTarget->_pLeft)
			{
				pRetNode = pTarget->_pLeft;

				if (pTarget->IsLeft())
				{
					pTarget->_pParent->_pLeft = pTarget->_pLeft;
					pTarget->_pLeft->_pParent = pTarget->_pParent;
				}
				else
				{
					pTarget->_pParent->_pRight = pTarget->_pLeft;
					pTarget->_pLeft->_pParent = pTarget->_pParent;
				}
			}
			else
			{
				pRetNode = pTarget->_pRight;

				if (pTarget->IsLeft())
				{
					pTarget->_pParent->_pLeft = pTarget->_pRight;
					pTarget->_pRight->_pParent = pTarget->_pParent;
				}
				else
				{
					pTarget->_pParent->_pRight = pTarget->_pRight;
					pTarget->_pRight->_pParent = pTarget->_pParent;
				}
			}

			*wasBlack = pTarget->_black;
			delete pTarget;
			pTarget = nullptr;
		}
	}

	--_size;
	return pRetNode;
}

template <typename DATA>
void cRBT<DATA>::BalanceProc_Delete(stNode* pTarget, bool wasBlack)
{
	// 아래 상황의 경우 이 함수는 진행될 수 없음
	// 1. 삭제된 노드가 마지막으로 남은 노드였던 경우
	// 2. 삭제된 노드의 색상이 false(RED)였던 경우
	// 3. 재귀로 인한 호출에서 pTarget이 루트 노드가 된 경우
	if (nullptr == _pRoot || false == wasBlack || _pRoot == pTarget)
	{
		return;
	}

	// 가장 큰 분기는 pTarget이 부모 노드의 왼쪽 또는 오른쪽 노드인지 판단

	if (pTarget->IsLeft())
	{
		stNode* pSibling = pTarget->_pParent->_pRight;

		// 1. pTarget이 false(RED) 노드인 경우
		if (false == pTarget->_black)
		{
			// 1-(1). pTarget 노드를 true(BLACK)으로 변경

			pTarget->_black = true;
			return;	
		}

		// 2. pSibling이 false(RED)인 경우
		if (false == pSibling->_black)
		{
			// 2-(1). 형제 노드를 true(BLACK)로 변경
			// 2-(2). 부모 노도를 false(RED)로 변경
			// 2-(3). 부모 노드를 기준으로 좌회전
			// 2-(4). pTarget 노드를 기준으로 밸런스 작업 재진행

			pSibling->_black = true;
			pTarget->_pParent->_black = false;
			RotateLeft(pTarget->_pParent);
			BalanceProc_Delete(pTarget, wasBlack);
			return;
		}

		// 3. pSibling이 true(BLACK)이고, 양쪽 자식이 true(BLACK)
		if (true == pSibling->_pLeft->_black && true == pSibling->_pRight->_black)
		{
			// 3-(1). 형제 노드를 false(RED)로 변경
			// 3-(2). 이로 인해 형제가 밸런스가 맞았으면, 부모 노드를 기준으로 밸런스 작업 재진행

			pSibling->_black = false;
			BalanceProc_Delete(pTarget->_pParent, wasBlack);
			return;
		}

		// 4. pSibling이 true(BLACK)이고, 오른쪽 자식이 true(BLACK)
		if (true == pSibling->_pRight->_black)
		{
			// 4-(1). 형제 노드를 false(RED)로 변경
			// 4-(2). 형제의 왼쪽 자식을 true(BLACK)로 변경
			// 4-(3). 형제 노드를 기준으로 우회전
			// 4-(4). 5번 케이스와 동일한 상황이 되며, 밸런스 작업 재진행

			pSibling->_black = false;
			pSibling->_pLeft->_black = true;
			RotateRight(pSibling);
			BalanceProc_Delete(pTarget, wasBlack);
			//!!return;
		}

		// 5. pSibling이 true(BLACK)이고, 오른쪽 자식이 false(RED)
		if (false == pSibling->_pRight->_black)
		{
			// 5-(1). 형제 노드의 색상을 부모의 색상으로 변경
			// 5-(2). 형제 노드의 자식 노드들 색상을 true(BLACK)로 변경
			// 5-(3). 부모 노드의 색상을 true(BLACK)로 변경
			// 5-(4). 부모 노드를 기준으로 좌회전하여 밸런스를 맞춤

			pSibling->_black = pSibling->_pParent->_black;
			pTarget->_pParent->_black = true;
			pSibling->_pRight->_black = true;
			RotateLeft(pTarget->_pParent);
			//!!return;
		}
	}
	else
	{
		// 모든 로직은 위 로직과 대칭으로 진행됨

		stNode* pSibling = pTarget->_pParent->_pLeft;

		if (false == pTarget->_black)
		{
			pTarget->_black = true;
			return;
		}

		if (false == pSibling->_black)
		{
			pSibling->_black = true;
			pSibling->_pParent->_black = false;
			RotateRight(pTarget->_pParent);
			BalanceProc_Delete(pTarget, wasBlack);
			return;
		}

		if (true == pSibling->_pLeft->_black && true == pSibling->_pRight->_black)
		{
			pSibling->_black = false;
			BalanceProc_Delete(pTarget->_pParent, wasBlack);
			return;
		}

		if (true == pSibling->_pLeft->_black)
		{
			pSibling->_black = false;
			pSibling->_pRight->_black = true;
			RotateLeft(pSibling);
			BalanceProc_Delete(pTarget, wasBlack);
		}

		if (false == pSibling->_pLeft->_black)
		{
			pSibling->_black = pSibling->_pParent->_black;
			pTarget->_pParent->_black = true;
			pSibling->_pLeft->_black = true;
			RotateRight(pTarget->_pParent);
		}
	}
}

template <typename DATA>
typename void cRBT<DATA>::RotateLeft(stNode* pAxis)
{
	if (nullptr == pAxis || _Nil == pAxis || _Nil == pAxis->_pRight)
	{
		return;
	}

	stNode* pAxisRight = pAxis->_pRight;
	if (_Nil != pAxisRight->_pLeft)
	{
		pAxis->_pRight = pAxisRight->_pLeft;
		pAxisRight->_pLeft->_pParent = pAxis;
	}
	else
	{
		pAxis->_pRight = _Nil;
	}

	pAxisRight->_pParent = pAxis->_pParent;
	if (nullptr != pAxis->_pParent && pAxis == pAxis->_pParent->_pLeft)
	{
		pAxis->_pParent->_pLeft = pAxisRight;
	}
	else if (nullptr != pAxis->_pParent && pAxis == pAxis->_pParent->_pRight)
	{
		pAxis->_pParent->_pRight = pAxisRight;
	}
	pAxis->_pParent = pAxisRight;
	pAxisRight->_pLeft = pAxis;

	if (nullptr == pAxisRight->_pParent)
	{
		_pRoot = pAxisRight;
	}
}

template <typename DATA>
typename void cRBT<DATA>::RotateRight(stNode* pAxis)
{
	if (nullptr == pAxis || _Nil == pAxis || _Nil == pAxis->_pLeft)
	{
		return;
	}

	stNode* pAxisLeft = pAxis->_pLeft;
	if (_Nil != pAxisLeft->_pRight)
	{
		pAxis->_pLeft = pAxisLeft->_pRight;
		pAxisLeft->_pRight->_pParent = pAxis;
	}
	else
	{
		pAxis->_pLeft = _Nil;
	}

	pAxisLeft->_pParent = pAxis->_pParent;
	if (nullptr != pAxis->_pParent && pAxis == pAxis->_pParent->_pLeft)
	{
		pAxis->_pParent->_pLeft = pAxisLeft;
	}
	else if (nullptr != pAxis->_pParent && pAxis == pAxis->_pParent->_pRight)
	{
		pAxis->_pParent->_pRight = pAxisLeft;
	}
	pAxis->_pParent = pAxisLeft;
	pAxisLeft->_pRight = pAxis;

	if (nullptr == pAxisLeft->_pParent)
	{
		_pRoot = pAxisLeft;
	}
}

template <typename DATA>
typename cRBT<DATA>::iterator& cRBT<DATA>::iterator::operator++()
{
	if (_Nil != _pNode->_pRight)
	{
		_pNode = _pNode->_pRight;
		while (_Nil != _pNode->_pLeft)
		{
			_pNode = _pNode->_pLeft;
		}
	}
	else
	{
		while (1)
		{
			if (_pNode->IsRoot())
			{
				_pNode = _Nil;
				break;
			}
			else if (_pNode->IsLeft())
			{
				_pNode = _pNode->_pParent;
				break;
			}
			else
			{
				//if (_pNode->IsRight())
				_pNode = _pNode->_pParent;
			}
		}
	}
	return *this;
}

template <typename DATA>
typename const cRBT<DATA>::iterator cRBT<DATA>::iterator::operator++(int)
{
	iterator retIter = _pNode;
	if (_Nil != _pNode->_pRight)
	{
		_pNode = _pNode->_pRight;
		while (_Nil != _pNode->_pLeft)
		{
			_pNode = _pNode->_pLeft;
		}
	}
	else
	{
		while (1)
		{
			if (_pNode->IsRoot())
			{
				_pNode = _Nil;
				break;
			}
			else if (_pNode->IsLeft())
			{
				_pNode = _pNode->_pParent;
				break;
			}
			else
			{
				//if (_pNode->IsRight())
				_pNode = _pNode->_pParent;
			}
		}
	}
	return retIter;
}

template <typename DATA>
typename cRBT<DATA>::iterator& cRBT<DATA>::iterator::operator--()
{
	if (_Nil != _pNode->_pLeft)
	{
		_pNode = _pNode->_pLeft;
		while (_Nil != _pNode->_pRight)
		{
			_pNode = _pNode->_pRight;
		}
	}
	else
	{
		while (1)
		{
			if (_pNode->IsRoot())
			{
				_pNode = _Nil;
				break;
			}
			else if (_pNode->IsRight())
			{
				_pNode = _pNode->_pParent;
				break;
			}
			else
			{
				//if (_pNode->IsLeft())
				_pNode = _pNode->_pParent;
			}
		}
	}
	return *this;
}

template <typename DATA>
typename const cRBT<DATA>::iterator cRBT<DATA>::iterator::operator--(int)
{
	iterator retIter = _pNode;
	if (_Nil != _pNode->_pLeft)
	{
		_pNode = _pNode->_pLeft;
		while (_Nil != _pNode->_pRight)
		{
			_pNode = _pNode->_pRight;
		}
	}
	else
	{
		while (1)
		{
			if (_pNode->IsRoot())
			{
				_pNode = _Nil;
				break;
			}
			else if (_pNode->IsRight())
			{
				_pNode = _pNode->_pParent;
				break;
			}
			else
			{
				//if (_pNode->IsLeft())
				_pNode = _pNode->_pParent;
			}
		}
	}
	return retIter;
}


template <typename DATA>
bool cRBT<DATA>::VerifyTree(int putSize)
{
	static_assert(std::is_integral<DATA>());	// 자료 구조 검증용 함수로 정수 자료형 기준으로만 동작함

	if (0 >= putSize)
	{
		DebugBreak();
		return false;
	}
	clear();

	// 데이터를 putSize만큼 삽입한 후에 삭제함, 매 삽입 및 삭제 과정에서 아래의 사항들을 확인
	// 1. 중위 순회를 하였을 때 데이터가 오름 차순으로 들어있는지
	// 2. 모든 경로에 대하여 BLACK 노드의 수가 동일하고, RED-RED로 연결된 경우가 없는지
	for (int i = 1; i <= putSize; ++i)
	{
		int countBlack = 0;
		insert(i);
		Verify_Insequence();
		Verify_CountBlack(_pRoot, 1, &countBlack);		// 루트는 블랙
	}
	for (int i = 1; i <= putSize; ++i)
	{
		int countBlack = 0;
		iterator iter = find(i);
		erase(iter);
		Verify_Insequence();
		Verify_CountBlack(_pRoot, 1, &countBlack);		// 루트는 블랙
	}

	return true;
}
template <typename DATA>
bool cRBT<DATA>::Verify_Insequence()
{
	if (_size == 0)
	{
		return true;
	}

	iterator iter = begin();
	int prevVal = *iter;
	for (iter; iter != end(); ++iter)
	{
		if (iter == begin())
		{
			continue;
		}

		if (prevVal > *iter)
		{
			DebugBreak();
			return false;
		}
		prevVal = *iter;
	}

	return true;
}
template <typename DATA>
bool cRBT<DATA>::Verify_CountBlack(stNode* pNode, int expectedBlackCount, int* pBlackCount)
{
	if (0 == _size)
	{
		return true;
	}

	if (_Nil != pNode->_pLeft)
	{
		if (pNode->_black == false && pNode->_pLeft->_black == false)
		{
			DebugBreak();
			return false;
		}
		if (true == pNode->_pLeft->_black)
		{
			Verify_CountBlack(pNode->_pLeft, ++expectedBlackCount, pBlackCount);
			--expectedBlackCount;
		}
		else
		{
			Verify_CountBlack(pNode->_pLeft, expectedBlackCount, pBlackCount);
		}
	}

	if (_Nil != pNode->_pRight)
	{
		if (pNode->_black == false && pNode->_pRight->_black == false)
		{
			DebugBreak();
			return false;
		}
		if (true == pNode->_pRight->_black)
		{
			Verify_CountBlack(pNode->_pRight, ++expectedBlackCount, pBlackCount);
			--expectedBlackCount;
		}
		else
		{
			Verify_CountBlack(pNode->_pRight, expectedBlackCount, pBlackCount);
		}
	}

	// 최하단 리프 노드이므로 블랙 노드 카운트를 비교함
	if (_Nil == pNode->_pLeft && _Nil == pNode->_pRight)
	{
		// 맨 처음으로 리프 노드를 만났을 경우 pBlackCount를 갱신 이후엔 갱신된 값으로 기대값을 비교
		if (0 == *pBlackCount)
		{
			*pBlackCount = expectedBlackCount;
		}
		
		if (*pBlackCount != expectedBlackCount)
		{
			DebugBreak();
			return false;
		}
	}

	return true;
}