
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

#define JGI_AVAILL	0xf

	void JobGroup::destroy(JobGroup* pGroup)
	{
		delete pGroup;
	}

	ksU32 JobGroup::Add(Job&& pJob, ksU32 index)
	{
		if (index == 0)
			mBusy->SetState(true);

		index = index & mCapacityMinusOne;
		while (mAvailable[index] != JGI_AVAILL)
		{
			KS_ASSERT(!"Group full");
			ksSleepMilli(1);
		}
		mAvailable[index]	= 0x0;
		mJobHandles[index]	= Service<JobScheduler>::Get()->QueueJob(ks::move(pJob));

		return index;
	}

	void JobGroup::onCompleted(ksU32 index)
	{
		index				= index & mCapacityMinusOne;
		mAvailable[index]	= JGI_AVAILL;
		if (atomic_decrement(&mNumJobs) == 0)
			mBusy->Notify();
	}

	void JobGroup::Sync(const ksU32 pJobIndex)
	{
		for (ksU32 i = 0; i <= pJobIndex; ++i)		// first kick-off pending jobs
		{
			if (mAvailable[i] != JGI_AVAILL && ( i == pJobIndex || mJobHandles[i].IsValid() ) )
				mJobHandles[i].Sync();
		}

		for (ksU32 i = 0; i <= pJobIndex; ++i)		// now wait on unfinished ones
			mJobHandles[i].Sync();
	}

	void JobGroup::Sync()
	{
		for (ksU32 i = 0; i <= mCapacityMinusOne; ++i)
		{
			if (mAvailable[i] != JGI_AVAILL && mJobHandles[i].IsValid())
				mJobHandles[i].Sync();
		}
		mBusy->Wait();
	}


	JobGroup::JobGroup(ksU32 pCapacity) : mCapacityMinusOne(pCapacity - 1), mNumJobs(0)
	{
		mBusy		= new Event(false);
		mAvailable	= new char[pCapacity];
		mJobHandles = new JobHandle[pCapacity];

		memset(mAvailable, JGI_AVAILL, pCapacity);
	}

	JobGroup::~JobGroup()
	{
		Sync();
		delete mBusy;
		delete [] mJobHandles;
		delete [] mAvailable;
	}
}