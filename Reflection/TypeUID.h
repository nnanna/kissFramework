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


namespace ks {
	

	template<typename T>
	struct TypeUID
	{
		static unsigned		TypeID();
		static const char*	Typename();
	};


	static void extract_classname_range(const char* fname, unsigned& begin, unsigned& end)
	{
		unsigned reentrants(0);
		begin = end = 0;
		const char* itr = fname;
		while (*itr != '\0' && end == 0)
		{
			if (*itr == '<')
			{
				if (begin == 0)
					begin = (itr - fname) + 1;
				else
					++reentrants;
			}

			if (*itr == '>' && reentrants-- == 0)
			{
				end = itr - fname;
			}
			++itr;
		}
	}

	template<typename T>
	inline const char* TypeUID<T>::Typename()
	{
#define MAX_TYPENAME 127
		static char tName[MAX_TYPENAME + 1] = { 0 };
		if (tName[0] == 0)
		{
			unsigned begin, end;
			const char* fname = __FUNCTION__;
			extract_classname_range(fname, begin, end);

			unsigned size = end - begin;
			if (size > MAX_TYPENAME)
				size = MAX_TYPENAME;
			memcpy(tName, fname + begin, size);
		}

		return tName;
	}

	template<typename T>
	inline unsigned TypeUID<T>::TypeID()
	{
#define FNV_PRIME1	2166136261
#define FNV_PRIME2	16777619
		static unsigned sUID(FNV_PRIME1);
		if (sUID == FNV_PRIME1)
		{
			const char* name = Typename();
			const char* itr = name;
			while (*itr != '\0')
			{
				sUID = (sUID ^ *itr) * FNV_PRIME2;
				++itr;
			}
		}
		return sUID;
	}



	struct UIDGenerator
	{
		UIDGenerator();

		unsigned Get(const unsigned mask = 0xfffffff);		// masking allows indirect support of UIDs that are less than 32bit
		unsigned GetAsync(const unsigned mask = 0xffffffff);

		static const unsigned INVALID_UID;

	private:
		volatile unsigned mMarker;
	};


	template<class T>
	class InstanceUIDGenerator
	{
	public:
		static unsigned Get(const unsigned mask = 0xffffffff)			{ return mGenerator.Get(mask); }
		static unsigned GetAsync(const unsigned mask = 0xffffffff)		{ return mGenerator.GetAsync(mask); }

	protected:
		static UIDGenerator	mGenerator;
	};

	template<class T>
	UIDGenerator InstanceUIDGenerator<T>::mGenerator;


} // namespace ks


#endif