

#ifndef KS_SEMAPHORE_H
#define KS_SEMAPHORE_H

namespace ks {

	class Semaphore
	{
	public:

		Semaphore();

		~Semaphore();

		void signal(int count = 1);

		void wait();

	private:
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
		bool		mOpen;
		unsigned	mListeners;
		Semaphore	mSem;
	};
}

#endif