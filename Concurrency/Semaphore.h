

#ifndef KS_SEMAPHORE_H
#define KS_SEMAPHORE_H

namespace ks {

	class Semaphore
	{
	public:

		Semaphore();

		~Semaphore();

		Semaphore(Semaphore&& o);
		void operator=(Semaphore&& o);

		void signal(int count = 1);

		bool wait(unsigned timeoutMS = 0xffffffff);	// returns true if was signaled 

	private:
		void destroy();
		Semaphore(const Semaphore&);
		Semaphore& operator=(const Semaphore&);

		size_t	mCtx;
		int		mNumSignals;
	};


	class Event
	{
	public:
		Event(bool state = true);
		void Reset()				{ SetState(true); }
		void SetState(bool state);
		void Notify();
		void Wait(int timeoutMS = 0xffffffff);

	private:
		unsigned	mState;
		Semaphore	mSem;
	};
}

#endif