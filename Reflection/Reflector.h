//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2014)
///	@about		lightweight reflection interface
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
#ifndef REFLECTOR_H
#define REFLECTOR_H

#include "defines.h"

namespace ks{

class IReflector
{
public:
	virtual ~IReflector()	{}
	virtual const char*		ToString(const void* pValue) const = 0;
	virtual const char*		Typename() const = 0;
	virtual ksType			TypeID() const = 0;
};


template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

class Refl
{
public:
	template<typename T>
	Refl(const T& pRHS)			{ mInterface = getReflectInterface<T>(pRHS); mClient = &pRHS; }

	template<typename T>
	Refl(const T* pRHS)			{ mInterface = getReflectInterface<T>(pRHS); mClient = pRHS; }

	~Refl()						{}

	const char*		ToString() const		{ return mInterface->ToString( mClient ); }
	const char*		Typename() const		{ return mInterface->Typename(); }
	ksType			TypeID() const			{ return mInterface->TypeID(); }

	const IReflector* operator->() const	{ return mInterface; }

private:
	IReflector*		mInterface;
	const void*		mClient;

	template<typename T>
	static IReflector* getReflectInterface(const enable_if_t< std::is_enum<T>::value, T>& pRHS);	// enum specialisation

	template<typename T>
	static IReflector* getReflectInterface(const T& pRHS);
};

}	// namespace ks

#endif