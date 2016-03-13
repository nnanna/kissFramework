//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		28/01/2016
///	@about		ReadWriteLock : lockless async read, explicit lock on write
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

#include "atomics.h"

namespace ks {
	
	typedef unsigned int u32;
	
	class ReadWriteLock;

	struct ReadGuard
	{
		ReadGuard(ReadWriteLock* pLock);
		~ReadGuard();
		ReadGuard(ReadGuard&&);
		void Release();
		bool Acquired() const;
	private:
		ReadGuard(const ReadGuard&);
		ReadGuard& operator=(const ReadGuard&);
		ReadWriteLock*	mOwner;
	};

	struct WriteGuard
	{
		WriteGuard(ReadWriteLock* pLock);
		~WriteGuard();
		WriteGuard(WriteGuard&&);
		void Release();
		bool Acquired() const;
	private:
		WriteGuard(const WriteGuard&);
		WriteGuard& operator=(const WriteGuard&);
		ReadWriteLock*	mOwner;
	};



	class ReadWriteLock
	{
		friend struct ReadGuard;
		friend struct WriteGuard;

	public:
		ReadWriteLock();
		~ReadWriteLock();

		ReadGuard	Read();
		WriteGuard	Write();

		ReadGuard	TryRead();
		WriteGuard	TryWrite();

	private:
		void		Release(WriteGuard&);
		void		Release(ReadGuard&);

		u32			mMutualExclusivityMask;
		char		pad0[CACHE_LINE_SIZE - sizeof(u32)];
		ThreadID	mWritingThread;
		char		pad1[CACHE_LINE_SIZE - sizeof(ThreadID)];
		int			mReentrancyCount;
	};

}