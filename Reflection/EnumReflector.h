//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2014)
/// @usage		SomeEnum pstate	= eWalking; Refl( pstate ).ToString(); Refl( pstate ).Typename();
/// @usage2		EnumReflector<SomeEnum> refl; SomeEnum pstate = eFlossing; refl.ToString(&pstate);
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

#ifndef ENUM_REFLECTOR_H
#define ENUM_REFLECTOR_H

#include "Reflector.h"
#include "EnumString.h"

namespace ks {

	template<typename T>
	class EnumReflector : public IReflector
	{
	public:
		EnumReflector();
		~EnumReflector();

		const char*		ToString(const void* pValue) const override;
		const char*		Typename() const override;
		ksType			TypeID() const override;
	};

	template<typename T>
	EnumReflector<T>::EnumReflector()
	{}

	template<typename T>
	EnumReflector<T>::~EnumReflector()
	{}

	template<typename T>
	const char* EnumReflector<T>::ToString(const void* pValue) const
	{
		return enumToString(*static_cast<const T*>(pValue));
	}

	template<typename T>
	ksType EnumReflector<T>::TypeID() const		{ return TypeUID<T>::TypeID(); }

	template<typename T>
	const char* EnumReflector<T>::Typename() const	{ return TypeUID<T>::Typename(); }



	/////////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////////////////////
	template<typename T>
	IReflector* Refl::getReflectInterface(const enable_if_t< std::is_enum<T>::value, T>& pRHS)
	{
		static EnumReflector<T> enumReflect;
		return &enumReflect;
	}

}	// namespace ks

#endif