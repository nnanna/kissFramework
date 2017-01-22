/////////////////////////////////////////////////////////////////
//	CyclicQueue.hpp
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


#ifndef CYCLIC_QUEUE_HPP
#define CYCLIC_QUEUE_HPP

#include "CyclicQueue.h"
#include <Concurrency\atomics.h>

namespace ks {


	template<typename T>
	CyclicQueue<T>::CyclicQueue( const ksU32 pCapacity ) : mReadHead(0), mWriteHead(0), mTail(0), mItems( nullptr ), mCapacity( pCapacity + cq_caret_buffer )
	{
		mItems		= new T[ mCapacity ];
		mWriteHead	= new char[ mCapacity ];
		memset( mWriteHead, 0, mCapacity );
	}

	template<typename T>
	CyclicQueue<T>::~CyclicQueue()
	{
		delete [] mItems;
		delete [] mWriteHead;
	}

	template<typename T>
	typename CyclicQueue<T>::queue_item CyclicQueue<T>::dequeue()
	{
#if 0
		EnterCriticalSection( &mPopCS );
		bool isValid = mReadHead != mTail;
		queue_item<T> qitem( std::move(mItems[ mReadHead ]), isValid );

		if( isValid )
		{
			mReadHead	= next(mReadHead);
			mWriteHead[ mReadHead ] = 1;
		}
		LeaveCriticalSection( &mPopCS );

#else
		bool isValid(false);
		ksU32 index(0), nextHead(0);
		do
		{
			index		= mReadHead;
			nextHead	= next( index );
			isValid		= (index != (mTail % mCapacity)); //(mWriteHead[ index ] != 0);	//
		} while ( isValid && atomic_compare_and_swap( &mReadHead, index, nextHead ) != index );	// acquire index

		while (isValid && mWriteHead[index] != 2)
		{
			YieldProcessor();
		}

		queue_item qitem( std::move(mItems[ index ]), isValid );													// grab data

		WRITE_BARRIER;
		if(isValid)	mWriteHead[ index ]	= 0;																		// release index
#endif
		return qitem;
	}


	template<typename T>
	const T* const CyclicQueue<T>::enqueue(T&& pVal)
	{
		T* handle(nullptr);

		ksU32 tail	= atomic_increment(&mTail) - 1;
		tail		%= mCapacity;
		ksU32 id	= mWriteHead[ tail ]++;

		if( id == 0 )
		{
			mItems[ tail ]		= std::move( pVal );
			WRITE_BARRIER;
			handle				= mItems + tail;
			mWriteHead[ tail ]	= 2;
		}
		else
		{
			atomic_decrement(&mTail);
			KS_ASSERT( 0 && "push_back failed. You may need to increase the capacity of this CyclicQueue<T>." );
		}

		return handle;
	}

/*@}*/
}	//namespace ks


#endif