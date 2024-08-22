
#pragma once

#include <type_traits>

template <typename DATA>
class cList
{
private:
	struct stNode;

public:
	class iterator;
	friend iterator;
public:

	cList();
	~cList() {}

	iterator begin() { return _headDummy._pNextNode; }
	iterator end() { return &_tailDummy; }
	iterator erase(iterator whereIter);

	void push_front(const DATA& val);
	void push_back(const DATA& val);

	DATA& front() { return _headDummy._pNextNode->_data; }
	DATA& back() { return _tailDummy._pPrevNode->_data; }

	void pop_front();
	void pop_back();

	int size() { return _size; }
	bool empty();
	void clear();

	void remove(const DATA& val);
	bool find(const DATA& val);

private:
	struct stNode
	{
		friend class cList;

		stNode()
		{
			//memset(&_data, 0, sizeof(DATA));
		}
		stNode(const DATA& val)
		{
			_data = val;
		}
		~stNode() {}
	private:
		DATA _data;
		stNode* _pNextNode;
		stNode* _pPrevNode;
	};

public:
	class iterator
	{
		friend class cList;

	public:
		iterator(stNode* pNode = nullptr)
		{
			_pNode = pNode;
		}

		iterator& operator++()
		{
			_pNode = _pNode->_pNextNode;
			return *this;
		}
		const iterator operator++(int)
		{
			iterator retIter = _pNode;
			_pNode = _pNode->_pNextNode;
			return retIter;
		}

		iterator& operator--()
		{
			_pNode = _pNode->_pPrevNode;
			return *this;
		}
		const iterator operator--(int)
		{
			iterator retIter = _pNode;
			_pNode = _pNode->_pPrevNode;
			return retIter;
		}

		DATA& operator*()
		{
			return _pNode->_data;
		}
		bool operator==(const iterator& other)
		{
			// _data 와 함께 주소값까지 동일해야함
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

private:
	int _size;
	stNode _headDummy;
	stNode _tailDummy;
};


// ==========================================================================================
// ==========================================================================================
// ==========================================================================================
// ==========================================================================================


template <typename DATA>
cList<DATA>::cList()
{
	_size = 0;

	_headDummy._pPrevNode = nullptr;
	_headDummy._pNextNode = &_tailDummy;

	_tailDummy._pPrevNode = &_headDummy;
	_tailDummy._pNextNode = nullptr;
}

template <typename DATA>
typename cList<DATA>::iterator cList<DATA>::erase(iterator whereIter)
{
	stNode* pEraseNode = whereIter._pNode;
	stNode* pPrevNode = whereIter._pNode->_pPrevNode;
	stNode* pNextNode = whereIter._pNode->_pNextNode;

	pPrevNode->_pNextNode = pNextNode;
	pNextNode->_pPrevNode = pPrevNode;

	whereIter = whereIter._pNode->_pNextNode;
	delete pEraseNode;
	--_size;

	return whereIter;
}

template <typename DATA>
void cList<DATA>::push_front(const DATA& val)
{
	stNode* pNewNode = new stNode(val);
	stNode* pNextNode = _headDummy._pNextNode;

	pNewNode->_pPrevNode = &_headDummy;
	pNewNode->_pNextNode = pNextNode;

	_headDummy._pNextNode = pNewNode;
	pNextNode->_pPrevNode = pNewNode;

	++_size;
}
template <typename DATA>
void cList<DATA>::push_back(const DATA& val)
{
	stNode* pNewNode = new stNode(val);
	stNode* pPrevNode = _tailDummy._pPrevNode;

	pNewNode->_pPrevNode = pPrevNode;
	pNewNode->_pNextNode = &_tailDummy;

	_tailDummy._pPrevNode = pNewNode;
	pPrevNode->_pNextNode = pNewNode;

	++_size;
}

template <typename DATA>
void cList<DATA>::pop_front()
{
	if (0 == _size)
	{
		return;
	}

	stNode* pDeleteNode = _headDummy._pNextNode;
	stNode* pNextNode = pDeleteNode->_pNextNode;

	pNextNode->_pPrevNode = &_headDummy;
	_headDummy._pNextNode = pNextNode;

	delete pDeleteNode;
	--_size;
}
template <typename DATA>
void cList<DATA>::pop_back()
{
	if (0 == _size)
	{
		return;
	}

	stNode* pDeleteNode = _tailDummy._pPrevNode;
	stNode* pPrevNode = pDeleteNode->_pPrevNode;

	pPrevNode->_pNextNode = &_tailDummy;
	_tailDummy._pPrevNode = pPrevNode;

	delete pDeleteNode;
	--_size;
}


template <typename DATA>
bool cList<DATA>::empty()
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
template <typename DATA>
void cList<DATA>::clear()
{
	while (!empty())
	{
		pop_front();
	}
}

template <typename DATA>
void cList<DATA>::remove(const DATA& val)
{
	iterator iter;
	for (iter = begin(); iter != end(); ++iter)
	{
		if (*iter == val)
		{
			erase(iter);
			return;
		}
	}
}
template <typename DATA>
bool cList<DATA>::find(const DATA& val)
{
	iterator iter;
	for (iter = begin(); iter != end(); ++iter)
	{
		if (*iter == val)
		{
			return true;
		}
	}
	return false;
}