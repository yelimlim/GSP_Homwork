#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "SyncExecutable.h"
#include "Timer.h"



Timer::Timer()
{
	LTickCount = GetTickCount64();
}


void Timer::PushTimerJob(SyncExecutablePtr owner, const TimerTask& task, uint32_t after)
{
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);

	//TODO: mTimerJobQueue에 TimerJobElement를 push..
    TimerJobElement job{ owner, task, (int64_t)after }; ///# 만들어진 생성자가 있는데 왜 굳이 {} 생성을? 

	int64_t dueTimeTick = after + LTickCount; ///# Timer 코드의 동작을 이해하고 이렇게 해야함.
	mTimerJobQueue.push(TimerJobElement(owner, task, dueTimeTick));

    ///mTimerJobQueue.push(job);
}


void Timer::DoTimerJob()
{
	/// thread tick update
	LTickCount = GetTickCount64();

	while (!mTimerJobQueue.empty())
	{
		const TimerJobElement& timerJobElem = mTimerJobQueue.top(); 

		if (LTickCount < timerJobElem.mExecutionTick)
			break;

		timerJobElem.mOwner->EnterLock();
		
		timerJobElem.mTask();

		timerJobElem.mOwner->LeaveLock();

		mTimerJobQueue.pop();
	}


}

