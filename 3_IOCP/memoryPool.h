#pragma once

#include <Windows.h>
#include <cassert>
#include <new>							// placementNew

namespace MemoryPool
{
	template <typename DATA>
	class cPoolArr_stack
	{
	private:
		template <typename DATA>
		struct stDataNode
		{
		private:
			friend class cPoolArr_stack;
			stDataNode() {}
			~stDataNode() {}

		private:

#ifndef NDEBUG
			__int64 _underflowGuard;			// 디버깅 가드
#endif // !NDEBUG

			DATA _data;
			stDataNode<DATA>* _pNext;

#ifndef NDEBUG
			bool _bInUse;						// 노드 사용 여부
			__int64 _overflowGuard;				// 디버깅 가드
#endif // !NDEBUG
		};

	public:
		cPoolArr_stack(int poolSize, bool placementNew = true);
		~cPoolArr_stack();

	public:
		DATA* Alloc();
		void Free(DATA* pFree);

		int GetTotalSize() const { return _totalSize; }
		int GetUsedSize() const { return _usedSize; }

	private:
		stDataNode<DATA>* _pTop;			// 스택 구조 메모리풀 상단부 포인터
		stDataNode<DATA>* _pStart;			// 메모리 해제를 위한 시작부 포인터

		int _totalSize;						// 메모리풀 전체 노드 수
		int _usedSize;						// 메모리풀 현재 사용중인 노드 수
		bool _bPlacementNew;				// placementNew 사용 여부

		SRWLOCK _srwPool;					// 멀티 스레드 환경용
	};

	template <typename DATA>
	class cPoolList_stack
	{
	private:
		template <typename DATA>
		struct stDataNode
		{
		private:
			friend class cPoolList_stack;
			stDataNode() {}
			~stDataNode() {}

		private:

#ifndef NDEBUG
			__int64 _underflowGuard;			// 디버깅 가드
#endif // !NDEBUG

			DATA _data;
			stDataNode<DATA>* _pNext;

#ifndef NDEBUG
			bool _bInUse;						// 노드 사용 여부
			__int64 _overflowGuard;				// 디버깅 가드
#endif // !NDEBUG
		};

	public:
		cPoolList_stack(int poolSize, bool placementNew = true);
		~cPoolList_stack();

	public:
		DATA* Alloc();
		void Free(DATA* pFree);

		int GetTotalSize() const { return _totalSize; }
		int GetUsedSize() const { return _usedSize; }

	private:
		stDataNode<DATA>* _pTop;			// 스택 구조 메모리풀 상단부 포인터

		int _totalSize;						// 메모리풀 전체 노드 수
		int _usedSize;						// 메모리풀 현재 사용중인 노드 수
		bool _bPlacementNew;				// placementNew 사용 여부

		SRWLOCK _srwPool;					// 멀티 스레드 환경용
	};






	//============================================================
	//============================================================
	//함수 정의 모음

	template <typename DATA>
	cPoolArr_stack<DATA>::cPoolArr_stack(int poolSize, bool placementNew)
	{
		_totalSize = poolSize;
		_usedSize = 0;
		_bPlacementNew = placementNew;
		InitializeSRWLock(&_srwPool);
		if (0 >= _totalSize)
		{
			_pTop = nullptr;
			_pStart = nullptr;
		}
		else
		{
			_pStart = (stDataNode<DATA>*)malloc(sizeof(stDataNode<DATA>) * _totalSize);
			if (nullptr != _pStart)
			{
				for (int i = 0; i < _totalSize; ++i)
				{
#ifndef NDEBUG
					_pStart[i]._underflowGuard = _pStart[i]._overflowGuard = (__int64)this;
					_pStart[i]._bInUse = false;
#endif // NDEBUG

					if (!_bPlacementNew)
					{
						// 생성자가 메모리풀 객체를 생성한 순간에만 호출되길 바라는 경우
						new(&(_pStart[i]._data)) DATA;
					}

					if (i != 0)
					{
						_pStart[i]._pNext = &_pStart[i - 1];
					}
					else
					{
						_pStart[i]._pNext = nullptr;
					}
				}

				// 모든 작업을 마친 후에는 _pTop을 마지막 노드로 이동
				_pTop = &(_pStart[_totalSize - 1]);
			}
			else
			{
				abort();
			}
		}
	}
	template <typename DATA>
	cPoolArr_stack<DATA>::~cPoolArr_stack()
	{
		if (!_bPlacementNew)
		{
			for (int i = 0; i < _totalSize; ++i)
			{
				_pStart[i]._data.~DATA();
			}
		}
		_usedSize = 0;
		free(_pStart);
	}

