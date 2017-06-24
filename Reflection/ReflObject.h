
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2017)
///	@about		Class reflection and Delegate interface
///				Allows arbitrary hinding of delegates to class type
///				Client code only required to implement one static method and one static member.
///				T::InitBindings(BindingsMap) and T::MaxNumBindings
///				compile-time allocated. easy to use and can bind any number of objects.
///				Class methods can be bound via BindingMap::Add() or ADD_BINDING macro.
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

namespace ks {

	// There's only one of this per class type via templated lazy-allocation in GetBindings().
	class BindingsMap
	{
		friend class ReflObject;
	public:
		template <class T>
		inline BindingsMap& Add(const char* pName, T pFunc)
		{
			if (mCaret < mNumBindings)
			{
				IFuncInvoker::PlacementBuffer placementBuffer;
				IFuncInvoker::Create(pName, pFunc, placementBuffer);
				Add(pName, placementBuffer);
			}
			else
			{
				KS_ASSERT(0 && "Bindings Full! T::MaxNumBindings needs to be higher");
			}
			return *this;
		}

		//
		// for easier binding of methods
		// to use this macro, just [typedef <YOURCLASS> CLASSTYPEDEF;] at the top of T::InitBindings() body
		//
#define ADD_BINDING(func)	Add(#func, &CLASSTYPEDEF::func)

		u32 GetHash(const char* pName) const;

		IFuncInvoker* Find(const char* pName) const;

		IFuncInvoker* Find(u32 pHash) const;

		IFuncInvoker* At(u32 index) const;

	private:
		char*	mAllocationBuffer;
		u32*	mHashes;
		u32		mNumBindings;
		u32		mCaret;

		BindingsMap(char* pAllocationBuffer, u32* pHashBuffer, u32 pAllocationBufferSize);

		void Add(const char* pName, IFuncInvoker::PlacementBuffer& pDelegate);

		template<class T>
		static BindingsMap& GetBindings()
		{
			static char GBindingsBuffer[sizeof(BindingsMap)];
			static char GBindingsBufferData[T::MaxNumBindings * FUNC_INVOKER_SIZE];
			static u32 GHashBuffer[T::MaxNumBindings];
			static BindingsMap* GBindings(nullptr);
			if (GBindings == nullptr)
			{
				GBindings = new (GBindingsBuffer)BindingsMap(GBindingsBufferData, GHashBuffer, T::MaxNumBindings);
				T::InitBindings(*GBindings);
			}

			return *GBindings;
		}
	};

	class ReflObject
	{
	public:
		ReflObject() = delete;

		ReflObject(ReflObject& o);

		template<class T>
		ReflObject(T& pObject) : mObject(&pObject), mBindings(&BindingsMap::GetBindings<T>())
		{}

		~ReflObject()
		{}

		void operator=(const ReflObject& o);

		const char* Typename() const;

		u32 TypeID() const;

		Any Call(const char* pFuncName);
		Any Call(const char* pFuncName, Any&& arg1);
		Any Call(const char* pFuncName, Any&& arg1, Any&& arg2);

	private:
		void*			mObject;
		BindingsMap*	mBindings;
	};

}