/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2015)
///	@about		ThreadLocal stack allocator from heap memory for multithreaded environments
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

#include "ThreadStackAllocator.h"
#include <Concurrency\atomics.h>
#include <Debug.h>
#include <stdio.h>


namespace ks
{
	namespace mem {

#define TSA_DIAGNOSTICS		(defined _DEBUG)

#ifndef TSA_MIN_STACK_SIZE
	#define TSA_MIN_STACK_SIZE	MEGABYTE
#endif

#ifndef TSA_MAX_NUM_THREAD_STACKS
	#define TSA_MAX_NUM_THREAD_STACKS 8
#endif

	struct handle
	{
		void* Map()
		{
#if TSA_DIAGNOSTICS
			KS_ASSERT(locked == 0);
			locked = 1;
#endif
			return mem;
		}

		void Unmap(void* ptr)
		{
#if TSA_DIAGNOSTICS
			KS_ASSERT(mem == ptr);
			locked = 0;
#endif
		}

		void*	mem;
		u32		size;
#if TSA_DIAGNOSTICS
		char	locked;
#endif
	};


	ks_thread_local handle	tlsMemHandle = {};

	struct ThreadStackPool
	{
		ThreadStackPool()
			: mMappedMem{}
			, mNumThreadStacks(0)
		{}

		~ThreadStackPool()
		{
			for (u32 i = 0; i < TSA_MAX_NUM_THREAD_STACKS && mMappedMem[i]; ++i)
			{
				free(mMappedMem[i]);
				mMappedMem[i] = nullptr;
			}
		}

		void* allocate()
		{
			const u32 index = (tlsMemHandle.mem == nullptr) ? atomic_increment(&mNumThreadStacks) - 1 : TSA_MAX_NUM_THREAD_STACKS;
			if (index < TSA_MAX_NUM_THREAD_STACKS)
			{
				tlsMemHandle.mem = malloc(TSA_MIN_STACK_SIZE);
				tlsMemHandle.size = TSA_MIN_STACK_SIZE;

				mMappedMem[index] = tlsMemHandle.mem;
			}

			return tlsMemHandle.Map();
		}

		void release(void* ptr)
		{
			tlsMemHandle.Unmap(ptr);
		}

	private:
		void*	mMappedMem[TSA_MAX_NUM_THREAD_STACKS];		// just so we can properly clean up allocations
		u32		mNumThreadStacks;

	}gThreadStackPool;


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// ThreadStackAllocator
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	ks_thread_local u32 tlsStackUsage = 0;

	ThreadStackAllocator::ThreadStackAllocator(u32 pCapacity) : mMem(nullptr), mIndex(0), mCapacity(pCapacity)
	{
		if (mCapacity)
		{
			if (tlsStackUsage == 0)
			{
				mMem = gThreadStackPool.allocate();
			}
			else if (tlsStackUsage + pCapacity <= tlsMemHandle.size)
			{
				mMem = (char*)tlsMemHandle.mem + tlsStackUsage;
			}
			else
			{
				printf("Thread stack out of mem, defaulting to heap. Consider resizing to %f MB", float(tlsStackUsage / MEGABYTE));
				mMem = malloc(mCapacity);
			}
		}

		tlsStackUsage += mCapacity;
	}

	ThreadStackAllocator::~ThreadStackAllocator()
	{
		if (mMem)
		{
			if (mMem < tlsMemHandle.mem || mMem >(char*)tlsMemHandle.mem + tlsStackUsage)
			{
				free(mMem);											// outside range = dynamically allocated
			}
			else if ((tlsStackUsage - mCapacity) == 0)
			{
				gThreadStackPool.release(mMem);
			}

			tlsStackUsage -= mCapacity;
		}
	}

	void* ThreadStackAllocator::allocate()
	{
		return allocate(mCapacity - mIndex);
	}

	void* ThreadStackAllocator::allocate(u32 size)
	{
		void* ptr(nullptr);
		if (mIndex + size <= mCapacity)
		{
			ptr		= static_cast<char*>(mMem) + mIndex;
			mIndex	+= size;
		}
		else
		{
			KS_ASSERT(0 && "Out of memory: stack factory buffer too small");
		}

		return ptr;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// StackBuffer
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	StackBuffer::StackBuffer(void* pMem, u32 pCapacity) : mMem(pMem), mIndex(0), mCapacity(pCapacity)
	{}

	StackBuffer::~StackBuffer()
	{}

	void* StackBuffer::allocate()
	{
		return allocate(mCapacity - mIndex);
	}

	void* StackBuffer::allocate(u32 size)
	{
		void* ptr(nullptr);
		if (mIndex + size <= mCapacity)
		{
			ptr = static_cast<char*>(mMem)+mIndex;
			mIndex += size;
		}
		else
		{
			KS_ASSERT(0 && "Out of memory: stack buffer too small");
		}

		return ptr;
	}

} //namespace mem
} //namespace ks

