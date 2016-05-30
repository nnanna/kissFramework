/////////////////////////////////////////////////////////////////
//	CyclicConcurrentQueue.h
/*
Copyright (C) 2015 Nnanna Kama

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/////////////////////////////////////////////////////////////////

#pragma once

#ifndef CYCLIC_CONCURRENT_QUEUE
#define CYCLIC_CONCURRENT_QUEUE

#include <defines.h>
#include <Concurrency\atomics.h>

namespace ks
{
	template<bool isPowerOfTwo> ksU32 pseudo_modulus(ksU32 a, ksU32 b);

	template<> inline ksU32 pseudo_modulus<true>(ksU32 a, ksU32 b)	{ return a & b; }
	template<> inline ksU32 pseudo_modulus<false>(ksU32 a, ksU32 b)	{ return a % (b + 1); }

	template<bool isSingleThreaded>
	bool fail_compare_swap(ksU32& ptr, ksU32 oldVal, ksU32 newVal);		// double negative

	template<>
	inline bool fail_compare_swap<true>(ksU32& ptr, ksU32 oldVal, ksU32 newVal)	{ ptr = newVal; return false; }

	template<>
	inline bool fail_compare_swap<false>(ksU32& ptr, ksU32 oldVal, ksU32 newVal)
	{
		// first test (ptr != oldVal) to avoid a futile compare_swap call.
		return atomic_compare_and_swap(&ptr, oldVal, newVal) != oldVal;
	}


	template<typename T>
	class CyclicConcurrentQueue
	{
#define TAILHEAD_DIVIDER	1	// helps prevent looparound so only head can catch up to tail and never vice-versa
	public:

		enum EnqueueException
		{
			eQueueFull
		};

		enum CCQMode
		{
			ccq_multi = 0,
			ccq_single
		};

		struct queue_item
		{
			queue_item() : valid(false)
			{}

			queue_item(T&& pItem, bool pValid) : valid(pValid)
			{
				if (valid)	data	= ks::move(pItem);
			}

			queue_item(queue_item&& o) : valid(o.valid)
			{
				if (valid)	data	= ks::move(o.data);
				o.valid				= false;
			}

			void operator=(queue_item&& o)
			{
				valid = o.valid;
				if (valid && this != &o)
				{
					data	= ks::move(o.data);
					o.valid = false;
				}
			}

			bool operator*() const		{ return valid; }
			T* operator->()				{ return &data; }
			const T* operator->() const	{ return &data; }
			operator T&()				{ return data; }
			operator const T&() const	{ return data; }

			T 		data;
			bool	valid;
		private:
			queue_item(const queue_item&);
			queue_item& operator=(const queue_item&);
		};

		CyclicConcurrentQueue(const ksU32 pCapacity) :
			mCapacity(pCapacity),
			mIsPowerOfTwo(((pCapacity + 1) & pCapacity) == 0),
			mReadHead(0),
			mWriteHead(0),
			mReadTail(0),
			mWriteTail(0)
		{
			mItems = new T[mCapacity + TAILHEAD_DIVIDER];
		}

		CyclicConcurrentQueue(const CyclicConcurrentQueue& pOther) = delete;

		~CyclicConcurrentQueue()
		{
			delete[] mItems;
		}

		T* const enqueue(T&& pItem)	{ return mIsPowerOfTwo ? enqueue<ccq_multi, true>(ks::move(pItem)) : enqueue<ccq_multi, false>(ks::move(pItem)); }

		queue_item	dequeue()		{ return mIsPowerOfTwo ? dequeue<ccq_multi, true>() : dequeue<ccq_multi, false>(); }

		void dequeue(queue_item& q)	{ return mIsPowerOfTwo ? dequeue<ccq_multi, true>(q) : dequeue<ccq_multi, false>(q); }

		T* const enqueue_singlethreaded(T&& pItem)
		{
			return mIsPowerOfTwo ? enqueue<ccq_single, true>(ks::move(pItem)) : enqueue<ccq_single, false>(ks::move(pItem));
		}

		queue_item	dequeue_singlethreaded()
		{
			return mIsPowerOfTwo ? dequeue<ccq_single, true>() : dequeue<ccq_single, false>();
		}


		inline bool full() const		{ return next<false>(mWriteTail) == mWriteHead; }
		inline bool empty() const		{ return mReadHead == mReadTail; }
		inline void clear()				{ mReadHead = mWriteHead = mReadTail = mWriteTail = 0; }

		ksU32 size() const
		{
			const ksU32 currentTail = mReadTail;
			const ksU32 currentHead = mReadHead;
			if (currentTail < currentHead)
			{
				return (mCapacity - currentHead) + currentTail;
			}
			else
			{
				return currentTail - currentHead;
			}
		}

	private:
		template<bool isPowerOfTwo>
		inline ksU32 next(ksU32 pFrom) const	{ return pseudo_modulus<isPowerOfTwo>(pFrom + 1, mCapacity); }


		template<CCQMode mode, bool isPowerOfTwo>
		T* const enqueue(T&& pItem)
		{
			ksU32 index(0), nextTail(0), spin_count(0);
			bool failed(true);
			do
			{
				conditional_yield(spin_count);
				index		= mWriteTail;
				nextTail	= next<isPowerOfTwo>(index);
#if KSCQ_ENABLE_QUEUE_FULL_EXCEPTION
			} while ((nextTail != mWriteHead) && (failed = fail_compare_swap<mode>(mWriteTail, index, nextTail)));
#else
			} while ((nextTail == mWriteHead) || (failed = fail_compare_swap<mode>(mWriteTail, index, nextTail)));
#endif

			if (!failed)
			{
				mItems[index] = ks::move(pItem);
				//WRITE_BARRIER;

				spin_count = 0;
				// first test (mReadTail != index) to avoid a futile compare_swap call.
				while ((mReadTail != index) || fail_compare_swap<mode>(mReadTail, index, nextTail))
				{
					conditional_yield(spin_count);
				}

				return mItems + index;
			}
			else
			{
				throw EnqueueException::eQueueFull;
				return nullptr;
			}
		}

		template<CCQMode mode, bool isPowerOfTwo>
		void dequeue(queue_item& qitem)
		{
			ksU32 index(0), nextHead(0), spin_count(0);
			bool available(false);
			do
			{
				index = mReadHead;
				nextHead = next<isPowerOfTwo>(index);
				available = !empty();
			} while (available && ks::fail_compare_swap<mode>(mReadHead, index, nextHead) && conditional_yield(spin_count));

			qitem	= queue_item(ks::move(mItems[index]), available);

			while (available && ks::fail_compare_swap<mode>(mWriteHead, index, nextHead))
			{
				ksYieldThread;
			}
		}

		template<CCQMode mode, bool isPowerOfTwo>
		queue_item dequeue()
		{
			queue_item qitem;
			dequeue<mode, isPowerOfTwo>(qitem);
			return qitem;
		}

		inline bool conditional_yield(ksU32& spin_count)
		{
			// http://www.codeproject.com/Articles/184046/Spin-Lock-in-C
			if (spin_count++)
			{
				if (spin_count < CONTEXT_SWITCH_LATENCY)
					ksYieldProcessor;
				else
					ksYieldThread;
			}
			return true;	// only returns bool so it can be used as a condition on loop 'headers'
		}

		// 'technically wasteful' padding/alignment to eliminate false sharing.
		// http://www.drdobbs.com/parallel/eliminate-false-sharing/217500206?pgno=4

		T*	mItems;
		char pad0[CACHE_LINE_SIZE - sizeof(T*)];
	
		ksU32		mReadHead;
		char pad1[ CACHE_LINE_SIZE - sizeof(ksU32) ];
	
		ksU32		mWriteHead;
		char pad2[ CACHE_LINE_SIZE - sizeof(ksU32) ];

		ksU32		mReadTail;
		char pad3[CACHE_LINE_SIZE - sizeof(ksU32)];

		ksU32		mWriteTail;
		char pad4[CACHE_LINE_SIZE - sizeof(ksU32)];

		const ksU32	mCapacity;
		const bool	mIsPowerOfTwo;

	public:
		// it appears that if your threads exceed your processors, it's better to call SwitchToThread() on windows
		// since we default to YieldProcessor, this value defines the number of attempts before doing this
		static ksU32 CONTEXT_SWITCH_LATENCY;
	};

	template<typename T>
	ksU32 CyclicConcurrentQueue<T>::CONTEXT_SWITCH_LATENCY = 2;

}

#endif	// CYCLIC_CONCURRENT_QUEUE
