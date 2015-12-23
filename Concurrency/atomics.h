/*
 * atomics.h
 * Copyright (c)
 *	@author		Nnanna Kama
 *	@date		28/02/2015
 Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

#ifndef ATOMICS_H_
#define ATOMICS_H_


#if _MSC_VER

#include "Windows.h"

	#define atomic_compare_and_swap(ptr, oldval, newval)			InterlockedCompareExchange(ptr, newval, oldval)
	#define atomic_compare_and_swap_pointer(ptr, oldval, newval)	InterlockedCompareExchangePointer(ptr, newval, oldval)
	#define atomic_exchange_pointer(ptr, newval)					InterlockedExchangePointer(ptr, newval)
	#define atomic_or_into(ptr, val)								InterlockedOr( (volatile long*)ptr, val )
	#define atomic_increment										InterlockedIncrement
	#define atomic_decrement										InterlockedDecrement

	#define WRITE_BARRIER				_WriteBarrier(); MemoryBarrier()
	#define READ_BARRIER				_ReadBarrier(); MemoryBarrier()
	#define THREAD_YIELD				YieldProcessor()
	#define THREAD_SWITCH				SwitchToThread()
	#define THREAD_SLEEP				Sleep

	#define ks_current_thread_id		GetCurrentThreadId
	#define ks_thread_id				DWORD

#else	// __GNUC__

#include "sched.h"

	#define atomic_compare_and_swap			__sync_val_compare_and_swap
	#define atomic_compare_and_swap_pointer		__sync_val_compare_and_swap
	#define atomic_exchange_pointer(ptr, newval)					InterlockedExchangePointer(ptr, newval)
	#define atomic_increment(x)			__sync_add_and_fetch( x, 1 )
	#define atomic_decrement(x)			__sync_sub_and_fetch( x, 1 )

	#define WRITE_BARRIER				asm volatile("": : :"memory"); __sync_synchronize()
	#define READ_BARRIER				asm volatile("": : :"memory"); __sync_synchronize()
	#define THREAD_YIELD				sched_yield()
	#define THREAD_SWITCH				asm volatile("pause\n": : :"memory")
	#define THREAD_SLEEP				sleep


	#define ks_thread_id				pthread_t
#endif


#endif /* ATOMICS_H_ */
