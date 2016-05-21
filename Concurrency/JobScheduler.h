
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		14/02/2016
///
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////


#ifndef KS_JOB_SCHEDULER
#define KS_JOB_SCHEDULER

#include <Concurrency\Job.h>
#include <Containers\Array.h>

namespace ks {


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FORWARD DECLS
	template<typename T>
	class CyclicConcurrentQueue;

	class Semaphore;
	struct JSThread;
	struct JSEventHandle;
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	class JobScheduler
	{
	public:
		JobScheduler( ksU32 pNumThreads = 4, ksU32 pMaxNumJobs = 31, bool pSingleProducer = false );
		~JobScheduler();

		template<typename _FN>
		JobHandle QueueJob(_FN&& pFunctor, const char* pName);

		template<typename _FN, typename _CT>
		JobHandle QueueJob(_FN&& pFunctor, _CT&& pOnCompletion, const char* pName);

		JobHandle QueueJob(Job&& pJob);

		bool Running() const;

		bool SingleProducerMode() const;

		void Wait(const ksU32 pJobID);

	private:
		friend JSThread;
		void Signal();

		void Wait();

		CyclicConcurrentQueue<Job>*	mJobQueue;
		Array<JSThread*>			mWorkerThreads;
		Array<JSEventHandle*>		mCompletionEvents;
		ksU32						mFlags;
		Semaphore*					mSemaphore;

	};
}


#endif