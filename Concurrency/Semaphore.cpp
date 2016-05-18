
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
		mCtx = (size_t)new SemContext();
	}

	Semaphore::~Semaphore()
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
		mCtx = (size_t)CreateSemaphore(NULL, 0, 0x0fffffff, NULL);
	}

	Semaphore::~Semaphore()
	{
		CloseHandle((HANDLE)mCtx);
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

}
