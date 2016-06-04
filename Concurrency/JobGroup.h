
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

	class JobGroup
	{
	public:
		template<ksU32 CAPACITY>
		static JobGroup* create()
		{
			static_assert( (CAPACITY & (CAPACITY - 1)) == 0, "Capacity must be power of two.");
			return new JobGroup(CAPACITY);
		}

		static void destroy(JobGroup* pGroup);

		template<typename _FN>
		ksU32 Add(_FN&& pFunctor, const char* pName)
		{
			const ksU32 id = atomic_increment(&mNumJobs) - 1;

			return Add( Job(ks::move(pFunctor), [this, id](ksU32) { onCompleted(id); }, pName), id );
		}

		// partially synchronise up to pJobIndex
		void Sync(const ksU32 pJobIndex);

		void Sync();

	private:
		JobGroup(ksU32 pCapacity);
		~JobGroup();

		ksU32 Add(Job&& pFunctor, ksU32 index);

		void onCompleted(ksU32 index);

		const ksU32		mCapacityMinusOne;		// i know. it's ridiculous. now leave me be.
		ksU32			mNumJobs;
		char*			mAvailable;
		JobHandle*		mJobHandles;
		Event*			mBusy;

	};
}


#endif