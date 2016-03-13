//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		11/03/2016
///	@about		BitLock : Useful for concurrent access to specific indices/buckets within a large container
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

	//
	//
	class BitLock
	{
	public:
		BitLock();
		void Lock(u32 index);
		void Unlock(u32 index);

		// for less bit-contention over bigger ranges. trivially extensible to higher bit ranges
		void Lock64(u32 index);
		void Unlock64(u32 index);

		void Lock128(u32 index);
		void Unlock128(u32 index);
	private:
#define BL_ARRAY_SIZE	4
		template<u32 BITRANGE>
		void lock(u32 index);

		template<u32 BITRANGE>
		void unlock(u32 index);

		u32		mBits[ BL_ARRAY_SIZE ];
		char	pad[CACHE_LINE_SIZE - (sizeof(u32) * BL_ARRAY_SIZE)];
	};

}