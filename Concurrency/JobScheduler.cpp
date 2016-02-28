
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

#include <Concurrency\JobScheduler.h>
#include <Containers\CyclicConcurrentQueue.h>
#include <thread>

namespace ks {

#define	JS_FLAG_RUNNING			(1<<1)
#define	JS_FLAG_SINGLEPRODUCER	(1<<2)

	struct JSThread
	{
		JSThread(const JobScheduler* pArg1, CyclicConcurrentQueue<Job>* pArg2) : mThread{ ThreadRoutine, pArg1, pArg2 }
		{
			mThread.detach();		// gotta manage its lifetime ourselves
		}

		static void ThreadRoutine(const JobScheduler* context, CyclicConcurrentQueue<Job>* queue)
		{
			while (context->Running())
			{
				auto job = queue->dequeue();
				if (*job)
					job->Execute();
				else
					THREAD_SLEEP(1);	// TODO: semaphore up in this biz'niss
			}
		}


		std::thread mThread;
	};


	JobScheduler::JobScheduler(ksU32 pNumThreads, ksU32 pMaxNumJobs, bool pSingleProducer )
		: mJobQueue(nullptr), mWorkerThreads(pNumThreads), mFlags(0)
	{
		if (pSingleProducer)
			mFlags |= JS_FLAG_SINGLEPRODUCER;

		mFlags |= JS_FLAG_RUNNING;

		mJobQueue		= new CyclicConcurrentQueue<Job>(pMaxNumJobs);

		for (ksU32 i = 0; i < pNumThreads; ++i)
			mWorkerThreads.push_back(new JSThread(this, mJobQueue));
	}

	JobScheduler::~JobScheduler()
	{
		mFlags = 0;
		THREAD_SLEEP(3);				// wait for the threads routines to quit. TODO: don't judge me.

		for (auto i : mWorkerThreads)
			delete i;

		delete mJobQueue;
	}


	JobHandle JobScheduler::QueueJob(Job&& pJob)
	{
		Job* handle = SingleProducerMode() ? mJobQueue->enqueue_singlethreaded(ks::move(pJob)) : mJobQueue->enqueue(ks::move(pJob));
		return JobHandle(handle);
	}

	bool JobScheduler::Running() const				{ return (mFlags & JS_FLAG_RUNNING) > 0; }

	bool JobScheduler::SingleProducerMode() const	{ return (mFlags & JS_FLAG_SINGLEPRODUCER) > 0; }
}