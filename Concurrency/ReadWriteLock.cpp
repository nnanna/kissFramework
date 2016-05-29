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
		mOwner = other.mOwner;
		other.mOwner = nullptr;
	}

	void ReadGuard::Release()
	{
		if (mOwner)
		{
			mOwner->Release(*this);
			mOwner = nullptr;
		}
	}

	bool ReadGuard::Acquired() const			{ return mOwner != nullptr; }


	WriteGuard::WriteGuard(ReadWriteLock* pLock) : mOwner(pLock)
	{}

	WriteGuard::~WriteGuard()
	{
		if (mOwner)
			mOwner->Release(*this);
	}

	WriteGuard::WriteGuard(WriteGuard&& other)
	{
		mOwner = other.mOwner;
		other.mOwner = nullptr;
	}

	bool WriteGuard::Acquired() const			{ return mOwner != nullptr; }

	void WriteGuard::Release()
	{
		if (mOwner)
		{
			mOwner->Release(*this);
			mOwner = nullptr;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// ReadWriteLock: lockless async read, explicit lock on write. Re-entrant
	//////////////////////////////////////////////////////////////////////////
	static void cond_wait()			{ ksYieldProcessor; }

	ReadWriteLock::ReadWriteLock() : mMutualExclusivityMask(exclusive_none), mWritingThread(0), mReentrancyCount(0)
	{}

	ReadWriteLock::~ReadWriteLock()
	{
		KS_ASSERT(mMutualExclusivityMask == 0);
	}

	ReadGuard ReadWriteLock::Read()
	{
		const ThreadID threadID = GetCurrentThreadId();
		u32 mask = atomic_increment(&mMutualExclusivityMask);

		while ((mask >> READ_COUNT_BITSIZE) != 0)
		{
			if (threadID == mWritingThread)
			{
				break;
			}

			cond_wait();
			mask = mMutualExclusivityMask;
		}

		return ReadGuard(this);
	}


	ReadGuard ReadWriteLock::TryRead()
	{
		u32 mask = atomic_increment(&mMutualExclusivityMask);

		if ((mask >> READ_COUNT_BITSIZE) != 0 && GetCurrentThreadId() != mWritingThread)
		{
			atomic_decrement(&mMutualExclusivityMask);
			mask = 0;
		}

		return mask > 0 ? ReadGuard(this) : ReadGuard(nullptr);
	}

	WriteGuard ReadWriteLock::Write()
	{
		const ThreadID threadID = GetCurrentThreadId();
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		while (atomic_compare_and_swap(&mMutualExclusivityMask, exclusive_none, lock_key) != exclusive_none)
		{
			if (threadID == mWritingThread)
			{
				++mReentrancyCount;
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
		int reentrants = mReentrancyCount > 0 ? mReentrancyCount-- : 0;
		if (reentrants == 0)
		{
			mWritingThread = 0;
			const u32 lock_key = atomic_and(&mMutualExclusivityMask, READ_COUNT_MASK);
			if ((lock_key >> READ_COUNT_BITSIZE) != 1)
			{
				KS_ASSERT(!"ReadWriteLockException::eAlreadyUnlocked");
			}
		}
	}

	void ReadWriteLock::Release(ReadGuard&)
	{
		atomic_decrement(&mMutualExclusivityMask);
	}

}