

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

		struct SemContext*	mCtx;
	};
}

#endif