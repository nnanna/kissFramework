
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		04/06/2016
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

#include "Service.h"
#include <Concurrency\JobGroup.h>
#include <Concurrency\Semaphore.h>
#include <Concurrency\JobScheduler.h>

namespace ks {

#define JGI_AVAIL				0xf
#define JG_COND_AVAIL			0x0f0f0f0f
#define JG_COND_LOCK			0x0f000000
#define JG_COND_MASK			0xff000000						// the first 8 bits of mDeferredConditions is reserved for tagging & locking
#define JGI_TO_RANGE(x)			(x) &= mCapacityMinusOne
#define JGI_DEFERRED_TAG(x)		(x) | (1 << 31)
#define JGI_GET_DEFERRED_TAG(x) ( (x) & (1 << 31) )

#define JGD_ENDMARGIN_BIT			(1 << 30)
#define JGD_DEPENDENCY_BIT			(1 << 31)
#define JGD_DEPENDENCY_TAG(x)		(x) | JGD_DEPENDENCY_BIT
#define JGD_ENDMARGIN_TAG(x)		(x) | JGD_ENDMARGIN_BIT
#define JGD_GET_DEPENDENCY_TAG(x)	( ( (x) & JGD_DEPENDENCY_BIT ) >> 31 )
#define JGD_GET_ENDMARGIN_TAG(x)	( ( (x) & JGD_ENDMARGIN_BIT ) >> 30 )

	void JobGroup::destroy(JobGroup* pGroup)
	{
		delete pGroup;
	}

	ksU32 JobGroup::Add(Job&& pJob, ksU32 index)
	{
		if (atomic_increment(&mNumJobs) == 1)
			mBusy->SetState(true);

		JGI_TO_RANGE(index);
		while (mAvailable[index] != JGI_AVAIL)
		{
			KS_ASSERT(!"Group full");
			ksSleepMilli(1);
		}
		mAvailable[index]	= 0x0;
		mJobHandles[index]	= Service<JobScheduler>::Get()->QueueJob(ks::move(pJob));

		return index;
	}

	ksU32 JobGroup::QueueAtEnd(Job&& pJob, ksU32 pID, ksU32 pEndMargin)
	{
		JGI_TO_RANGE(pID);
		addDeferred(ks::move(pJob), pID, JGD_ENDMARGIN_TAG(pEndMargin), mNumJobs <= pEndMargin);
		return JGI_DEFERRED_TAG(pID);
	}

	ksU32 JobGroup::QueueAfter(Job&& pJob, ksU32 pID, const ksU32 pDependencyIndex)
	{
		JGI_TO_RANGE(pID);
		addDeferred( ks::move(pJob), pID, JGD_DEPENDENCY_TAG(pDependencyIndex), mAvailable[pDependencyIndex] == JGI_AVAIL );
		return JGI_DEFERRED_TAG(pID);
	}

	void JobGroup::addDeferred(Job&& pJob, ksU32 pID, ksU32 pCondition, bool pConditionTestResult)
	{
		if (mDeferredQueue == nullptr)
			throw mode::JG_NEEDS_DEFERRED_QUEUE;	// ensure JobGroup is initialised with the right flag(s)

		if (pConditionTestResult)
		{
			while (atomic_compare_and_swap(mDeferredConditions + pID, JG_COND_AVAIL, JG_COND_LOCK) != JG_COND_AVAIL)
			{
				KS_ASSERT(!"overflow, possibly. try increasing group capacity");
				ksSleepMilli(1);
			}
			ksU32 index = atomic_increment(&mJobIDs) - 1;
			index		= Add(ks::move(pJob), index);
			ksU32 prev	= atomic_compare_and_swap(mDeferredConditions + pID, JG_COND_LOCK, index);
			KS_ASSERT(prev == JG_COND_LOCK /*|| prev == JG_COND_AVAIL*/);	// it can actually complete the job before this function exits!
			return;
		}

		while (atomic_compare_and_swap(mDeferredConditions + pID, JG_COND_AVAIL, JG_COND_LOCK) != JG_COND_AVAIL)
		{
			KS_ASSERT(!"Deferred queue full");
			ksSleepMilli(1);
		}
		atomic_increment(&mNumDeferredJobs);
		mDeferredQueue[pID]			= ks::move(pJob);
		ksU32 prev					= atomic_set(mDeferredConditions + pID, pCondition);
		KS_ASSERT(prev == JG_COND_LOCK);
	}


