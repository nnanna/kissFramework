
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


#ifndef KS_JOB_GROUP
#define KS_JOB_GROUP

#include <Concurrency\Job.h>

namespace ks {


	class Event;

	//
	// This is actually more of a scheduler than JobScheduler (which is really a JobDispatcher). TODO.
	//
	class JobGroup
	{
	public:

		enum mode
		{
			JG_NONE						= (1 << 0),
			JG_NEEDS_DEFERRED_QUEUE		= (1 << 1),
		};

		template<ksU32 CAPACITY>
		static JobGroup* create( ksU32 pFlags = JG_NONE )
		{
			static_assert( (CAPACITY & (CAPACITY - 1)) == 0, "Capacity must be power of two.");
			return new JobGroup(CAPACITY, pFlags);
		}

		static void destroy(JobGroup* pGroup);

		template<typename _FN>
		ksU32 Add(_FN&& pFunctor, const char* pName)
		{
			const ksU32 id	= atomic_increment(&mJobIDs) - 1;
			return Add( Job(ks::move(pFunctor), [this, id](ksU32) { onCompleted(id); }, pName), id );
		}

		//
		// Add this job sequentially so it only starts after previous ones are completed
		// if pEndMargin is specified, the job's kicked when mNumJobs <= pEndMargin. Useful for specifying job priority (indirectly)
		//
		template<typename _FN>
		ksU32 QueueAtEnd(_FN&& pFunctor, const char* pName, ksU32 pEndMargin = 0)
		{
			const ksU32 id = atomic_increment(&mDeferredIDs) - 1;
			return QueueAtEnd(Job(ks::move(pFunctor), [this, id](ksU32) { onDeferredCompleted(id); }, pName), id, pEndMargin);
		}

		//
		// Start this job only after the job at pDependencyIndex is completed.
		//
		template<typename _FN>
		ksU32 QueueAfter(_FN&& pFunctor, const char* pName, ksU32 pDependencyIndex)
		{
			const ksU32 id = atomic_increment(&mDeferredIDs) - 1;
			return QueueAfter(Job(ks::move(pFunctor), [this, id](ksU32) { onDeferredCompleted(id); }, pName), id, pDependencyIndex);
		}

		//
		// partially synchronise up to pJobIndex
		//
		void Sync(const ksU32 pJobIndex);

		void Sync();

	private:
		JobGroup(ksU32 pCapacity, ksU32 pFlags);
		~JobGroup();

		ksU32 Add(Job&& pJob, ksU32 pID);
		ksU32 QueueAtEnd(Job&& pJob, ksU32 pID, ksU32 pEndMargin);
		ksU32 QueueAfter(Job&& pJob, ksU32 pID, const ksU32 pDependencyIndex);

		void onCompleted(ksU32 index);
		void onDeferredCompleted(ksU32 id);

		void addDeferred(Job&& pJob, ksU32 id, ksU32 pCondition, bool pConditionTestResult);

		const ksU32		mCapacityMinusOne;		// i know. it's ridiculous. now leave me be :P
		ksU32			mJobIDs;
		ksU32			mNumJobs;
		char*			mAvailable;
		JobHandle*		mJobHandles;

		ksU32			mDeferredIDs;
		ksU32			mNumDeferredJobs;
		Job*			mDeferredQueue;
		ksU32*			mDeferredConditions;
		Event*			mBusy;

	};
}


#endif