//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		28/06/2015
///	@about		A fast lightweight alternative to std::vector
/// @credit		based off http://bitsquid.blogspot.co.uk/2012/11/bitsquid-foundation-library.html array.h, with modifications
///	@references	http://www.drdobbs.com/generictyped-buffers-i/184403791 | http://upcoder.com/3/roll-your-own-vector/
///				http://www.drdobbs.com/generic-typed-buffers-ii/184403799
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

#include <type_traits>
#include <xmemory>

#ifndef KS_ARRAY
#define KS_ARRAY


namespace ks {

	namespace details
	{
		// use SFINAE to optimise trivially copyable types
		template <typename T = void>
		using if_copyable_t = typename std::enable_if< std::is_trivially_copyable<T>::value, T>::type;

		template <typename T = void>
		using if_non_copyable_t = typename std::enable_if< !std::is_trivially_copyable<T>::value, T>::type;
		
		template<typename T>
		void copy_into( if_copyable_t<T> *& pDest, const T* pSource, const size_t pSize )
		{
			memcpy(pDest, pSource, sizeof(T)*pSize);
		}

		template<typename T>
		void copy_into( if_non_copyable_t<T> *& pDest, const T* pSource, const size_t pSize )
		{
			size_t i = 0;
			switch ((pSize - i) & 7)	// Duff's device
			{
			case 0:
				while (i != pSize)
				{
					pDest[ i ] = pSource[ i++ ];
			case 7: pDest[ i ] = pSource[ i++ ];
			case 6: pDest[ i ] = pSource[ i++ ];
			case 5: pDest[ i ] = pSource[ i++ ];
			case 4: pDest[ i ] = pSource[ i++ ];
			case 3: pDest[ i ] = pSource[ i++ ];
			case 2: pDest[ i ] = pSource[ i++ ];
			case 1: pDest[ i ] = pSource[ i++ ];
				}
			}
		}

		template<typename T, class TAllocator>
		void allocate( if_copyable_t<T> *& pDest, const size_t pSize, TAllocator& pAllocator)
		{
			pDest = (T *)pAllocator.allocate( sizeof(T) * pSize );
		}

		template<typename T, class TAllocator>
		void allocate( if_non_copyable_t<T> *& pDest, const size_t pSize, TAllocator& pAllocator)
		{
			pDest = (T *)pAllocator.allocate( sizeof(T) * pSize );
			for ( size_t i = 0; i < pSize; ++i )
				pAllocator.construct( pDest + i, T() );
		}
		

