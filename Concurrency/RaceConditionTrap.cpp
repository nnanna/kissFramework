//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		11/12/2016
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

#include "RaceConditionTrap.h"
#include "atomics.h"
#include "Debug.h"



RaceConditionTrap::RaceConditionTrap(RaceConditionContext& ctx) : mCtx(ctx)
{
	const int PrevThread		= mCtx.ThreadId;
	const int CurrentThreadId	= GetCurrentThreadId();
	mCtx.ThreadId				= CurrentThreadId;
	KS_ASSERT(++mCtx.ReentrancyCount == 1 || PrevThread == CurrentThreadId);
}
RaceConditionTrap::~RaceConditionTrap()
{
	const int CurrentThreadId = GetCurrentThreadId();
	KS_ASSERT(--mCtx.ReentrancyCount == 0 || mCtx.ThreadId == CurrentThreadId);
}

ScopedReadContentionCheck::ScopedReadContentionCheck(RaceConditionContext& ctx) : mCtx(ctx)
{
	KS_ASSERT(mCtx.ReentrancyCount == 0 || mCtx.ThreadId == GetCurrentThreadId());
}
ScopedReadContentionCheck::~ScopedReadContentionCheck()
{
	KS_ASSERT(mCtx.ReentrancyCount == 0 || mCtx.ThreadId == GetCurrentThreadId());
}

SilentRaceConditionTrap::SilentRaceConditionTrap(RaceConditionContext& ctx) : mCtx(ctx)
{
	mCtx.ThreadId = GetCurrentThreadId();
	++mCtx.ReentrancyCount;
}
SilentRaceConditionTrap::~SilentRaceConditionTrap()
{
	--mCtx.ReentrancyCount;
}

bool SilentRaceConditionTrap::IsContended() const
{
	return mCtx.ReentrancyCount == 1 || mCtx.ThreadId == GetCurrentThreadId();
}