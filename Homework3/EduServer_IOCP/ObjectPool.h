#pragma once


#include "Exception.h"
#include "FastSpinlock.h"


//도대체 이 pool은 또 언제 쓰이는 풀인가???
template <class TOBJECT, int ALLOC_COUNT = 100>
//ClassTypeLock에 선언 되어 있는 LockGurad를 쓰기 위해 상속
class ObjectPool : public ClassTypeLock<TOBJECT>
{
public:

	static void* operator new(size_t objSize)
	{
		//TODO: TOBJECT 타입 단위로 lock 잠금
        LockGuard lock;

		if (!mFreeList)
		{
			mFreeList = new uint8_t[sizeof(TOBJECT)*ALLOC_COUNT];

			uint8_t* pNext = mFreeList;
			uint8_t** ppCurr = reinterpret_cast<uint8_t**>(mFreeList);

			for (int i = 0; i < ALLOC_COUNT - 1; ++i)
			{
				pNext += sizeof(TOBJECT);
				*ppCurr = pNext;
				ppCurr = reinterpret_cast<uint8_t**>(pNext);
			}

            *ppCurr = nullptr;
			mTotalAllocCount += ALLOC_COUNT;
		}

		uint8_t* pAvailable = mFreeList;
		mFreeList = *reinterpret_cast<uint8_t**>(pAvailable);
        //mCurrentUseCount는 어디에 쓰는 거지? ///# 직접 쓰는데 없다. delete에서 crash체크용 ^^
        //나는 이게 다음 빈 곳을 찾아주는 것인 줄 알았는데
        //그냥 mFreeList를 넘겨 주네
		++mCurrentUseCount;

		return pAvailable;
	}

	static void	operator delete(void* obj)
	{
		//TODO: TOBJECT 타입 단위로 lock 잠금
        LockGuard lock;


		CRASH_ASSERT(mCurrentUseCount > 0);

		--mCurrentUseCount;

		*reinterpret_cast<uint8_t**>(obj) = mFreeList;
		mFreeList = static_cast<uint8_t*>(obj);
	}


private:
	static uint8_t*	mFreeList;
	static int		mTotalAllocCount; ///< for tracing
	static int		mCurrentUseCount; ///< for tracing
    
};


template <class TOBJECT, int ALLOC_COUNT>
uint8_t* ObjectPool<TOBJECT, ALLOC_COUNT>::mFreeList = nullptr;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mTotalAllocCount = 0;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mCurrentUseCount = 0;