		template<typename T, class TAllocator>
		void deallocate(if_copyable_t<T> *& pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{
			pAllocator.deallocate(pBegin, pCapacity);
		}

		template<typename T, class TAllocator>
		void deallocate( if_non_copyable_t<T> *& pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{
			for ( size_t i = 0; i < pCapacity; ++i )
				pAllocator.destroy( pBegin + i );
			
			pAllocator.deallocate(pBegin, pCapacity);
		}

		template<typename T>
		void duff_fill(T* pDest, const T& pFillValue, const size_t pSize)
		{
			size_t i = 0;
			switch ((pSize - i) & 7)
			{
			case 0:
				while (i != pSize)
				{
					pDest[ i++ ] = pFillValue;
			case 7: pDest[ i++ ] = pFillValue;
			case 6: pDest[ i++ ] = pFillValue;
			case 5: pDest[ i++ ] = pFillValue;
			case 4: pDest[ i++ ] = pFillValue;
			case 3: pDest[ i++ ] = pFillValue;
			case 2: pDest[ i++ ] = pFillValue;
			case 1: pDest[ i++ ] = pFillValue;
				}
			}
		}
	}


	template<typename T, class TAllocator = std::allocator<T> >
	class Array 
	{
#define KS_ARRAY_MOVE_SEMANTICS_ONLY	1

#if _DEBUG
	#define CHECK_OUT_OF_BOUNDS( x )	if( (x) >= _capacity ) { __debugbreak(); /*KS_ASSERT( "Array out of bounds detected" );*/ }
#else
	#define CHECK_OUT_OF_BOUNDS( x )
#endif

public:
	typedef typename T*			iterator;
	typedef typename const T*	const_iterator;

		size_t size() const		{ return _size;			}
		bool any() const 		{ return _size != 0;	}
		bool empty() const		{ return _size == 0;	}

		T* begin() 				{ return _begin;			}
		const T* begin() const	{ return _begin;			}
		T* end() 				{ return _begin + _size;	}
		const T* end() const	{ return _begin + _size;	}

		T& front() 				{ CHECK_OUT_OF_BOUNDS(0); return _begin[0];			}
		const T& front() const	{ CHECK_OUT_OF_BOUNDS(0); return _begin[0];			}
		T& back() 				{ CHECK_OUT_OF_BOUNDS(_size - 1); return _begin[_size-1];	}
		const T& back() const	{ CHECK_OUT_OF_BOUNDS(_size - 1); return _begin[_size - 1]; }

		void pop_back()			{ CHECK_OUT_OF_BOUNDS(_size - 1); --_size; }

		void clear()			{ resize(0);}
		void trim()				{ set_capacity(_size); }

		void resize(size_t new_size)
		{
			if (new_size > _capacity)
				grow(new_size);
			_size = new_size;
		}

		void resize(size_t new_size, const T& pFillValue )
		{
			const size_t prev_size = _size;
			resize( new_size );

			details::duff_fill(_begin + prev_size, pFillValue, _size);
		}

		void reserve(size_t new_capacity)
		{
			if (new_capacity > _capacity)
				set_capacity(new_capacity);
		}

		void set_capacity(size_t new_capacity)
		{
			if (new_capacity == _capacity)
				return;

			if (new_capacity < _size)
				resize(new_capacity);

			T *new_data = 0;
			if (new_capacity > 0)
			{
				details::allocate<T>( new_data, new_capacity, _allocator );
				details::copy_into( new_data, _begin, _size );		// TODO: move semantics if_non_trivially_copyable?
			}
			details::deallocate<T>(_begin, _capacity, _allocator);
			_begin		= new_data;
			_capacity	= new_capacity;
		}

		void grow(size_t min_capacity = 0)
		{
			size_t new_capacity = _capacity * 2 + 8;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			set_capacity(new_capacity);
		}

		void push_back(const T &item)
		{
			push_back( std::move(T(item)) );
		}

		void push_back(T&& item)
		{
			if (_size + 1 > _capacity)
				grow();
			_begin[_size++] = std::move(item);
		}


		Array() : _begin(nullptr), _size(0), _capacity(0), _allocator( TAllocator() )
		{}

		Array( size_t pCapacity ) : _begin(nullptr), _size(0), _capacity(0), _allocator( TAllocator() )
		{
			set_capacity( pCapacity );
		}

		~Array()
		{
			details::deallocate<T>(_begin, _capacity, _allocator);
		}

#if !KS_ARRAY_MOVE_SEMANTICS_ONLY
		Array(const Array &other) : _allocator(other._allocator), _size(0), _capacity(0), _begin(nullptr)
		{
			const size_t n = other._size;
			set_capacity(n);
			details::copy_into(_begin, other.begin_, sizeof(T) * n );
			_size = n;
		}

		Array &operator=(const Array &other)
		{
			const size_t n = other._size;
			resize(n);
			details::copy_into(_begin, other.begin_, sizeof(T)*n );
			return *this;
		}
#else
		Array(const Array &other) = delete;
		Array &operator=(const Array &other) = delete;
#endif

		Array(Array &&other)
			: _begin(other._begin)
			, _size(other._size)
			, _capacity(other._capacity)
		{
			_allocator		= std::move( other._allocator );

			other._begin	= nullptr;
			other._size		= 0;
			other._capacity	= 0;
		}

		Array &operator=(Array &&other)
		{
			if ( this != &other )
			{
				details::deallocate<T>(_begin, _capacity, _allocator);

				_begin		= other._begin;
				_size		= other._size;
				_capacity	= other._capacity;
				_allocator	= std::move( other._allocator );

				other._begin	= nullptr;
				other._capacity	= 0;
				other._size		= 0;
			}
			return *this;
		}

		T & operator[](size_t i)				{ CHECK_OUT_OF_BOUNDS( i ); return _begin[i]; }

		const T & operator[](size_t i) const	{ CHECK_OUT_OF_BOUNDS( i ); return _begin[i]; }


	private:
		T*			_begin;
		size_t		_size;
		size_t		_capacity;
		TAllocator	_allocator;		// don't use directly, rather redirect via details::
	};


}	//namespace ks


#endif	// KS_ARRAY
