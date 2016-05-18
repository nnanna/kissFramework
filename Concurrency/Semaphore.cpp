
#define KERNEL_SEMAPHORE	_MSC_VER

#include "Semaphore.h"
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
		mCtx = new SemContext();
	}

	Semaphore::~Semaphore()
	{}

	void Semaphore::signal(int count /*= 1*/)
	{
		std::unique_lock<std::mutex> lock(mCtx->mtx);
		++mCtx->mCount;
		lock.unlock();
		mCtx->cv.notify_one();	// TODO: notify count
	}

	void Semaphore::wait()
	{
		std::unique_lock<std::mutex> lock(mCtx->mtx);
		mCtx->cv.wait(lock, [this]() { return mCtx->mCount != 0; });
		--mCtx->mCount;
	}

#else

	struct SemContext
	{
		SemContext()
		{
			mSemaphore = CreateSemaphore(NULL, 0, 0x0fffffff, NULL);
		}
		HANDLE	mSemaphore;
	};


	Semaphore::Semaphore()
	{
		mCtx = new SemContext();
	}

	Semaphore::~Semaphore()
	{
		CloseHandle(mCtx->mSemaphore);
	}

	void Semaphore::signal(int count /*= 1*/)
	{
		ReleaseSemaphore(mCtx->mSemaphore, count, 0);
	}

	void Semaphore::wait()
	{
		WaitForSingleObject(mCtx->mSemaphore, INFINITE);
	}

#endif

}
