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

#include "Delegate.h"
#include "crc32.h"


namespace ks {

	IFuncInvoker::IFuncInvoker(const char* pName) : mRegistryIndex(-1)
	{
		mNameID = CRC32(pName);
	}

	IFuncInvoker::~IFuncInvoker()
	{
		InvokerRegistry::Remove(this);
		mNameID			= 0;
		mRegistryIndex	= -1;
	}

	void IFuncInvoker::SetRegistryIndex(u32 uIndex)
	{
		KS_ASSERT(mRegistryIndex == -1 && "Should be set just once!");
		mRegistryIndex = uIndex;
	}

	/////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////////////////
	InvokerRegistry	InvokerRegistry::gInvRegistry;

	InvokerRegistry::InvokerRegistry()
	{}

	InvokerRegistry::~InvokerRegistry()
	{
		for (auto i = this->begin(); i != this->end(); ++i)
		{
			delete (*i);
		}
	}

	/*static*/ int InvokerRegistry::Add(IFuncInvoker* pInvoker)
	{
		// TODO: fill null spaces. Ideally there won't be null spaces unless the end-user starts creating dynamic delegates
		// Alternatively, enforce PlacementBuffer usage for end-user.
		int index = gInvRegistry.push_back(pInvoker);
		pInvoker->SetRegistryIndex(index);
		return index;
	}

	/*static*/ void InvokerRegistry::Remove(IFuncInvoker* pInvoker)
	{
		const u32 index = pInvoker->GetRegistryIndex();
		if (index < gInvRegistry.size() && pInvoker == gInvRegistry[index])
		{
			gInvRegistry[index] = nullptr;		// do no erase to avoid re-ordering the indices
		}
	}

	/*static*/ IFuncInvoker* InvokerRegistry::At(u32 uIndex)
	{
		return (uIndex < gInvRegistry.size()) ? gInvRegistry.at(uIndex) : nullptr;
	}

}