	void JobGroup::onCompleted(ksU32 index)
	{
		JGI_TO_RANGE(index);
		mAvailable[index]	= JGI_AVAIL;


		// check Deferred queue for pending conditions
		if (mNumDeferredJobs > 0)
		{
#define JGDC_ACQUIRE(j)		atomic_compare_and_swap( mDeferredConditions + j, cond, JG_COND_LOCK ) == cond
			const ksU32 searchCount = mNumDeferredJobs;
			for (ksU32 i = 0, numFound = 0; i <= mCapacityMinusOne && numFound < searchCount; ++i)
			{
				bool passed(false);
				ksU32 cond = mDeferredConditions[i];
				if ( JGD_GET_DEPENDENCY_TAG(cond) )
				{
					cond	= cond & ~JGD_DEPENDENCY_BIT;
					passed	= mAvailable[cond] == JGI_AVAIL;
					++numFound;
				}
				else if ( JGD_GET_ENDMARGIN_TAG(cond) )
				{
					ksU32 margin	= cond & ~JGD_ENDMARGIN_BIT;
					passed			= margin >= mNumJobs;		// TODO: can falsely pass multiple conditions if they all evaluate mNumJobs before it's incremented in Add(). track with additional variable
					++numFound;
				}

				if (passed && JGDC_ACQUIRE(i))
				{
					int count = atomic_decrement(&mNumDeferredJobs);
					KS_ASSERT(count >= 0);
					const ksU32 id = atomic_increment(&mJobIDs) - 1;
					index = Add(ks::move(mDeferredQueue[i]), id);
					ksU32 prev = atomic_compare_and_swap(mDeferredConditions + i, JG_COND_LOCK, index);
					KS_ASSERT(prev == JG_COND_LOCK);
				}
			}
		}

		if (atomic_decrement(&mNumJobs) == 0)
		{
			mBusy->Notify();
			//atomic_set(&mJobIDs, 0);	// why set it back to zero?!
		}
	}

	void JobGroup::onDeferredCompleted(ksU32 id)
	{
		JGI_TO_RANGE(id);

		ksU32 index = atomic_or(mDeferredConditions + id, 0);
		while ((index & JG_COND_MASK) != 0)		// this probably completed before all of it's post-queue logic is done. TODO
		{
			ksYieldThread;
			index = atomic_or(mDeferredConditions + id, 0);
		}

		index = atomic_set(mDeferredConditions + id, JG_COND_AVAIL);
		onCompleted(index);
	}

	void JobGroup::Sync(const ksU32 pJobIndex)
	{
		if (JGI_GET_DEFERRED_TAG(pJobIndex))
		{
			KS_ASSERT(!"deferred jobs currently unsupported");
			return;
		}

		for (ksU32 i = 0; i <= pJobIndex; ++i)		// first kick-off waiting jobs
		{
			if (mAvailable[i] != JGI_AVAIL )
				mJobHandles[i].StealExecute();
		}

		for (ksU32 i = 0; i <= pJobIndex; ++i)		// now wait on unfinished ones
			mJobHandles[i].Sync();
	}

	void JobGroup::Sync()
	{
		for (ksU32 i = 0; i <= mCapacityMinusOne; ++i)
		{
			if (mAvailable[i] != JGI_AVAIL && mJobHandles[i].IsValid())
				mJobHandles[i].Sync();
		}
		mBusy->Wait();
	}


	JobGroup::JobGroup(ksU32 pCapacity, ksU32 pFlags) 
		: mCapacityMinusOne(pCapacity - 1)
		, mNumJobs(0)
		, mJobIDs(0)
		, mDeferredIDs(0)
		, mNumDeferredJobs(0)
		, mDeferredQueue(nullptr)
		, mDeferredConditions(nullptr)
	{
		mBusy		= new Event(false);
		mAvailable	= new char[pCapacity];
		mJobHandles = new JobHandle[pCapacity];

		memset(mAvailable, JGI_AVAIL, pCapacity);

		if (pFlags & JG_NEEDS_DEFERRED_QUEUE)
		{
			mDeferredQueue		= new Job[pCapacity];
			mDeferredConditions = new ksU32[pCapacity];
			memset(mDeferredConditions, JGI_AVAIL, pCapacity * sizeof(ksU32));
		}
	}

	JobGroup::~JobGroup()
	{
		Sync();
		delete mBusy;
		delete [] mJobHandles;
		delete [] mAvailable;
		delete [] mDeferredQueue;
		delete [] mDeferredConditions;
	}
}