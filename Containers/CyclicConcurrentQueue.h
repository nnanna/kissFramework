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

#include "atomics.h"

typedef unsigned ksU32;

namespace ks
{
	#ifndef CACHE_LINE_SIZE
	#define CACHE_LINE_SIZE 64
	#endif
	
	template<typename T> struct strip_ref		{ typedef T type; };

	template<typename T> struct strip_ref<T&>	{ typedef T type; };
	template<typename T> struct strip_ref<T&&>	{ typedef T type; };

	template <typename T>
	typename strip_ref<T>::type&& move(T&& arg)	{ return static_cast<typename strip_ref<T>::type&&>(arg); }

	template<bool isPowerOfTwo> ksU32 pseudo_modulus( ksU32 a, ksU32 b );

	template<> inline ksU32 pseudo_modulus<true>( ksU32 a, ksU32 b )	{ return a & b; }
	template<> inline ksU32 pseudo_modulus<false>( ksU32 a, ksU32 b )	{ return a % (b + 1); }

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
		queue_item( T& pItem, bool pValid ) : data( pItem ), valid( pValid )
		{}

		bool operator*() const		{ return valid; }
		T* operator->()				{ return &data; }
		const T* operator->() const	{ return &data; }
		operator T&()				{ return data; }
		operator const T&() const	{ return data; }

		T& 			data;
		const bool	valid;
	};

	CyclicConcurrentQueue( const ksU32 pCapacity ) :
		mCapacity( pCapacity ),
		mIsPowerOfTwo(((pCapacity + 1) & pCapacity) == 0),
		mHead(0),
		mReadTail(0),
		mWriteTail(0)
	{
		mItems	= new T [ mCapacity + TAILHEAD_DIVIDER ];
	}

	CyclicConcurrentQueue( const CyclicConcurrentQueue& pOther ) = delete;

	~CyclicConcurrentQueue()
	{
		delete [] mItems;
	}

	void enqueue(T&& pItem)	{ mIsPowerOfTwo ? enqueue<ccq_multi, true>(ks::move(pItem)) : enqueue<ccq_multi, false>(ks::move(pItem)); }

	queue_item	dequeue()	{ return mIsPowerOfTwo ? dequeue<ccq_multi, true>() : dequeue<ccq_multi, false>(); }

	void enqueue_singlethreaded(T&& pItem)
	{
		mIsPowerOfTwo ? enqueue<ccq_single, true>(ks::move(pItem)) : enqueue<ccq_single, false>(ks::move(pItem));
	}

	queue_item	dequeue_singlethreaded()
	{
		return mIsPowerOfTwo ? dequeue<ccq_single, true>() : dequeue<ccq_single, false>();
	}


	inline bool full() const		{ return next<false>(mWriteTail) == mHead; }
	inline bool empty() const		{ return mHead == mReadTail; }
	inline void clear()				{ mHead = mReadTail = mWriteTail = 0; }
	
	ksU32 size() const
	{
		const ksU32 currentTail = mReadTail;
		const ksU32 currentHead = mHead;
		if( currentTail < currentHead )
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
	inline ksU32 next(ksU32 pFrom) const	{ return ks::pseudo_modulus<isPowerOfTwo>(pFrom + 1, mCapacity); }


	template<CCQMode mode, bool isPowerOfTwo>
	void enqueue( T&& pItem )
	{
		ksU32 index(0), nextTail(0);
		bool failed(true);
		do
		{
			index			= mWriteTail;
			nextTail		= next<isPowerOfTwo>( index );
		} while ((nextTail != mHead) && (failed = ks::fail_compare_swap<mode>(mWriteTail, index, nextTail)) );

		if (!failed)
		{
			mItems[ index ]	= ks::move( pItem );
			//WRITE_BARRIER;

			ksU32 timeout(0);
			// first test (mReadTail != index) to avoid a futile compare_swap call.
			while ((mReadTail != index) || ks::fail_compare_swap<mode>(mReadTail, index, nextTail))
			{
				// http://www.codeproject.com/Articles/184046/Spin-Lock-in-C
				if (timeout < CONTEXT_SWITCH_LATENCY)
					THREAD_YIELD;
				else
					THREAD_SWITCH;
				++timeout;
			}
		}
		else
		{
			throw EnqueueException::eQueueFull;
		}
	}

	template<CCQMode mode, bool isPowerOfTwo>
	queue_item dequeue()
	{
		ksU32 index(0), nextHead(0);
		bool available(false);
		do
		{
			index		= mHead;
			nextHead	= next<isPowerOfTwo>(index);
			available	= !empty();
		} while (available && ks::fail_compare_swap<mode>(mHead, index, nextHead));

		return queue_item(mItems[index], available);	// ideally you'd wanna grab the data before incrementing mHead, no?
		// Generally, this is fine™ as long as the queue doesn't fill up quicker than it is consumed - which is a problem in itself - it should be adequately sized
		// Options: use mWriteHead and mReadHead? Always copy the data in the do...while() (BAD)?
	}
	
	// 'technically wasteful' padding/alignment to eliminate false sharing.
	// http://www.drdobbs.com/parallel/eliminate-false-sharing/217500206?pgno=4
	
	__declspec(align(CACHE_LINE_SIZE)) T*	mItems;
	char pad0[ CACHE_LINE_SIZE - sizeof(T*) ];
	
	ksU32		mHead;
	char pad1[ CACHE_LINE_SIZE - sizeof(ksU32) ];
	
	ksU32		mReadTail;
	char pad2[ CACHE_LINE_SIZE - sizeof(ksU32) ];
	
	ksU32		mWriteTail;
	char pad3[ CACHE_LINE_SIZE - sizeof(ksU32) ];
	
	const ksU32	mCapacity;
	const bool	mIsPowerOfTwo;

public:
	// it appears that if your threads exceed your processors, it's better to call SwitchToThread() on windows
	// since we default to YieldProcessor, this value defines the number of attempts before doing this
	static ksU32 CONTEXT_SWITCH_LATENCY;
};

template<typename T>
ksU32 CyclicConcurrentQueue<T>::CONTEXT_SWITCH_LATENCY = 2;

#endif	// CYCLIC_CONCURRENT_QUEUE
