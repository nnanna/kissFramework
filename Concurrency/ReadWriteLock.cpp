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
#include "atomics.h"

namespace ks {

#define READ_COUNT_MASK				0x0fffffff
#define READ_COUNT_BITSIZE			28
#define EXCL_ENCODE( e, incount )	((e) << READ_COUNT_BITSIZE) | ((incount) & READ_COUNT_MASK)

	enum exclusivity
	{
		exclusive_none,
		exclusive_write
	};

	ReadGuard::ReadGuard(ReadWriteLock& pLock) : mOwner(&pLock)
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


	WriteGuard::WriteGuard(ReadWriteLock& pLock) : mOwner(&pLock)
	{}

	WriteGuard::WriteGuard(WriteGuard&& other)
	{
		mOwner = other.mOwner;
		other.mOwner = nullptr;
	}

	WriteGuard::~WriteGuard()
	{
		if (mOwner)
			mOwner->Release(*this);
	}


//////////////////////////////////////////////////////////////////////////
// ReadWriteLock: lockless async push_back, explicit lock on write
//////////////////////////////////////////////////////////////////////////
	bool ReadWriteLock::cond_wait()
	{
		THREAD_YIELD;
		return true;
	}

	ReadWriteLock::ReadWriteLock() : mMutualExclusivityMask(exclusive_none)
	{}

	ReadWriteLock::~ReadWriteLock()
	{}

	WriteGuard ReadWriteLock::Write()
	{
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		while (atomic_compare_and_swap(&mMutualExclusivityMask, exclusive_none, lock_key) != exclusive_none)
		{
			cond_wait();
		}
		return WriteGuard(*this);
	}

	void ReadWriteLock::Release(WriteGuard&)
	{
		const u32 lock_key(EXCL_ENCODE(exclusive_write, 0));
		if (atomic_compare_and_swap(&mMutualExclusivityMask, lock_key, exclusive_none) != lock_key)
		{
			//throw ReadWriteLockException::eAlreadyUnlocked;
		}
	}

	ReadGuard ReadWriteLock::Read()
	{
		u32 insert_key(0), current_key(0);
		// No need to encode since we know the insert counter exclusively owns the lower 28 bits of mMutualExclusivityMask.
		// mask off the mutualexclusitivity to get the insert count. should aquire (or keep) insert exclusivity by inrementing insert count. 
		do
		{
			current_key = (mMutualExclusivityMask & READ_COUNT_MASK);
			insert_key = current_key + 1;
		} while (atomic_compare_and_swap(&mMutualExclusivityMask, current_key, insert_key) != current_key && cond_wait());
		
		return ReadGuard(*this);
	}

	void ReadWriteLock::Release(ReadGuard&)
	{
		u32 exit_key(0), current_key(0);
		do
		{
			current_key = (mMutualExclusivityMask & READ_COUNT_MASK);
			exit_key = current_key - 1;
		} while (atomic_compare_and_swap(&mMutualExclusivityMask, current_key, exit_key) != current_key && cond_wait());

	}


}