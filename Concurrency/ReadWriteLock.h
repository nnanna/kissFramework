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

namespace ks {
	
	typedef unsigned int u32;
	
	class ReadWriteLock;

	struct ReadGuard
	{
		ReadGuard(ReadWriteLock& pLock);
		~ReadGuard();
		ReadGuard(ReadGuard&&);
	private:
		ReadGuard(const ReadGuard&);
		ReadGuard& operator=(const ReadGuard&);
		ReadWriteLock*	mOwner;
	};

	struct WriteGuard
	{
		WriteGuard(ReadWriteLock& pLock);
		~WriteGuard();
		WriteGuard(WriteGuard&&);
	private:
		WriteGuard(const WriteGuard&);
		WriteGuard& operator=(const WriteGuard&);
		ReadWriteLock*	mOwner;
	};



	class ReadWriteLock
	{
	public:
		ReadWriteLock();
		~ReadWriteLock();

		WriteGuard	Write();
		void		Release(WriteGuard&);

		ReadGuard	Read();
		void		Release(ReadGuard&);

	private:
		u32		mMutualExclusivityMask;
		bool	cond_wait();

	};


}