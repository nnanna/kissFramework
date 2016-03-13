//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		14/02/2016
/// @brief:		template implementations for .cpp files only, to cut down compile overhead
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

namespace ks {

	template<typename _FN>
	JobHandle JobScheduler::QueueJob( _FN&& pFunctor, const char* pName )
	{
		Job* handle = SingleProducerMode() ? mJobQueue->enqueue_singlethreaded(Job(ks::move(pFunctor), pName))
											: mJobQueue->enqueue(Job(ks::move(pFunctor), pName));

		Signal();

		return JobHandle(handle);
	}


	template<typename _FN, typename _CT>
	JobHandle JobScheduler::QueueJob(_FN&& pFunctor, _CT&& pOnCompletion, const char* pName)
	{
		Job* handle = SingleProducerMode() ? mJobQueue->enqueue_singlethreaded(Job(ks::move(pFunctor), ks::move(pOnCompletion), pName))
			: mJobQueue->enqueue(Job(ks::move(pFunctor), ks::move(pOnCompletion), pName));

		Signal();

		return JobHandle(handle);
	}
}