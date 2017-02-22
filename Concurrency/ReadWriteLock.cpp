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
#include "Semaphore.h"
#include "Debug.h"

namespace ks {

#define READ_COUNT_MASK				0x7fffffff
#define READ_COUNT_BITSIZE			31
#define WRITE_MASK_VAL				(1 << READ_COUNT_BITSIZE)

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

	ReadWriteLock::ReadWriteLock() : mMutualExclusivityMask(0), mWritingThread(0), mReentrancyCount(0)
	{
		mEvent = (uintptr_t)CreateEvent(
			NULL,               // default security attributes
			TRUE,				// manual-reset event
			FALSE,				// initial state is nonsignaled
			nullptr				// object name
			);
	}

	ReadWriteLock::~ReadWriteLock()
	{
		KS_ASSERT(mMutualExclusivityMask == 0);
		CloseHandle((HANDLE)mEvent);
	}

	ReadGuard ReadWriteLock::Read()
	{
		const u32 omask = atomic_increment(&mMutualExclusivityMask);
		u32 mask = omask;

		while ((mask >> READ_COUNT_BITSIZE) != 0)
		{
			if (GetCurrentThreadId() == mWritingThread)
			{
				return ReadGuard(this);
			}

			WaitForSingleObject((HANDLE)mEvent, 1);
			mask = mMutualExclusivityMask;
		}
		if ((omask & READ_COUNT_MASK) == 1)		// only 'first' reader should block the event
			ResetEvent((HANDLE)mEvent);

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
		while (atomic_compare_and_swap(&mMutualExclusivityMask, 0, WRITE_MASK_VAL) != 0)
		{
			if (threadID == mWritingThread)
			{
				++mReentrancyCount;
				return WriteGuard(this);
			}

			WaitForSingleObject((HANDLE)mEvent, INFINITE);
		}
		ResetEvent((HANDLE)mEvent);

		KS_ASSERT(mWritingThread == 0 || mWritingThread == threadID);
		mWritingThread = threadID;

		return WriteGuard(this);
	}

	WriteGuard ReadWriteLock::TryWrite()
	{
		const bool success = (atomic_compare_and_swap(&mMutualExclusivityMask, 0, WRITE_MASK_VAL) == 0);
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
			const u32 lock_key = atomic_and(&mMutualExclusivityMask, READ_COUNT_MASK);	// [release]
			if ((lock_key >> READ_COUNT_BITSIZE) != 1)
			{
				KS_ASSERT(!"ReadWriteLockException::eAlreadyUnlocked");
			}

			SetEvent((HANDLE)mEvent);
		}
	}

	void ReadWriteLock::Release(ReadGuard&)
	{
		if (atomic_decrement(&mMutualExclusivityMask) == 0)
			SetEvent((HANDLE)mEvent);
	}

}