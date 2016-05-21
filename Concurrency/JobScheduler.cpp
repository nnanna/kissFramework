
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
#include <Concurrency\Semaphore.h>
#include <Containers\CyclicConcurrentQueue.h>
#include <thread>

namespace ks {


#define	JS_FLAG_RUNNING			(1<<1)
#define	JS_FLAG_SINGLEPRODUCER	(1<<2)

	struct JSEventHandle
	{
		JSEventHandle() : mJobID(0)
		{}

		void Start(u32 id)
		{
			atomic_set(&mJobID, id);
			mEvent.SetState(true);
		}

		void Stop()
		{
			atomic_and(&mJobID, 0);
			mEvent.Notify();
		}

		ksU32 JobID() const		{ return atomic_or(&mJobID, 0); }

		bool Wait(ksU32 pJobID)
		{
			if (pJobID == JobID())
			{
				mEvent.Wait();
				return true;
			}

			return false;
		}

		ksU32	mJobID;
		Event	mEvent;
	};

	struct JSThread
	{
		JSThread(JobScheduler* pArg1, CyclicConcurrentQueue<Job>* pArg2, ksU32 worker_index)
			: mThread{ ThreadRoutine, pArg1, pArg2, worker_index }
		{
			mThread.detach();		// gotta manage its lifetime ourselves
		}

		static void ThreadRoutine(JobScheduler* context, CyclicConcurrentQueue<Job>* queue, const ksU32 worker_index)
		{
			JSEventHandle* jsevent = context->mCompletionEvents[worker_index];
			while (context->Running())
			{
				auto job = queue->dequeue();
				if (*job)
				{
					jsevent->Start(job->UID());

					job->Execute();

					jsevent->Stop();
				}
				else
				{
					context->Wait();
				}
			}
		}


		std::thread mThread;
	};


	JobScheduler::JobScheduler(ksU32 pNumThreads, ksU32 pMaxNumJobs, bool pSingleProducer )
		: mJobQueue(nullptr), mWorkerThreads(pNumThreads), mCompletionEvents(pNumThreads), mFlags(0), mSemaphore(nullptr)
	{
		if (pSingleProducer)
			mFlags |= JS_FLAG_SINGLEPRODUCER;

		mFlags |= JS_FLAG_RUNNING;

		mJobQueue		= new CyclicConcurrentQueue<Job>(pMaxNumJobs);
		mSemaphore		= new Semaphore();

		for (ksU32 i = 0; i < pNumThreads; ++i)
		{
			mCompletionEvents.push_back(new JSEventHandle());
			mWorkerThreads.push_back(new JSThread(this, mJobQueue, i));
		}
	}

	JobScheduler::~JobScheduler()
	{
		mFlags = 0;
		for (u32 n = 0; n < mWorkerThreads.size(); ++n)
			Signal();

		THREAD_SLEEP(30);		// @TODO: don't use glut - it makes you do bad things

		for (auto i : mWorkerThreads)
			delete i;

		delete mSemaphore;
		delete mJobQueue;
	}


	JobHandle JobScheduler::QueueJob(Job&& pJob)
	{
		Job* handle = SingleProducerMode() ? mJobQueue->enqueue_singlethreaded(ks::move(pJob)) : mJobQueue->enqueue(ks::move(pJob));
		Signal();
		return JobHandle(handle);
	}

	bool JobScheduler::Running() const				{ return (mFlags & JS_FLAG_RUNNING) > 0; }

	bool JobScheduler::SingleProducerMode() const	{ return (mFlags & JS_FLAG_SINGLEPRODUCER) > 0; }


	void JobScheduler::Wait(const ksU32 pJobID)
	{
		// between a job being poppped off the queue and signaling the job event Start()
		// there's a small window during which a wait could be issued on that job
		// resulting in a false completion status since it's neither WAITING nor currently RUNNING
		// I'm temporarily alleviating this by 'spinning' a few times before concluding that the job is completed
		// 'temporarily' == till my brain gets smart enough to figure out a super cool fix
		bool found = false;
		ksU32 num_spins = 0;
		static ksU32 MAX_SPIN_CHECK(5);
		while (found == false && num_spins++ < MAX_SPIN_CHECK)
		{
			for (ksU32 i = 0; i < mCompletionEvents.size(); ++i)
			{
				if (mCompletionEvents[i]->JobID() == pJobID)
				{
					found = mCompletionEvents[i]->Wait(pJobID);
					break;
				}
			}

			if(!found)	THREAD_SWITCH;
		}
	}


	void JobScheduler::Signal()
	{
		mSemaphore->signal();
	}

	void JobScheduler::Wait()
	{
		mSemaphore->wait();
	}
}