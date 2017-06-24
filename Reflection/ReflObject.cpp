
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2017)
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

#include "ReflObject.h"
#include "crc32.h"

namespace ks {

	BindingsMap::BindingsMap(char* pAllocationBuffer, u32* pHashBuffer, u32 pNumBindings)
		: mAllocationBuffer(pAllocationBuffer)
		, mHashes(pHashBuffer)
		, mNumBindings(pNumBindings)
		, mCaret(0)
	{}

	void BindingsMap::Add(const char* pName, IFuncInvoker::PlacementBuffer& pBuffer)
	{
		memcpy_s(mAllocationBuffer + (mCaret * FUNC_INVOKER_SIZE), FUNC_INVOKER_SIZE, *pBuffer, sizeof(IFuncInvoker::PlacementBuffer));
		mHashes[mCaret++] = GetHash(pName);

#define KSBM_OPTIMAL_LINEAR_SEARCH_SIZE	8

		if (mCaret == mNumBindings && mNumBindings > KSBM_OPTIMAL_LINEAR_SEARCH_SIZE)
		{
			// TODO: sort delegates? might be necessary if they're larger than KSBM_OPTIMAL_LINEAR_SEARCH_SIZE.
		}
	}

	u32 BindingsMap::GetHash(const char* pName) const
	{
		return CRC32(pName);
	}

	IFuncInvoker* BindingsMap::Find(const char* pName) const
	{
		return Find(GetHash(pName));
	}

	IFuncInvoker* BindingsMap::Find(u32 pHash) const
	{
		// TODO: if IsSorted() {i.e mCaret == mNumBindings}.. do binary search.
		for (u32 i = 0; i < mCaret; ++i)
		{
			if (mHashes[i] == pHash)
				return (IFuncInvoker*)(mAllocationBuffer + (i * FUNC_INVOKER_SIZE));
		}
		return nullptr;
	}

	IFuncInvoker* BindingsMap::At(u32 index) const
	{
		if (index < mCaret)
			return (IFuncInvoker*)(mAllocationBuffer + (index * FUNC_INVOKER_SIZE));

		return nullptr;
	}

//////////////////////////////////////////////////////////////////////////////
// ReflObject
//////////////////////////////////////////////////////////////////////////////
#if _DEBUG || REFL_PROTECTED_MODE
#define REFL_ASSERT_EARLY_OUT(x) if(!(x)) { KS_ASSERT(0); return Any(); }
#else
#define REFL_ASSERT_EARLY_OUT(x)
#endif

	ReflObject::ReflObject(ReflObject& o) : mObject(o.mObject), mBindings(o.mBindings)
	{}

	void ReflObject::operator=(const ReflObject& o)
	{
		mObject = o.mObject;
		mBindings = o.mBindings;
	}

	const char* ReflObject::Typename() const
	{
		return mBindings->At(0)->Typename();
	}

	u32 ReflObject::TypeID() const
	{
		return mBindings->At(0)->TypeID();
	}

	Any ReflObject::Call(const char* pFuncName)
	{
		IFuncInvoker* f = mBindings->Find(pFuncName);
		REFL_ASSERT_EARLY_OUT(f);
		return (*f)(mObject);
	}
	Any ReflObject::Call(const char* pFuncName, Any&& arg1)
	{
		IFuncInvoker* f = mBindings->Find(pFuncName);
		REFL_ASSERT_EARLY_OUT(f);
		return (*f)(mObject, arg1);
	}
	Any ReflObject::Call(const char* pFuncName, Any&& arg1, Any&& arg2)
	{
		IFuncInvoker* f = mBindings->Find(pFuncName);
		REFL_ASSERT_EARLY_OUT(f);
		return (*f)(mObject, arg1, arg2);
	}

}