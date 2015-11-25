//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2014)
///	@about		Templated auto-generation of typeid and specialisable typenames
///	@references	article "Making your own type id is fun" by Alex Darby
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
#ifndef TYPE_UID_H
#define TYPE_UID_H

#define NAME_MAX	64

typedef kissU32			kissType;

template<typename T>
struct TypeUID
{
	static kissType		TypeID();
	static const char*	Typename();
};

template<typename T>
inline kissType TypeUID<T>::TypeID()				{ static char mID; return kissType(&mID); }


template<>
inline const char* TypeUID<int>::Typename()			{ return "int"; }

template<>
inline const char* TypeUID<bool>::Typename()		{ return "bool"; }

template<>
inline const char* TypeUID<float>::Typename()		{ return "float"; }

template<typename T>
inline const char* TypeUID<T>::Typename()
{
	static char tName[ NAME_MAX ] = { 0 };
	if (tName[0] == 0)
		sprintf_s(tName, NAME_MAX, "%08x", TypeID());

	return tName;
}


#define DECLARE_TYPENAME(TType)					\
template<>										\
inline const char* TypeUID<TType>::Typename()	\
{ return #TType; }								\

#endif