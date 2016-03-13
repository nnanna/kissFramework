

#ifndef KS_SEMAPHORE_H
#define KS_SEMAPHORE_H


#include <mutex>
#include <condition_variable>

namespace ks {

	class Semaphore
	{
	public:

		Semaphore();

		void signal();

		void wait();

		void finish();

		unsigned int trywait() const;

	private:
		unsigned int			mCount;
		std::mutex				mMtx;
		std::condition_variable cv;
	};
}

#endif