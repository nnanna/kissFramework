//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		28/01/2016
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

#include "ReadWriteLock.h"
#include "Debug.h"

namespace ks {

#define READ_COUNT_MASK				0x7fffffff
#define READ_COUNT_BITSIZE			31
#define EXCL_ENCODE( e, incount )	((e) << READ_COUNT_BITSIZE) | ((incount) & READ_COUNT_MASK)

	enum exclusivity
	{
		exclusive_none,
		exclusive_write
	};

	ReadGuard::ReadGuard(ReadWriteLock* pLock) : mOwner(pLock)
	{}

	ReadGuard::~ReadGuard()
	{
		if (mOwner)
			mOwner->Release(*this);
	}

	ReadGuard::ReadGuard(ReadGuard&& other)
	{
		mOwner			= other.mOwner;
		other.mOwner	= nullptr;
	}

	bool ReadGuard::Acquired() const			{ return mOwner != nullptr;  }


	WriteGuard::WriteGuard(ReadWriteLock* pLock) : mOwner(pLock)
	{}

	WriteGuard::WriteGuard(WriteGuard&& other)
	{
		mOwner = other.mOwner;
		other.mOwner = nullptr;
	}

	bool WriteGuard::Acquired() const			{ return mOwner != nullptr; }

	WriteGuard::~WriteGuard()
	{
		if (mOwner)
			mOwner->Release(*this);
	}


//////////////////////////////////////////////////////////////////////////
// ReadWriteLock: lockless async push_back, explicit lock on write
//////////////////////////////////////////////////////////////////////////
	static void cond_wait()			{ THREAD_YIELD;	}

	ReadWriteLock::ReadWriteLock() : mMutualExclusivityMask(exclusive_none), mWritingThread(0), mReentrancyCount(0)
	{}

	ReadWriteLock::~ReadWriteLock()
	{
		KS_ASSERT( mMutualExclusivityMask == 0 );
	}

	ReadGuard ReadWriteLock::Read()
	{
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		// No need to encode since we know the insert counter exclusively owns the lower 28 bits of mMutualExclusivityMask.
		// mask off the mutualexclusitivity to get the insert count. should aquire (or keep) insert exclusivity by inrementing insert count. 

		const ThreadID threadID = GetCurrentThreadId();
		u32 current_key	= (mMutualExclusivityMask & READ_COUNT_MASK);
		u32 insert_key	= current_key + 1;

		while (atomic_compare_and_swap(&mMutualExclusivityMask, current_key, insert_key) != current_key)
		{
			if (threadID == mWritingThread && mMutualExclusivityMask == lock_key)
			{
				atomic_increment((u32*)&mReentrancyCount);
				break;
			}

			cond_wait();
			current_key = (mMutualExclusivityMask & READ_COUNT_MASK);
			insert_key = current_key + 1;
		}

		return ReadGuard(this);
	}


	ReadGuard ReadWriteLock::TryRead()
	{
		u32 current_key = (mMutualExclusivityMask & READ_COUNT_MASK);
		u32 insert_key = current_key + 1;

		const bool success = (atomic_compare_and_swap(&mMutualExclusivityMask, current_key, insert_key) == current_key);

		return success ? ReadGuard(this) : ReadGuard(nullptr);
	}

	WriteGuard ReadWriteLock::Write()
	{
		const ThreadID threadID = GetCurrentThreadId();
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		while (atomic_compare_and_swap(&mMutualExclusivityMask, exclusive_none, lock_key) != exclusive_none)
		{
			if (threadID == mWritingThread)
			{
				atomic_increment((u32*)&mReentrancyCount);
				break;
			}
			cond_wait();
		}

		KS_ASSERT(mWritingThread == 0 || mWritingThread == threadID);
		mWritingThread = threadID;

		return WriteGuard(this);
	}

	WriteGuard ReadWriteLock::TryWrite()
	{
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		const bool success = (atomic_compare_and_swap(&mMutualExclusivityMask, exclusive_none, lock_key) == exclusive_none);
		if (success)
			mWritingThread = GetCurrentThreadId();

		return success ? WriteGuard(this) : WriteGuard(nullptr);
	}

	void ReadWriteLock::Release(WriteGuard&)
	{
		KS_ASSERT(mWritingThread == GetCurrentThreadId() && mReentrancyCount >= 0);
		int reentrants = mReentrancyCount > 0 ? atomic_decrement((u32*)&mReentrancyCount) + 1 : 0;
		if (reentrants == 0)
		{
			mWritingThread = 0;
			const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
			if (atomic_compare_and_swap(&mMutualExclusivityMask, lock_key, exclusive_none) != lock_key)
			{
				KS_ASSERT( ! "ReadWriteLockException::eAlreadyUnlocked" );
			}
		}
	}

	void ReadWriteLock::Release(ReadGuard&)
	{
		const u32 lock_key = EXCL_ENCODE(exclusive_write, 0);
		u32 mutual_mask = mMutualExclusivityMask;
		u32 current_key = mutual_mask & READ_COUNT_MASK;
		u32 exit_key = current_key - 1;
		while (atomic_compare_and_swap(&mMutualExclusivityMask, current_key, exit_key) != current_key )
		{
			if (mutual_mask == lock_key && mWritingThread == GetCurrentThreadId())	// it's a re-entrant read
			{
				int rentrants = atomic_decrement((u32*)&mReentrancyCount);
				KS_ASSERT(rentrants >= 0);
				break;
			}

			cond_wait();
			mutual_mask = mMutualExclusivityMask;
			current_key = mutual_mask & READ_COUNT_MASK;
			exit_key = current_key - 1;
		}
	}


}