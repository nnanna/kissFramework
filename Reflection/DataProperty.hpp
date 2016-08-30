
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

#include "DataProperty.h"
#include "TypeUID.h"
#include <type_traits>
#include <Debug.h>

namespace ks {

	namespace proputil {

		template <typename T = void>	using if_pointer_t = typename std::enable_if< std::is_pointer<T>::value, T>::type;
		template <typename T = void>	using if_value_t = typename std::enable_if< !std::is_pointer<T>::value, T>::type;

		template<typename T> void construct(uintptr_t& dest, unsigned& dest_size, T& p_data);

		template<typename T> void construct(uintptr_t& dest, unsigned& dest_size, if_pointer_t<T>& p_data)
		{
			dest = (uintptr_t)p_data;
			dest_size = 0;				// make pointers zero-sized mainly for identification/type checking
		}

		template<typename T> void construct(uintptr_t& dest, uint16_t& dest_size, if_value_t<T>& p_data)
		{
			const unsigned size = sizeof(p_data);
			if (size > sizeof(uintptr_t))
			{
				static_assert( std::is_trivially_copyable<T>::value, "unsupported for non-trival types" );
				dest = (uintptr_t)malloc(size);
				memcpy_s((void*)dest, size, &p_data, size);
			}
			else
			{
				(T&)dest = p_data;
			}
			dest_size = size;
		}

		void copy_construct(uintptr_t& dest, uint16_t& dest_size, const uintptr_t& p_source, uint16_t size)
		{
			if (size > sizeof(uintptr_t))
			{
				dest = (uintptr_t)malloc(size);
				memcpy_s((void*)dest, size, (void*)p_source, size);
			}
			else
			{
				dest = p_source;
			}
			dest_size = size;
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TypeStorage
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ITypeStorage
	{
		virtual unsigned TypeID() const = 0;
	};

	template<typename T>
	struct TypeStorage : public ITypeStorage
	{
		unsigned TypeID() const override { return TypeUID<T>::TypeID(); }
	};

	template<typename T>
	ITypeStorage* getTypeStorage()
	{
		typedef typename bare_type<T>::Type	BareType;
		static TypeStorage< BareType > sTypeStore;
		return &sTypeStore;
	}


	template<typename T>
	DataProperty::DataProperty(T& p_data) : m_data(0), m_size(0)
	{
		proputil::construct<T>(m_data, m_size, p_data);
		m_typeid = getTypeStorage<T>();
	}

	template<typename T>
	DataProperty::DataProperty(ref_wrap<T>& p_data) : m_data(0), m_size(0)
	{
		proputil::copy_construct(m_data, m_size, (const uintptr_t)&p_data.get(), sizeof(T*));	// store as pointer and tag appropriately
		m_size = 0;
		m_typeid = getTypeStorage<T>();
	}

	void DataProperty::operator=(const DataProperty& o)
	{
		destroy();
		proputil::copy_construct(m_data, m_size, o.m_data, o.m_size);
		m_typeid = o.m_typeid;
	}

	void DataProperty::operator=(DataProperty&& o)
	{
		if (this != &o)
		{
			destroy();
			m_data		= o.m_data;
			m_size		= o.m_size;
			m_typeid	= o.m_typeid;
			o.m_data	= o.m_size = 0;
		}
	}

	DataProperty::~DataProperty()
	{
		destroy();
	}

	template<typename T>
	inline DataProperty::operator T&()
	{
		return As<T>();
	}

	template<typename T>
	inline T& DataProperty::As()
	{
		if ( !IsCompatible<T>() )
		{
			KS_ASSERT(0 && "Incompatible types");
		}

		if (std::is_pointer<T>::value && m_size == 0)			// stored as pointer, being read as pointer
			return (T&)m_data;
		else if (m_size == 0 || m_size > sizeof(uintptr_t))		// stored as pointer or buffer, being read as reference or buffer
			return *(T*)m_data;

		return (T&)m_data;										// default: stored as value type, read as value type
	}

	template<typename T>
	inline bool DataProperty::IsCompatible() const
	{
		typedef typename bare_type<T>::Type	BareType;
		return m_typeid->TypeID() == TypeUID< BareType >::TypeID()
			|| ( std::_Is_numeric < BareType >::value && m_size <= sizeof(T) );		// is a numeric value type
	}

	void DataProperty::destroy()
	{
		if (m_size > sizeof(uintptr_t))
			free((void*)m_data);

		m_data = m_size = 0;
	}

}