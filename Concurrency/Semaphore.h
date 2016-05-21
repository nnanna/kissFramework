

#ifndef KS_SEMAPHORE_H
#define KS_SEMAPHORE_H

namespace ks {

	class Semaphore
	{
	public:

		Semaphore();

		~Semaphore();

		Semaphore(Semaphore&& o);
		Semaphore& operator=(Semaphore&& o);

		void signal(int count = 1);

		void wait();

	private:
		void destroy();
		Semaphore(const Semaphore&);
		Semaphore& operator=(const Semaphore&);

		size_t	mCtx;
	};


	class Event
	{
	public:
		Event(bool state = true);
		void SetState(bool state);
		void Notify();
		void Wait();

	private:
		unsigned	mState;
		Semaphore	mSem;
	};
}

#endif