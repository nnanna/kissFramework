

#include "Semaphore.h"

namespace ks {


	Semaphore::Semaphore() : mCount(0)
	{}

	void Semaphore::signal()
	{
		std::unique_lock<std::mutex> lock(mMtx);
		++mCount;
		lock.unlock();
		cv.notify_one();
	}

	void Semaphore::wait()
	{
		std::unique_lock<std::mutex> lock(mMtx);
		cv.wait(lock, [this]() { return mCount != 0; });
		--mCount;
	}

	void Semaphore::finish()
	{
		std::unique_lock<std::mutex> lock(mMtx);
		mCount = 0xffffffff;
		lock.unlock();
		cv.notify_all();
	}

	unsigned int Semaphore::trywait() const
	{
		return mCount;
	}

}
