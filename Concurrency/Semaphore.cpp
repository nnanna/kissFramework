
#define KERNEL_SEMAPHORE	_MSC_VER

#include "Semaphore.h"
#include "atomics.h"
#include "defines.h"
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
		mCtx = (size_t)new SemContext();
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

	Semaphore::Semaphore()
	{
		mCtx = (size_t)CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
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
		ReleaseSemaphore((HANDLE)mCtx, count, 0);
	}

	void Semaphore::wait()
	{
		WaitForSingleObject((HANDLE)mCtx, INFINITE);
	}

#endif

	Semaphore::Semaphore(Semaphore&& o) : mCtx(NULL)
	{
		*this = ks::move(o);
	}

	Semaphore& Semaphore::operator = (Semaphore&& o)
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
	Event::Event(bool state) : mOpen(state), mListeners(0)
	{}

	void Event::SetState(bool state)
	{
		atomic_or(&mOpen, state);
	}

	void Event::Notify()
	{
		atomic_and(&mOpen, false);
		unsigned count = atomic_or(&mListeners, 0);
		mSem.signal(count);
	}

	void Event::Wait()
	{
		atomic_increment(&mListeners);
		if (mOpen)
			mSem.wait();
		atomic_decrement(&mListeners);
	}
}
