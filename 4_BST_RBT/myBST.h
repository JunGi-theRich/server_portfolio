
#pragma once

#include <type_traits>

template <typename DATA>
class cBST
{
private:
	struct stNode;
public:
	class iterator;

public:
	cBST()
	{
		_pRoot = nullptr;
		_size = 0;
	}

	bool insert(const DATA& val);
	iterator find(const DATA& val);
	iterator erase(iterator whereIter);

	iterator begin()
	{
		stNode* pNode = _pRoot;
		while (nullptr != pNode->_pLeft)
		{
			pNode = pNode->_pLeft;
		}
		return pNode;
	}
	iterator end()
	{
		return nullptr;
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
	//==========

private:
	int _size;
	stNode* _pRoot;

private:
	struct stNode
	{
		friend class cBST;
		stNode()
		{
			memset(&_data, 0, sizeof(DATA));
			_pParent = nullptr;
			_pLeft = nullptr;
			_pRight = nullptr;
		}
		stNode(const DATA& val, stNode* pParent, stNode* pLeft, stNode* pRight)
		{
			_data = val;
			_pParent = pParent;
			_pLeft = pLeft;
			_pRight = pRight;
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

		stNode* _pParent;
		stNode* _pLeft;
		stNode* _pRight;
	};

public:
	class iterator
	{
		friend class cBST;

	public:
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
bool cBST<DATA>::insert(const DATA& val)
{
	stNode* pNewNode = new stNode(val, nullptr, nullptr, nullptr);

	if (nullptr == _pRoot)
	{
		_pRoot = pNewNode;
	}
	else
	{
		stNode* pParent = _pRoot;
		while (1)
		{
			if (pNewNode->_data < pParent->_data)
			{
				if (nullptr == pParent->_pLeft)
				{
					pParent->_pLeft = pNewNode;
					pNewNode->_pParent = pParent;
					break;
				}
				pParent = pParent->_pLeft;
			}
			else if (pNewNode->_data > pParent->_data)
			{
				if (nullptr == pParent->_pRight)
				{
					pParent->_pRight = pNewNode;
					pNewNode->_pParent = pParent;
					break;
				}
				pParent = pParent->_pRight;
			}
			else
			{
				// 중복값 허용하지 않음
				delete pNewNode;
				return false;
			}
		}
	}

	++_size;
	return true;
}

template <typename DATA>
typename cBST<DATA>::iterator cBST<DATA>::find(const DATA& val)
{
	stNode* pFindNode = _pRoot;
	while (1)
	{
		if (nullptr == pFindNode)
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
typename cBST<DATA>::iterator cBST<DATA>::erase(iterator whereIter)
{
	stNode* pDeleteNode = whereIter._pNode;
	++whereIter;

	// 삭제하려는 노드의 자식이 없는 경우
	if (nullptr == pDeleteNode->_pLeft && nullptr == pDeleteNode->_pRight)
	{
		if (_pRoot == pDeleteNode)
		{
			--_size;
			delete pDeleteNode;
			_pRoot = nullptr;
			return nullptr;
		}
		else
		{
			if (pDeleteNode->IsLeft())
			{
				pDeleteNode->_pParent->_pLeft = nullptr;
			}
			else
			{
				//pDeleteNode->IsRight()
				pDeleteNode->_pParent->_pRight = nullptr;
			}
			--_size;
			delete pDeleteNode;
			return whereIter;
		}
	}

	// 삭제하려는 노드의 자식이 두 개인 경우
	else if (nullptr != pDeleteNode->_pLeft && nullptr != pDeleteNode->_pRight)
	{
		// 삭제하려는 노드의 데이터를 왼쪽 자식 노드의 가장 큰 데이터로 교체하는 방식으로 진행
		// (또는 오른쪽 자식 중 가장 작은 값을 교체하는 방식도 가능함)
		stNode* pExchange = pDeleteNode->_pLeft;
		while (1)
		{
			if (nullptr == pExchange->_pRight)
			{
				// 오른쪽 노드면서 리프 노드인 경우 == 삭제하려는 노드의 왼쪽 자식 노드들 중 가장 큰 값
				break;
			}
			pExchange = pExchange->_pRight;
		}
		pDeleteNode->_data = pExchange->_data;
		pDeleteNode = pExchange;

		// pExchange를 대신 삭제하되, 왼쪽 자식이 있다면 부모와 연결해주어야 함(로직상 오른쪽 자식은 없음)
		if (nullptr != pExchange->_pLeft)
		{
			if (pExchange->IsLeft())
			{
				pExchange->_pParent->_pLeft = pExchange->_pLeft;
				pExchange->_pLeft->_pParent = pExchange->_pParent;
			}
			else
			{
				// if(pExchange->IsRight())
				pExchange->_pParent->_pRight = pExchange->_pLeft;
				pExchange->_pLeft->_pParent = pExchange->_pParent;
			}
		}
		else
		{
			if (pExchange->IsLeft())
			{
				pExchange->_pParent->_pLeft = nullptr;
			}
			else
			{
				pExchange->_pParent->_pRight = nullptr;
			}
		}
		--_size;
		delete pDeleteNode;
		return whereIter;
	}

	// 삭제하려는 노드의 자식이 하나인 경우
	else
	{
		if (nullptr != pDeleteNode->_pLeft)
		{
			if (_pRoot == pDeleteNode)
			{
				_pRoot = pDeleteNode->_pLeft;
				_pRoot->_pParent = nullptr;
			}
			else
			{
				if (pDeleteNode->IsLeft())
				{
					pDeleteNode->_pParent->_pLeft = pDeleteNode->_pLeft;
					pDeleteNode->_pLeft->_pParent = pDeleteNode->_pParent;
				}
				else
				{
					// if (pDeleteNode->IsRight())
					pDeleteNode->_pParent->_pRight = pDeleteNode->_pLeft;
					pDeleteNode->_pLeft->_pParent = pDeleteNode->_pParent;
				}
			}
		}
		else
		{
			if (_pRoot == pDeleteNode)
			{
				_pRoot = pDeleteNode->_pRight;
				_pRoot->_pParent = nullptr;
			}
			else
			{
				if (pDeleteNode->IsLeft())
				{
					pDeleteNode->_pParent->_pLeft = pDeleteNode->_pRight;
					pDeleteNode->_pRight->_pParent = pDeleteNode->_pParent;
				}
				else
				{
					// if (pDeleteNode->IsRight())
					pDeleteNode->_pParent->_pRight = pDeleteNode->_pRight;
					pDeleteNode->_pRight->_pParent = pDeleteNode->_pParent;
				}
			}
		}
		--_size;
		delete pDeleteNode;
		return whereIter;
	}
}


template <typename DATA>
typename cBST<DATA>::iterator& cBST<DATA>::iterator::operator++()
{
	if (nullptr != _pNode->_pRight)
	{
		_pNode = _pNode->_pRight;
		while (nullptr != _pNode->_pLeft)
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
				_pNode = nullptr;
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
typename const cBST<DATA>::iterator cBST<DATA>::iterator::operator++(int)
{
	iterator retIter = _pNode;
	if (nullptr != _pNode->_pRight)
	{
		_pNode = _pNode->_pRight;
		while (nullptr != _pNode->_pLeft)
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
				_pNode = nullptr;
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
typename cBST<DATA>::iterator& cBST<DATA>::iterator::operator--()
{
	if (nullptr != _pNode->_pLeft)
	{
		_pNode = _pNode->_pLeft;
		while (nullptr != _pNode->_pRight)
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
				_pNode = nullptr;
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
typename const cBST<DATA>::iterator cBST<DATA>::iterator::operator--(int)
{
	iterator retIter = _pNode;
	if (nullptr != _pNode->_pLeft)
	{
		_pNode = _pNode->_pLeft;
		while (nullptr != _pNode->_pRight)
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
				_pNode = nullptr;
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
bool cBST<DATA>::VerifyTree(int putSize)
{
	static_assert(std::is_integral<DATA>());	// 자료 구조 검증용 함수로 정수 자료형 기준으로만 동작함

	if (0 >= putSize)
	{
		DebugBreak();
		return false;
	}
	clear();

	// 1. 1부터 putSize만큼 데이터를 집어넣어 중위순회가 깨지지 않았는지 확인
	for (int i = 1; i <= putSize; ++i)
	{
		insert(i);
		Verify_Insequence();
	}

	// 2. 1부터 pustSize만큼 데이터를 삭제하며 중위순회가 깨지지 않았는지 확인
	// 이 때 find에 실패하면 nullptr이 반환되므로 erase(iter)는 실패(삽입한 데이터가 소멸됐음을 의미)
	for (int i = 1; i <= putSize; ++i)
	{
		iterator iter = find(i);
		erase(iter);
		Verify_Insequence();
	}

	return true;
}
template <typename DATA>
bool cBST<DATA>::Verify_Insequence()
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