	template <typename DATA>
	DATA* cPoolArr_stack<DATA>::Alloc()
	{
		if (nullptr == _pTop)
		{
			return nullptr;
		}

		AcquireSRWLockExclusive(&_srwPool);
		stDataNode<DATA>* pRetNode = _pTop;

#ifndef NDEBUG
		assert(false == pRetNode->_bInUse);			// Re-allocation
		pRetNode->_bInUse = true;
#endif // !NDEBUG

		_pTop = _pTop->_pNext;
		++_usedSize;
		ReleaseSRWLockExclusive(&_srwPool);

		if (_bPlacementNew)
		{
			// Alloc 호출시마다 생성자가 호출되길 바라는 경우
			new(&(pRetNode->_data)) DATA;
		}
		return &(pRetNode->_data);
	}
	template <typename DATA>
	void cPoolArr_stack<DATA>::Free(DATA* pFree)
	{
		if (nullptr != pFree)
		{
			UINT_PTR position = (UINT_PTR)pFree;
			size_t ofs = (size_t) & (((stDataNode<DATA>*)0)->_data);
			//size_t ofs = offsetof(stDataNode<DATA>, stDataNode<DATA>::_data);
			position -= ofs;
			stDataNode<DATA>* pFreeNode = (stDataNode<DATA>*)position;

#ifndef NDEBUG
			assert((__int64)this == pFreeNode->_underflowGuard);		// Damaged Guard
			assert((__int64)this == pFreeNode->_overflowGuard);			// Damaged Guard
			assert(true == pFreeNode->_bInUse);							// Re-free
			pFreeNode->_bInUse = false;
#endif // !NDEBUG

			if (_bPlacementNew)
			{
				// Free 호출시마다 소멸자가 호출
				pFreeNode->_data.~DATA();
			}

			AcquireSRWLockExclusive(&_srwPool);
			pFreeNode->_pNext = _pTop;
			_pTop = pFreeNode;
			--_usedSize;
			ReleaseSRWLockExclusive(&_srwPool);
		}
	}


	template <typename DATA>
	cPoolList_stack<DATA>::cPoolList_stack(int poolSize, bool placementNew)
	{
		_totalSize = poolSize;
		_usedSize = 0;
		_bPlacementNew = placementNew;
		InitializeSRWLock(&_srwPool);
		if (0 >= _totalSize)
		{
			_pTop = nullptr;
		}
		else
		{
			for (int i = 0; i < _totalSize; ++i)
			{
				stDataNode<DATA>* pNewNode = (stDataNode<DATA>*)malloc(sizeof(stDataNode<DATA>));
				if (nullptr != pNewNode)
				{
#ifndef NDEBUG
					pNewNode->_underflowGuard = pNewNode->_overflowGuard = (__int64)this;
					pNewNode->_bInUse = false;
#endif // !NDEBUG

					if (!_bPlacementNew)
					{
						new(&(pNewNode->_data)) DATA;
					}
					if (0 == i)
					{
						pNewNode->_pNext = nullptr;
					}
					else
					{
						pNewNode->_pNext = _pTop;
					}
					_pTop = pNewNode;
				}
				else
				{
					abort();
				}
			}
		}
	}
	template <typename DATA>
	cPoolList_stack<DATA>::~cPoolList_stack()
	{
		while (_pTop)
		{
			stDataNode<DATA>* pNextNode = _pTop->_pNext;
			if (!_bPlacementNew)
			{
				_pTop->_data.~DATA();
			}
			free(_pTop);
			_pTop = pNextNode;
		}
	}

	template <typename DATA>
	DATA* cPoolList_stack<DATA>::Alloc()
	{
		// 초기에 지정한 크기를 넘어서는 경우
		if (nullptr == _pTop)
		{
			stDataNode<DATA>* pNewNode = (stDataNode<DATA>*)malloc(sizeof(stDataNode<DATA>));
			if (nullptr != pNewNode)
			{

#ifndef NDEBUG
				pNewNode->_underflowGuard = pNewNode->_overflowGuard = (__int64)this;
				pNewNode->_bInUse = true;
#endif // !NDEBUG

				if (!_bPlacementNew)
				{
					new(&(pNewNode->_data)) DATA;
				}
				AcquireSRWLockExclusive(&_srwPool);
				++_totalSize;
				++_usedSize;
				ReleaseSRWLockExclusive(&_srwPool);
				return &(pNewNode->_data);
			}
			else
			{
				return nullptr;
			}
		}

		AcquireSRWLockExclusive(&_srwPool);
		stDataNode<DATA>* pRetNode = _pTop;

#ifndef NDEBUG
		assert(false == pRetNode->_bInUse);			// Re-allocation
		pRetNode->_bInUse = true;
#endif // !NDEBUG

		_pTop = _pTop->_pNext;
		++_usedSize;
		ReleaseSRWLockExclusive(&_srwPool);

		if (_bPlacementNew)
		{
			// Alloc 호출시마다 생성자가 호출되길 바라는 경우
			new(&(pRetNode->_data)) DATA;
		}
		return &(pRetNode->_data);
	}
	template <typename DATA>
	void cPoolList_stack<DATA>::Free(DATA* pFree)
	{
		if (nullptr != pFree)
		{
			UINT_PTR position = (UINT_PTR)pFree;
			size_t ofs = (size_t) & (((stDataNode<DATA>*)0)->_data);
			//size_t ofs = offsetof(stDataNode<DATA>, stDataNode<DATA>::_data);
			position -= ofs;
			stDataNode<DATA>* pFreeNode = (stDataNode<DATA>*)position;

#ifndef NDEBUG
			assert((__int64)this == pFreeNode->_underflowGuard);		// Damaged Guard
			assert((__int64)this == pFreeNode->_overflowGuard);			// Damaged Guard
			assert(true == pFreeNode->_bInUse);							// Re-free
			pFreeNode->_bInUse = false;
#endif // !NDEBUG

			if (_bPlacementNew)
			{
				// Free 호출시마다 소멸자가 호출되길 바라는 경우
				pFreeNode->_data.~DATA();
			}

			AcquireSRWLockExclusive(&_srwPool);
			pFreeNode->_pNext = _pTop;
			_pTop = pFreeNode;
			--_usedSize;
			ReleaseSRWLockExclusive(&_srwPool);
		}
	}
}