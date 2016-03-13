//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		11/03/2016
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

#include "BitLock.h"
#include "Debug.h"

namespace ks {


	BitLock::BitLock() : mBits{}
	{}

	void BitLock::Lock(u32 index)
	{
		index &= 31;							// to bit-range
		const u32 mask = (1 << index);
		while (((atomic_or_into(mBits, mask) & mask) >> index) != 0)
			THREAD_SWITCH;
	}

	void BitLock::Unlock(u32 index)
	{
		index &= 31;
		u32 mask = ~(1 << index);
		mask = atomic_and(mBits, mask) & ~mask;
		KS_ASSERT( (mask >> index) == 1 );
	}

	template<u32 BITRANGE>
	void BitLock::lock(u32 index)
	{
		static_assert( BITRANGE <= 32 * BL_ARRAY_SIZE, "BL_ARRAY_SIZE is too small for this bitrange" );
		index &= (BITRANGE - 1);
		u32 i = index >> 5;						// (index / 32) == array_index
		index &= 31;							// bit index
		const u32 mask = (1 << index);
		while (((atomic_or_into(mBits + i, mask) & mask) >> index) != 0)
			THREAD_SWITCH;
	}

	template<u32 BITRANGE>
	void BitLock::unlock(u32 index)
	{
		static_assert( BITRANGE <= 32 * BL_ARRAY_SIZE, "BL_ARRAY_SIZE is too small for this bitrange" );
		index	&= (BITRANGE - 1);
		u32 i	= index >> 5;
		index	&= 31;
		u32 mask = ~(1 << index);
		mask = atomic_and(mBits + i, mask) & ~mask;
		KS_ASSERT((mask >> index) == 1);
	}

	void BitLock::Lock64(u32 index)
	{
		lock<64>(index);
	}

	void BitLock::Unlock64(u32 index)
	{
		unlock<64>(index);
	}

	void BitLock::Lock128(u32 index)
	{
		lock<128>(index);
	}

	void BitLock::Unlock128(u32 index)
	{
		unlock<128>(index);
	}
}