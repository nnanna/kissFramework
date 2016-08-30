
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		29/08/2016
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

#include <stdint.h>

namespace ks {

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Beware: may incur excessive runtime allocations with types larger than 32bits. Prefer move semantics
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	struct ref_wrap
	{
		ref_wrap(T& ref) : m_ref(ref)
		{}
		T& get()	{ return m_ref; }
	private:
		T& m_ref;
	};

	template<typename T>
	ref_wrap<T> ref(T& o)	{ return ref_wrap<T>(o); }

	class DataProperty
	{
	public:
		template<typename T>
		DataProperty(T& p_data);

		template<typename T>
		DataProperty(ref_wrap<T>& p_data);

		void operator=(const DataProperty& o);
		void operator=(DataProperty&& o);

		~DataProperty();

		template<typename T>
		operator T&();

		template<typename T>
		T& As();

		template<typename T>
		bool IsCompatible() const;

	private:
		uintptr_t				m_data;
		uint16_t				m_size;
		struct ITypeStorage*	m_typeid;

		void		destroy();
	};


}