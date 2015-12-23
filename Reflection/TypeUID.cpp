/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2014)
///	@about		Templated auto-generation of typeid and specialisable typenames
///	@references	article "Making your own type id is fun" by Alex Darby
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


#include "TypeUID.h"
#include <Concurrency/atomics.h>

namespace ks	{

	const u32 UIDGenerator::INVALID_UID = 0;
	const u32 DEFAULT_UID				= UIDGenerator::INVALID_UID + 1;

	UIDGenerator::UIDGenerator() : mMarker(DEFAULT_UID)
	{}

	u32 UIDGenerator::Get(const u32 mask)
	{
		if (mMarker == (INVALID_UID | mask)) mMarker = DEFAULT_UID;		// could get triggered on wrap-around
		u32 result = mMarker++;

		return result;
	}

	u32 UIDGenerator::GetAsync(const u32 mask)
	{
		atomic_compare_and_swap(&mMarker, (INVALID_UID | mask), DEFAULT_UID);
		u32 result = atomic_or_into(&mMarker, INVALID_UID);
		atomic_increment(&mMarker);
		return result;
	}


}