
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
#ifndef DATAPROPERTY_HPP
#define DATAPROPERTY_HPP

#include "DataProperty.h"
#include "TypeUID.h"
#include <type_traits>
#include <string>
#include <Debug.h>

namespace ks {

	namespace proputil {

		template <typename T = void>	using if_pointer_t = typename std::enable_if< std::is_pointer<T>::value, T>::type;
		template <typename T = void>	using if_value_t = typename std::enable_if< !std::is_pointer<T>::value, T>::type;

		template<typename T> void construct(uintptr_t& dest, T& p_data);

		template<typename T> void construct(uintptr_t& dest, if_pointer_t<T>& p_data)
		{
			dest = (uintptr_t)p_data;
		}

		template<typename T> void construct(uintptr_t& dest, if_value_t<T>& p_data)
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
		}

		void copy_construct(uintptr_t& dest, const uintptr_t& p_source, unsigned size)
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
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TypeStorage
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ITypeStorage
	{
		virtual unsigned TypeID() const = 0;
		virtual unsigned BareTypeID() const = 0;
		virtual unsigned Size() const = 0;
		virtual bool IsPointer() const = 0;
		virtual bool IsNumeric() const = 0;
		virtual bool IsFloat() const = 0;
	};

	template<typename T>
	struct TypeStorage : public ITypeStorage
	{
		unsigned BareTypeID() const override	{ typedef typename bare_type<T>::Type BareType; return TypeUID<BareType>::TypeID(); }
		unsigned TypeID() const override		{ return TypeUID<T>::TypeID(); }
		unsigned Size() const override			{ return sizeof(T); }
		bool IsPointer() const override			{ return std::is_pointer<T>::value; }
		bool IsNumeric() const override			{ return std::_Is_numeric<T>::value; }
		bool IsFloat() const override			{ return std::_Is_floating_point<T>::value; }
	};

	template<typename T>
	ITypeStorage* getTypeStorage()
	{
		typedef typename strip_qualifiers<T>::Type	UQType;
		static TypeStorage<UQType> sTypeStore;
		return &sTypeStore;
	}

	template<typename T>
	DataProperty::DataProperty(T p_data) : m_data(0)
	{
		proputil::construct<T>(m_data, p_data);
		m_traits = getTypeStorage<T>();
	}

	template<typename T>
	DataProperty::DataProperty(T& p_data) : m_data(0)
	{
		proputil::construct<T>(m_data, p_data);
		m_traits = getTypeStorage<T>();
	}

	template<typename T>
	DataProperty::DataProperty(ref_wrap<T> p_data) : m_data(0)
	{
		proputil::copy_construct(m_data, (const uintptr_t)&p_data.get(), sizeof(T*));	// store as pointer and tag appropriately
		m_traits = getTypeStorage<T*>();
	}

	void DataProperty::operator=(const DataProperty& o)
	{
		destroy();
		m_traits = o.m_traits;
		proputil::copy_construct(m_data, o.m_data, m_traits->Size());
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
			KS_ASSERT(0 && "Incompatible type conversion.");
		}

		if (std::is_pointer<T>::value && m_traits->IsPointer())		// stored as pointer, being read as pointer
			return (T&)m_data;
		else if (m_traits->IsPointer())								// stored as pointer or buffer, being read as reference or buffer
			return *(T*)m_data;

		return (T&)m_data;											// default: stored as value type, read as value type
	}

	template<typename T>
	inline bool DataProperty::IsCompatible() const
	{
		typedef typename bare_type<T>::Type BareType;
		if (std::_Is_numeric<T>::value && m_traits->IsNumeric())
		{
			// float to int conversions are incompatible and must be explicit.
			return !m_traits->IsFloat() || m_traits->IsFloat() == std::is_floating_point<T>::value;
		}
		return m_traits->BareTypeID() == TypeUID<BareType>::TypeID();
	}

	void DataProperty::destroy()
	{
		if (IsValid() && !m_traits->IsPointer() && m_traits->Size() > sizeof(uintptr_t))
			free((void*)m_data);

		m_data = 0;
		m_traits = nullptr;
	}

}

#endif