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


#include <defines.h>
#include <Containers\Array.h>
#include <Reflection\DataProperty.h>

#ifndef KS_DELEGATE_H
#define KS_DELEGATE_H

namespace ks {

#define KSFI_HAVE_DEBUG_NAME			0	//_DEBUG

	typedef DataProperty	Any;

	class IFuncInvoker
	{
		u32	mNameID;				// hash from NameString
		u32	mRegistryIndex;

#define HEADER_ARG0 template<typename INST, typename RETURN_TYPE>
#define HEADER_ARG1	template<typename INST, typename RETURN_TYPE, typename ARG1>
#define HEADER_ARG2	template<typename INST, typename RETURN_TYPE, typename ARG1, typename ARG2>
#define HEADER_ARG3	template<typename INST, typename RETURN_TYPE, typename ARG1, typename ARG2, typename ARG3>
#define HEADER_ARG4	template<typename INST, typename RETURN_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
#define HEADER_ARG5	template<typename INST, typename RETURN_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
#define FUNC0 RETURN_TYPE (INST::*pFunc)()
#define FUNC1 RETURN_TYPE (INST::*pFunc)(ARG1)
#define FUNC2 RETURN_TYPE (INST::*pFunc)(ARG1, ARG2)
#define FUNC3 RETURN_TYPE (INST::*pFunc)(ARG1, ARG2, ARG3)
#define FUNC4 RETURN_TYPE (INST::*pFunc)(ARG1, ARG2, ARG3, ARG4)
#define FUNC5 RETURN_TYPE (INST::*pFunc)(ARG1, ARG2, ARG3, ARG4, ARG5)

#if KSFI_HAVE_DEBUG_NAME
#define FUNC_INVOKER_SIZE	((sizeof(u32) * 2) + (sizeof(uintptr_t) * 3))
#else
#define FUNC_INVOKER_SIZE	((sizeof(u32) * 2) + (sizeof(uintptr_t) * 2))
#endif

	public:
		IFuncInvoker(const char* pName);
		virtual ~IFuncInvoker();

		u32 GetNameID() const			{ return mNameID; }

		u32 GetRegistryIndex() const	{ return mRegistryIndex; }

		void SetRegistryIndex(u32 Index);

		virtual Any operator()(void* pClass) = 0;
		virtual Any operator()(void* pClass, Any& arg1) = 0;
		virtual Any operator()(void* pClass, Any& arg1, Any& arg2) = 0;
		virtual Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3) = 0;
		virtual Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3, Any& arg4) = 0;
		virtual Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3, Any& arg4, Any& arg5) = 0;

		virtual const char* Typename() const = 0;
		virtual u32 TypeID() const = 0;

		struct PlacementBuffer
		{
			char data[FUNC_INVOKER_SIZE];
			char* operator*() { return data; }
		};

		struct RegistryHandle
		{
			IFuncInvoker* Invoker;
			u32 RegistryIndex;
		};

		template<typename T>
		static RegistryHandle Create(const char* pName, T pFunc);

		template<typename T>
		static IFuncInvoker* Create(const char* pName, T pFunc, PlacementBuffer& pUserManagedAllocationMem);
	};

/////////////////////////////////////////////////////////////////////////
// Tag dispatch
/////////////////////////////////////////////////////////////////////////
	struct void_tag {};		// for void return methods
	struct value_tag {};	// methods that return non-void values
	struct error_tag {};	// illegal dispatches

	// tag-dispatcher
	template<typename T, bool B>
	struct dtag {};

	// partial specialisations
	template<>				struct dtag<void, true>		{ typedef void_tag type; };
	template<>				struct dtag<void, false>	{ typedef error_tag type; };
	template<typename T>	struct dtag<T, true>		{ typedef value_tag type; };
	template<typename T>	struct dtag<T, false>		{ typedef error_tag type; };


