/////////////////////////////////////////////////////////////////
//	CyclicQueue.h
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


#ifndef CYCLIC_QUEUE_H
#define CYCLIC_QUEUE_H


namespace ks {


	template<typename T>
	class CyclicQueue
	{
	public:

		struct queue_item
		{
			queue_item() : mIsValid(false)
			{}

			queue_item(std::nullptr_t) : mIsValid(false)
			{}

			queue_item(T&& pData, bool pValid) : mIsValid(pValid)
			{
				if (mIsValid)
					mData = std::move(pData);
			}

			queue_item(queue_item&& pOther) : mData(std::move(pOther.mData)), mIsValid(std::move(pOther.mIsValid))
			{}

			queue_item& operator=(queue_item&& pOther)
			{
				mData = std::move(pOther.mData);
				mIsValid = std::move(pOther.mIsValid);
				return *this;
			}

			inline operator T&()				{ return mData; }
			inline T* operator->()				{ return &mData; }
			inline bool operator*() const		{ return mIsValid; }

			T				mData;
		private:
			bool			mIsValid;

		private:
			queue_item& operator=(const queue_item&);
			queue_item(const queue_item&);
		};

		CyclicQueue( const ksU32 pCapacity );
		~CyclicQueue();

		queue_item dequeue();

		void dequeue(queue_item& q);

		T* const enqueue(T&& pVal);

		T* const enqueue_singlethreaded(T&& pVal);

		bool empty() const					{ return mReadHead == mTail; }
		void clear()						{ mReadHead = mTail = 0; }

	private:
		ksU32				mReadHead;
		char				pad0[CACHE_LINE_SIZE - sizeof(ksU32)];		// eliminates false sharing between indices
		char*				mWriteHead;									// per slot 'bloated bitfield' @TODO: still prone to false sharing
		char				pad1[CACHE_LINE_SIZE - sizeof(char*)];
		ksU32				mTail;
		char				pad2[CACHE_LINE_SIZE - sizeof(ksU32)];
		T*					mItems;
		const ksU32			mCapacity;

	};

}

#endif