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

	~CyclicConcurrentQueue()	{ delete [] mItems; }

	void enqueue( T&& pItem )	{ mIsPowerOfTwo ? enqueue<true>( ks::move(pItem) ) : enqueue<false>( ks::move(pItem) ); }

	queue_item	dequeue()		{ return mIsPowerOfTwo ? dequeue<true>() : dequeue<false>(); }


	inline bool full() const		{ return next<false>(mWriteTail) == mHead; }
	inline bool empty() const		{ return mHead == mReadTail; }
	
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


	template<bool isPowerOfTwo>
	void enqueue( T&& pItem )
	{
		ksU32 index(0), nextTail(0), cas(1);
		do
		{
			index			= mWriteTail;
			nextTail		= next<isPowerOfTwo>( index );
		} while ((nextTail != mHead) && (cas = atomic_compare_and_swap(&mWriteTail, index, nextTail)) != index);

		if (cas == index)
		{
			mItems[ index ]	= ks::move( pItem );
			WRITE_BARRIER;
			while( atomic_compare_and_swap( &mReadTail, index, nextTail ) != index )
			{
				THREAD_YIELD;
			}
		}
		else
		{
			throw EnqueueException::eQueueFull;
		}
	}

	template<bool isPowerOfTwo>
	queue_item dequeue()
	{
		ksU32 index(0), nextHead(0), cas(1);
		do
		{
			index		= mHead;
			nextHead	= next<isPowerOfTwo>(index);
		} while (!empty() && (cas = atomic_compare_and_swap(&mHead, index, nextHead)) != index);

		return queue_item(mItems[index], (cas == index));	// ideally you'd wanna grab the data before incrementing mHead, no?
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
};

#endif	// CYCLIC_CONCURRENT_QUEUE
