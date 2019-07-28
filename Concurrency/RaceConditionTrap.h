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

#pragma once


struct RaceConditionContext
{
	RaceConditionContext() : ThreadId(0), ReentrancyCount(0)
	{}
	int	ThreadId;
	int ReentrancyCount;
};

struct RaceConditionTrap
{
	RaceConditionTrap(RaceConditionContext& ctx);
	~RaceConditionTrap();
private:
	RaceConditionContext& mCtx;
};

struct ScopedReadContentionCheck
{
	ScopedReadContentionCheck(RaceConditionContext& ctx);
	~ScopedReadContentionCheck();
private:
	RaceConditionContext& mCtx;
};

struct SilentRaceConditionTrap
{
	SilentRaceConditionTrap(RaceConditionContext& ctx);
	~SilentRaceConditionTrap();
	bool IsContended() const;
private:
	RaceConditionContext& mCtx;
};

#if !FINAL_BUILD
	#define DECLARE_RACE_CONDITION_CONTEXT(ctx)			mutable RaceConditionContext ctx
	#define DECLARE_SILENT_RACE_CONDITION_CHECKER(c)	static RaceConditionContext stCcctx; SilentRaceConditionTrap c(stCcctx)
	#define DECLARE_STATIC_RACE_CONDITION_CONTEXT(ctx)	static RaceConditionContext ctx
	#define CHECK_IS_SINGLE_THREAD_ACCESS()				static RaceConditionContext ctx; RaceConditionTrap scc(ctx)
	#define CHECK_IS_THREADSAFE_ACCESS(ctx)				RaceConditionTrap scc(ctx)
	#define CHECK_IS_THREADSAFE_READ_ACCESS(ctx)		ScopedReadContentionCheck srcc(ctx)
#else
	#define DECLARE_RACE_CONDITION_CONTEXT(ctx)
	#define DECLARE_SILENT_RACE_CONDITION_CHECKER(c)
	#define DECLARE_STATIC_RACE_CONDITION_CONTEXT(ctx)
    #define CHECK_IS_SINGLE_THREAD_ACCESS()
	#define CHECK_IS_THREADSAFE_ACCESS(ctx)
	#define CHECK_IS_THREADSAFE_READ_ACCESS(ctx)
#endif