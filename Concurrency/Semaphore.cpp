
#define KERNEL_SEMAPHORE	_MSC_VER

#include "Semaphore.h"
#include "atomics.h"
#include "defines.h"
#include <Profiling\Trace.h>
#if KERNEL_SEMAPHORE
#include <Windows.h>
#else
#include <mutex>
#include <condition_variable>
#endif

namespace ks {

#if !KERNEL_SEMAPHORE
	struct SemContext
	{
		SemContext() : mCount(0)
		{}
		unsigned int			mCount;
		std::mutex				mtx;
		std::condition_variable cv;
	};

	Semaphore::Semaphore()
	{
		mCtx = (uintptr_t)new SemContext();
	}

	Semaphore::~Semaphore()
	{
		destroy();
	}

	void Semaphore::destroy()
	{
		delete (SemContext*)mCtx;
	}

	void Semaphore::signal(int count /*= 1*/)
	{
		SemContext* ctx = (SemContext*)mCtx;
		std::unique_lock<std::mutex> lock(ctx->mtx);
		++ctx->mCount;
		lock.unlock();
		ctx->cv.notify_one();	// TODO: notify count
	}

	void Semaphore::wait()
	{
		SemContext* ctx = (SemContext*)mCtx;
		std::unique_lock<std::mutex> lock(ctx->mtx);
		ctx->cv.wait(lock, [ctx]() { return ctx->mCount != 0; });
		--ctx->mCount;
	}

#else

	Semaphore::Semaphore() : mNumSignals(0)
	{
		mCtx = (uintptr_t)CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
	}

	Semaphore::~Semaphore()
	{
		destroy();
	}

	void Semaphore::destroy()
	{
		if(mCtx) CloseHandle((HANDLE)mCtx);
	}

	void Semaphore::signal(int count /*= 1*/)
	{
		TRACE_FUNC();
		if ((int)atomic_add((u32*)&mNumSignals, count) <= 0)	// avoid kernel transaction if no waiting threads
		{
			ReleaseSemaphore((HANDLE)mCtx, count, 0);
		}
	}

	bool Semaphore::wait(unsigned timeout /*= 0xffffffff*/)
	{
		TRACE_FUNC();
		if ((int)atomic_decrement((u32*)&mNumSignals) < 0)	// avoid kernel transaction if there's pending signals
		{
			if (WaitForSingleObject((HANDLE)mCtx, timeout) != WAIT_OBJECT_0)
			{
				atomic_increment((u32*)&mNumSignals);	// wait failed, rectify counter else it'll de-sync with kernel counter
				return false;
			}
		}
		return true;
	}

#endif

	Semaphore::Semaphore(Semaphore&& o) : mCtx(NULL)
	{
		*this = ks::move(o);
	}

	void Semaphore::operator = (Semaphore&& o)
	{
		if ( this != &o )
		{
			destroy();
			mCtx = o.mCtx;
			o.mCtx = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Event
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define EVENT_STATE_BIT					31

	Event::Event(bool state) : mState( (int)state << EVENT_STATE_BIT )
	{}

	void Event::SetState(bool state)
	{
		if (state)
			ATOMIC_SET_BIT(mState, EVENT_STATE_BIT);
		else
			ATOMIC_CLEAR_BIT(mState, EVENT_STATE_BIT);
	}

	void Event::Notify()
	{
		unsigned count = atomic_and(&mState, 0) & ~(1 << EVENT_STATE_BIT);
		if (count)
			mSem.signal(count);
	}

	void Event::Wait(int timeoutMS /*= 0xffffffff*/)
	{
		unsigned val(0);
		do
		{
			val = mState;
		} while ((val >> EVENT_STATE_BIT) != 0 && atomic_compare_and_swap(&mState, val, val + 1) != val);
		
		if ((val >> EVENT_STATE_BIT))
		{
			if (mSem.wait(timeoutMS) == false)
				atomic_decrement(&mState);
		}
	}
}
