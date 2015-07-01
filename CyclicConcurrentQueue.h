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

namespace ks
{
	template<typename T> struct strip_ref		{ typedef T type; };

	template<typename T> struct strip_ref<T&>	{ typedef T type; };
	template<typename T> struct strip_ref<T&&>	{ typedef T type; };

	template <typename T>
	typename strip_ref<T>::type&& move(T&& arg)	{ return static_cast<typename strip_ref<T>::type&&>(arg); }

	template<bool isMultipleOfTwo> size_t modulus( size_t a, size_t b );

	template<> 					size_t modulus<true>( size_t a, size_t b )		{ return a & (b - 1); }
	template<> 					size_t modulus<false>( size_t a, size_t b )		{ return a % b; }
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

	CyclicConcurrentQueue( const size_t pCapacity ) :
		mCapacity( pCapacity + TAILHEAD_DIVIDER ),
		misMultipleOfTwo( (mCapacity & ( mCapacity - 1 )) == 0 ),
		mHead(0),
		mReadTail(0),
		mWriteTail(0),
		mSize(0)
	{
		mItems	= new T [ mCapacity ];
	}

	CyclicConcurrentQueue( const CyclicConcurrentQueue& pOther ) = delete;

	~CyclicConcurrentQueue()	{ delete [] mItems; }

	bool enqueue( T&& pItem )	{ return misMultipleOfTwo ? enqueue<true>( ks::move(pItem) ) : enqueue<false>( ks::move(pItem) ); }

	queue_item	dequeue()		{ return misMultipleOfTwo ? dequeue<true>() : dequeue<false>(); }


	bool	full() const		{ return next<false>(mWriteTail) == mHead; }
	bool	empty() const		{ return mHead == mWriteTail; }
	size_t	size() const		{ return mSize; }

private:
	template<bool isMultipleOfTwo>
	size_t next( size_t pFrom ) const	{ return ks::modulus<isMultipleOfTwo>( pFrom + 1, mCapacity ); }


	template<bool isMultipleOfTwo>
	bool enqueue( T&& pItem )
	{
		bool available( false );
		size_t index(0), nextTail(0);
		do
		{
			index			= mWriteTail;
			nextTail		= next<isMultipleOfTwo>( index );
			available		= (nextTail != mHead) || (index != mWriteTail);
		}while( available && atomic_compare_and_swap( &mWriteTail, index, nextTail ) == nextTail );

		if( available )
		{
			mItems[ index ]	= ks::move( pItem );
			WRITE_BARRIER;
			while( atomic_compare_and_swap( &mReadTail, index, nextTail ) == nextTail )
			{
				THREAD_YIELD;
			}
			atomic_increment( &mSize );
		}
		else
		{
			throw EnqueueException::eQueueFull;
		}

		return available;
	}

	template<bool isMultipleOfTwo>
	queue_item dequeue()
	{
		size_t index(0), head(0);
		bool is_valid( false );
		do
		{
			index		= mHead;
			head		= next<isMultipleOfTwo>( index );
			is_valid	= !empty();
		}while( is_valid && atomic_compare_and_swap( &mHead, index, head ) == head );

		if( is_valid )
		{
			atomic_decrement( &mSize );
		}

		return queue_item( mItems[ index ], is_valid );	// ideally you'd wanna grab the data before incrementing mHead, no?
		// Generally, this is fine™ as long as the queue doesn't fill up quicker than it is consumed - which is a problem in itself - it should be adequately sized
		// Options: use mWriteHead and mReadHead? Always copy the data in the do...while() (BAD)?
	}

	T*				mItems;
	const size_t	mCapacity;
	const bool		misMultipleOfTwo;
	size_t			mHead;
	size_t			mReadTail;
	size_t			mWriteTail;
	size_t			mSize;
};

#endif	// CYCLIC_CONCURRENT_QUEUE
