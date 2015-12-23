/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2015)
///	@about		ThreadLocal stack allocator from heap memory for multithreaded environments.
///				Supports re-entrancy & cleans up on scope-exit
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


#ifndef KS_THREAD_ALLOCATOR
#define KS_THREAD_ALLOCATOR

#include <defines.h>

namespace ks {

namespace mem {

#define	MEGABYTE	1024 * 1024

	struct ThreadStackAllocator
	{
		ThreadStackAllocator(u32 pCapacity);
		~ThreadStackAllocator();

		void* alloc(u32 size);

	private:
		void*		mMem;
		u32			mIndex;
		const u32	mCapacity;
	};

} //namespace mem
} //namespace ks

#endif
