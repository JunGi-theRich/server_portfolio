
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
	// Ʈ�� ������ �Լ��� ���� �ڷ��������� ������
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

		// ��� �۾��� ��ģ �Ŀ� _pRoot�� false(RED)��� true(BLACK)�� �ٲ�
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
	pDeleteNode = DeleteProc(pDeleteNode, &bWasBlack);			// ������ ��� ���� Ȯ�� �� �ڽ� ��带 ����(Nil �Ǵ� �ڽ� ���)
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
				// �ߺ��� ������� ����
				return nullptr;
			}
		}
	}
}

template <typename DATA>
void cRBT<DATA>::BalanceProc_Insert(stNode* pTarget)
{
	// �Լ� ���� ����1 - �θ� �� ���θ� ����� ����
	if (nullptr == pTarget->_pParent || nullptr == pTarget->_pParent->_pParent)
	{
		return;
	}

	// �Լ� ���� ����2 - �θ� ��尡 false(RED)
	if (pTarget->_pParent->_black)
	{
		return;
	}

	stNode* pParent = pTarget->_pParent;
	stNode* pGrandParent = pParent->_pParent;

	// ���θ� ��� ���ʿ� �θ� ��尡 �ִ� ���
	if (pParent->IsLeft())
	{
		// 1. ���� ��尡 false(RED)
		if (!pGrandParent->_pRight->_black)
		{
			// 1-(1). ���θ� ��� false(RED)�� ����
			// 1-(2). �θ�� ���� ��带 true(BLACK)�� ����
			// 1-(3). �� �۾��� ���� ���� �ö󰡸� �ݺ�

			pGrandParent->_black = false;
			pParent->_black = true;
			pGrandParent->_pRight->_black = true;
			BalanceProc_Insert(pGrandParent);
		}

		// 2. ���� ��尡 true(BLACK)�̰�, pTarget�� �θ� ����� ���ʿ� �ִ� ���
		else if(pTarget->IsLeft())
		{
			// 2-(1). �θ� ��带 true(RED)�� ����
			// 2-(2). ���θ� ��带 false(RED)�� ����
			// 2-(3). ���θ� ��带 �������� ��ȸ��

			pParent->_black = true;
			pGrandParent->_black = false;
			RotateRight(pGrandParent);
		}

		// 3. ���� ��尡 true(BLACK)�̰�, pTarget�� �θ� ����� �����ʿ� �ִ� ���
		else
		{
			// 3-(1). �θ� ��带 �������� ��ȸ���Ͽ� �������� ��带 ������
			// 3-(2). �ű� ����� ������ true(BLACK)���� ����
			// 3-(3). �ű� ����� �θ� ���(�� ���θ� ���)�� false(RED)�� ����
			// 3-(4). �ű� ����� �θ� ���(�� ���θ� ���)�� �������� ��ȸ��

			RotateLeft(pParent);
			pTarget->_black = true;
			pGrandParent->_black = false;
			RotateRight(pGrandParent);
		}
	}

	// ���θ� ��� �����ʿ� �θ� ��尡 �ִ� ��� �� ���� �ݴ�� ������
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
	// pTarget�� �ڽ� ��尡 ���� ���
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

	// pTarget�� �� ���� �ڽ��� ���� ���
	else if (_Nil != pTarget->_pLeft && _Nil != pTarget->_pRight)
	{
		// �����Ϸ��� ����� ���� �ڽ� ���� �߿��� ���� ū ������ ��ü�ϰ� �ش� ��带 ��� ����
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

	// pTarget�� �ڽ� ��带 �ϳ� ���� ���
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
	// �Ʒ� ��Ȳ�� ��� �� �Լ��� ����� �� ����
	// 1. ������ ��尡 ���������� ���� ��忴�� ���
	// 2. ������ ����� ������ false(RED)���� ���
	// 3. ��ͷ� ���� ȣ�⿡�� pTarget�� ��Ʈ ��尡 �� ���
	if (nullptr == _pRoot || false == wasBlack || _pRoot == pTarget)
	{
		return;
	}

	// ���� ū �б�� pTarget�� �θ� ����� ���� �Ǵ� ������ ������� �Ǵ�

	if (pTarget->IsLeft())
	{
		stNode* pSibling = pTarget->_pParent->_pRight;

		// 1. pTarget�� false(RED) ����� ���
		if (false == pTarget->_black)
		{
			// 1-(1). pTarget ��带 true(BLACK)���� ����

			pTarget->_black = true;
			return;	
		}

		// 2. pSibling�� false(RED)�� ���
		if (false == pSibling->_black)
		{
			// 2-(1). ���� ��带 true(BLACK)�� ����
			// 2-(2). �θ� �뵵�� false(RED)�� ����
			// 2-(3). �θ� ��带 �������� ��ȸ��
			// 2-(4). pTarget ��带 �������� �뷱�� �۾� ������

			pSibling->_black = true;
			pTarget->_pParent->_black = false;
			RotateLeft(pTarget->_pParent);
			BalanceProc_Delete(pTarget, wasBlack);
			return;
		}

		// 3. pSibling�� true(BLACK)�̰�, ���� �ڽ��� true(BLACK)
		if (true == pSibling->_pLeft->_black && true == pSibling->_pRight->_black)
		{
			// 3-(1). ���� ��带 false(RED)�� ����
			// 3-(2). �̷� ���� ������ �뷱���� �¾�����, �θ� ��带 �������� �뷱�� �۾� ������

			pSibling->_black = false;
			BalanceProc_Delete(pTarget->_pParent, wasBlack);
			return;
		}

		// 4. pSibling�� true(BLACK)�̰�, ������ �ڽ��� true(BLACK)
		if (true == pSibling->_pRight->_black)
		{
			// 4-(1). ���� ��带 false(RED)�� ����
			// 4-(2). ������ ���� �ڽ��� true(BLACK)�� ����
			// 4-(3). ���� ��带 �������� ��ȸ��
			// 4-(4). 5�� ���̽��� ������ ��Ȳ�� �Ǹ�, �뷱�� �۾� ������

			pSibling->_black = false;
			pSibling->_pLeft->_black = true;
			RotateRight(pSibling);
			BalanceProc_Delete(pTarget, wasBlack);
			//!!return;
		}

		// 5. pSibling�� true(BLACK)�̰�, ������ �ڽ��� false(RED)
		if (false == pSibling->_pRight->_black)
		{
			// 5-(1). ���� ����� ������ �θ��� �������� ����
			// 5-(2). ���� ����� �ڽ� ���� ������ true(BLACK)�� ����
			// 5-(3). �θ� ����� ������ true(BLACK)�� ����
			// 5-(4). �θ� ��带 �������� ��ȸ���Ͽ� �뷱���� ����

			pSibling->_black = pSibling->_pParent->_black;
			pTarget->_pParent->_black = true;
			pSibling->_pRight->_black = true;
			RotateLeft(pTarget->_pParent);
			//!!return;
		}
	}
	else
	{
		// ��� ������ �� ������ ��Ī���� �����

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
	static_assert(std::is_integral<DATA>());	// �ڷ� ���� ������ �Լ��� ���� �ڷ��� �������θ� ������

	if (0 >= putSize)
	{
		DebugBreak();
		return false;
	}
	clear();

	// �����͸� putSize��ŭ ������ �Ŀ� ������, �� ���� �� ���� �������� �Ʒ��� ���׵��� Ȯ��
	// 1. ���� ��ȸ�� �Ͽ��� �� �����Ͱ� ���� �������� ����ִ���
	// 2. ��� ��ο� ���Ͽ� BLACK ����� ���� �����ϰ�, RED-RED�� ����� ��찡 ������
	for (int i = 1; i <= putSize; ++i)
	{
		int countBlack = 0;
		insert(i);
		Verify_Insequence();
		Verify_CountBlack(_pRoot, 1, &countBlack);		// ��Ʈ�� ��
	}
	for (int i = 1; i <= putSize; ++i)
	{
		int countBlack = 0;
		iterator iter = find(i);
		erase(iter);
		Verify_Insequence();
		Verify_CountBlack(_pRoot, 1, &countBlack);		// ��Ʈ�� ��
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

	// ���ϴ� ���� ����̹Ƿ� �� ��� ī��Ʈ�� ����
	if (_Nil == pNode->_pLeft && _Nil == pNode->_pRight)
	{
		// �� ó������ ���� ��带 ������ ��� pBlackCount�� ���� ���Ŀ� ���ŵ� ������ ��밪�� ��
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