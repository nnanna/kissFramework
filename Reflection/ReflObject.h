
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2017)
///	@about		Class reflection and Delegate interface
///				Allows arbitrary hinding of delegates to class type
///				Client code only required to implement one static method: T::InitBindings(BindingsMap)
///				Easy to use and can bind any number of objects.
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

	// There's only one of this per class type via templated lazy-allocation
	class BindingsMap
	{
		friend class ReflObject;		// clearance to access GetBindings()
	public:
		template <class T>
		inline BindingsMap& Add(const char* pName, T pFunc)
		{
			auto registryHandle = IFuncInvoker::Create(pName, pFunc);
			Add(registryHandle);

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

		// TODO
		// void Optimise(); make local storage of bindings. index and sort hashes for faster access TODO

	private:
		u32		mHead;
		u32		mNumBindings;
		//char*	mAllocationBuffer;	// for possible use with Optimise()

		BindingsMap();

		void Add(IFuncInvoker::RegistryHandle& rHandle);

		template<class T>
		static BindingsMap& GetBindings()
		{
			static char GBindingsBuffer[sizeof(BindingsMap)];
			static BindingsMap* GBindings(nullptr);
			if (GBindings == nullptr)
			{
				GBindings = new (GBindingsBuffer)BindingsMap();
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