//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2014)
///	@about		Class(es) to ease generating string names for enums.
/// 			experimental.
/// 			recursive hence potentially unsuitable for adequtely large/spaced-out enums. e.g 128 bit flags
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


#ifndef ENUMSTRING_H
#define ENUMSTRING_H

#include "../Common/Debug.h"

template<typename ENUMTYPE>
struct EnumString
{
	template<kiss32 ENUMID>
	static const char* get();
};

#define EXPORTENUMSTRING(EID)							\
	template<> template<>								\
	const char* EnumString<decltype(EID)>::get<EID>()	\
	{ return #EID; }									\


template<kiss32 FROM>
struct EnumLoop
{
	template<typename ENUMTYPE>
	static const char* get(ENUMTYPE pID);
};

#define BEGINENUMSTRING(ETYPE, EID)							\
	DECLARE_TYPENAME(ETYPE)									\
	EXPORTENUMSTRING(EID)									\
template<> template<>										\
inline const char* EnumLoop<EID-1>::get(decltype(EID) pID)	\
{															\
	return "MINIMUS_EXTREMUS";								\
}															\

#define FINISHENUMSTRING(EID)								\
	EXPORTENUMSTRING(EID)									\
template<> template<>										\
inline const char* EnumLoop<EID+1>::get(decltype(EID) pID)	\
{															\
	return "MAXIMUS_EXTREMUS";								\
}															\

template<typename ENUMTYPE> template<kiss32 ENUMID>
const char* EnumString<ENUMTYPE>::get()
{
	KISS_ASSERT(0 && "This enum hasn't been exported/declared");
	return "undeclared";
}

template<kiss32 FROM> template<typename ENUMTYPE>
inline const char* EnumLoop<FROM>::get(ENUMTYPE pID)
{
	if (pID == FROM)
		return EnumString<ENUMTYPE>::get<FROM>();
	else if (pID < FROM)
		return EnumLoop<FROM - 1>::get(pID);
	else
		return EnumLoop<FROM + 1>::get(pID);
}

template<typename ETYPE>
inline const char* enumToString(ETYPE pID)			// recurse to the right templated version.
{
	return EnumLoop<0>::get<ETYPE>(pID);
}

#endif	//ENUMSTRING_H