/////////////////////////////////////////////////////////////////////////
// TFuncInvoker
/////////////////////////////////////////////////////////////////////////
#define FI_HEADER		template<class INST, class TFUNC, typename RETURN_TYPE, int NARGS>

	FI_HEADER
	class TFuncInvoker : public IFuncInvoker
	{
#define FI_CLASSNAME	TFuncInvoker<INST, TFUNC, RETURN_TYPE, NARGS>

	public:
		TFuncInvoker(const char* pName, TFUNC pFunc)
			: IFuncInvoker(pName)
			, mFunc(pFunc)
		{
#if KSFI_HAVE_DEBUG_NAME
			mDebugName = pName;
#endif
		}

		~TFuncInvoker()
		{
#if KSFI_HAVE_DEBUG_NAME
			mDebugName = NULL;
#endif
		}

		inline Any operator()(void* pClass) override
		{
			return resolveDispatch((INST*)pClass, dtag<RETURN_TYPE, NARGS == 0>::type());
		}
		inline Any operator()(void* pClass, Any& arg1) override
		{
			return resolveDispatch(arg1, (INST*)pClass, dtag<RETURN_TYPE, NARGS == 1>::type());
		}
		inline Any operator()(void* pClass, Any& arg1, Any& arg2) override
		{
			return resolveDispatch(arg1, arg2, (INST*)pClass, dtag<RETURN_TYPE, NARGS == 2>::type());
		}
		inline Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3) override
		{
			return resolveDispatch(arg1, arg2, arg3, (INST*)pClass, dtag<RETURN_TYPE, NARGS == 3>::type());
		}
		inline Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3, Any& arg4) override
		{
			return resolveDispatch(arg1, arg2, arg3, arg4, (INST*)pClass, dtag<RETURN_TYPE, NARGS == 4>::type());
		}
		inline Any operator()(void* pClass, Any& arg1, Any& arg2, Any& arg3, Any& arg4, Any& arg5) override
		{
			return resolveDispatch(arg1, arg2, arg3, arg4, arg5, (INST*)pClass, dtag<RETURN_TYPE, NARGS == 5>::type());
		}

		const char* Typename() const override;

		u32 TypeID() const override;

	private:
		TFUNC					mFunc;
#if KSFI_HAVE_DEBUG_NAME
		const char*				mDebugName;
#endif

		Any errorCall()
		{
			KS_ASSERT(0 && "Invalid arguments for function call");
			return Any();
		}

		Any resolveDispatch(INST* p, value_tag);
		Any resolveDispatch(INST* p, void_tag);
		Any resolveDispatch(INST* p, error_tag) { return errorCall(); }

		Any resolveDispatch(Any& a1, INST* p, value_tag);
		Any resolveDispatch(Any& a1, INST* p, void_tag);
		Any resolveDispatch(Any& a1, INST* p, error_tag) { return errorCall(); }

		Any resolveDispatch(Any& a1, Any& a2, INST* p, value_tag);
		Any resolveDispatch(Any& a1, Any& a2, INST* p, void_tag);
		Any resolveDispatch(Any& a1, Any& a2, INST* p, error_tag) { return errorCall(); }

		Any resolveDispatch(Any& a1, Any& a2, Any& a3, INST* p, value_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, INST* p, void_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, INST* p, error_tag) { return errorCall(); }

		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, INST* p, value_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, INST* p, void_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, INST* p, error_tag) { return errorCall(); }

		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, Any& a5, INST* p, value_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, Any& a5, INST* p, void_tag);
		Any resolveDispatch(Any& a1, Any& a2, Any& a3, Any& a4, Any& a5, INST* p, error_tag) { return errorCall(); }
	};



	//////////////////////////////////////////////////////////////////////////
	// InvokerRegistry
	// Helps keep track of all IFuncInvoker(s) so they can be cleaned up on program exit
	// Do not use directly!
	// TODO: Add threadsafety mechanism
	//////////////////////////////////////////////////////////////////////////
	class InvokerRegistry : private Array< IFuncInvoker* >
	{
	private:
		InvokerRegistry();

		static InvokerRegistry		gInvRegistry;

	public:
		~InvokerRegistry();

		static int				Add(IFuncInvoker* pInvoker);
		static void				Remove(IFuncInvoker* pInvoker);
		static IFuncInvoker*	At(u32 Index);
	};
}

#endif