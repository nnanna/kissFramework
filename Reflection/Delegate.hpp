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
#include <Reflection\TypeUID.h>
#include <Reflection\DataProperty.hpp>

#ifndef KS_DELEGATE_HPP
#define KS_DELGATE_HPP

namespace ks {


	//////////////////////////////////////////////////////////////////////////
	// FunkInvoker
	//////////////////////////////////////////////////////////////////////////

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)();
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)(arg1);
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)(arg1, arg2);
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)(arg1, arg2, arg3);
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, Any& arg4, INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)(arg1, arg2, arg3, arg4);
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, Any& arg4, Any& arg5, INST* pInstance, void_tag)
	{
		(pInstance->*mFunc)(arg1, arg2, arg3, arg4, arg5);
		return Any();
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(INST* pInstance, value_tag)
	{
		Any ret = Any((pInstance->*mFunc)());
		return ret;
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, INST* pInstance, value_tag)
	{
		Any ret = (pInstance->*mFunc)(arg1);
		return ret;
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, INST* pInstance, value_tag)
	{
		Any ret = (pInstance->*mFunc)(arg1, arg2);
		return ret;
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, INST* pInstance, value_tag)
	{
		Any ret = (pInstance->*mFunc)(arg1, arg2, arg3);
		return ret;
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, Any& arg4, INST* pInstance, value_tag)
	{
		Any ret = (pInstance->*mFunc)(arg1, arg2, arg3, arg4);
		return ret;
	}

	FI_HEADER Any FI_CLASSNAME::resolveDispatch(Any& arg1, Any& arg2, Any& arg3, Any& arg4, Any& arg5, INST* pInstance, value_tag)
	{
		Any ret = (pInstance->*mFunc)(arg1, arg2, arg3, arg4, arg5);
		return ret;
	}


	//////////////////////////////////////////////////////////////////////////
	// IFuncInvoker
	// TODO: automatic type-deduction and argument forwarding -> unfold and const cast [done]
	// one create function with SFINAE on return type. [done]
	// FuncInvoker holds the generic function/method pointer data
	// Higher level (user-facing) delegate holds the instance of the class.
	// Preserve object type-safety as func interface abstracts to void*
	// DataProperty could be the delegate? Hold pointer to static list of invokers?
	//////////////////////////////////////////////////////////////////////////

	template<typename INST, typename RETURN_TYPE, int NARGS> struct FIFactory
	{
		template<typename FN>
		IFuncInvoker* Create(const char* pName, FN pFunc)
		{
			return new FuncInvoker<INST, FN, RETURN_TYPE, NARGS>(pName, pFunc);
		}
	};

	HEADER_ARG0 typename FIFactory<INST, RETURN_TYPE, 0> GetFIFactory(FUNC0)		{ return FIFactory<INST, RETURN_TYPE, 0>(); }
	HEADER_ARG0 typename FIFactory<INST, RETURN_TYPE, 0> GetFIFactory(FUNC0 const)	{ return FIFactory<INST, RETURN_TYPE, 0>(); }

	HEADER_ARG1 typename FIFactory<INST, RETURN_TYPE, 1> GetFIFactory(FUNC1)		{ return FIFactory<INST, RETURN_TYPE, 1>(); }
	HEADER_ARG1 typename FIFactory<INST, RETURN_TYPE, 1> GetFIFactory(FUNC1 const)	{ return FIFactory<INST, RETURN_TYPE, 1>(); }

	HEADER_ARG2 typename FIFactory<INST, RETURN_TYPE, 2> GetFIFactory(FUNC2)		{ return FIFactory<INST, RETURN_TYPE, 2>(); }
	HEADER_ARG2 typename FIFactory<INST, RETURN_TYPE, 2> GetFIFactory(FUNC2 const)	{ return FIFactory<INST, RETURN_TYPE, 2>(); }

	HEADER_ARG3 typename FIFactory<INST, RETURN_TYPE, 3> GetFIFactory(FUNC3)		{ return FIFactory<INST, RETURN_TYPE, 3>(); }
	HEADER_ARG3 typename FIFactory<INST, RETURN_TYPE, 3> GetFIFactory(FUNC3 const)	{ return FIFactory<INST, RETURN_TYPE, 3>(); }

	HEADER_ARG4 typename FIFactory<INST, RETURN_TYPE, 4> GetFIFactory(FUNC4)		{ return FIFactory<INST, RETURN_TYPE, 4>(); }
	HEADER_ARG4 typename FIFactory<INST, RETURN_TYPE, 4> GetFIFactory(FUNC4 const)	{ return FIFactory<INST, RETURN_TYPE, 4>(); }

	HEADER_ARG5 typename FIFactory<INST, RETURN_TYPE, 5> GetFIFactory(FUNC5)		{ return FIFactory<INST, RETURN_TYPE, 5>(); }
	HEADER_ARG5 typename FIFactory<INST, RETURN_TYPE, 5> GetFIFactory(FUNC5 const)	{ return FIFactory<INST, RETURN_TYPE, 5>(); }



	template<typename T>
	IFuncInvoker* IFuncInvoker::Create(const char* pName, T pFunc)
	{
		auto inv = GetFIFactory(pFunc).Create(pName, pFunc);
		InvokerRegistry::Add(inv);
		return inv;
	}
}

#endif