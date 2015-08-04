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


#ifndef KS_ARRAY
#define KS_ARRAY

#include <type_traits>
#include <xmemory>

namespace ks {

#if _DEBUG
	#define KS_ASSERT( cond )		{ if( (cond)  == false ) __debugbreak(); }
#else
	#define KS_ASSERT( cond )
#endif

	namespace details
	{
		// use SFINAE to optimize trivially copyable types
		template <typename T = void>
		using if_copyable_t = typename std::enable_if< std::is_trivially_copyable<T>::value, T>::type;

		template <typename T = void>
		using if_non_copyable_t = typename std::enable_if< !std::is_trivially_copyable<T>::value, T>::type;

		template<typename T>
		void copy_into( if_copyable_t<T> *& pDest, const T* pSource, const size_t pSize )
		{
			memcpy(pDest, pSource, sizeof(T) * pSize);
		}

		template<typename T>
		void copy_into( if_non_copyable_t<T> *& pDest, T* pSource, const size_t pSize )
		{
			size_t i = 0;
			switch ((pSize - i) & 7)	// Duff's device
			{
			case 0:
				while (i != pSize)
				{
					pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 7: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 6: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 5: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 4: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 3: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 2: pDest[ i ] = std::move( pSource[ i ] ); ++i;
			case 1: pDest[ i ] = std::move( pSource[ i ] ); ++i;
				}
			}
		}

		template<typename T, class TAllocator>
		void allocate( if_copyable_t<T> *& pDest, const size_t pSize, TAllocator& pAllocator)
		{
			pDest = pAllocator.allocate( pSize );
		}

		template<typename T, class TAllocator>
		void allocate( if_non_copyable_t<T> *& pDest, const size_t pSize, TAllocator& pAllocator)
		{
			pDest = pAllocator.allocate( pSize );
			for ( size_t i = 0; i < pSize; ++i )
				pAllocator.construct( pDest + i, T() );
		}


		template<typename T, class TAllocator>
		void deallocate( if_copyable_t<T> *& pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{
			pAllocator.deallocate(pBegin, pCapacity);
		}

		template<typename T, class TAllocator>
		void deallocate( if_non_copyable_t<T>*& pBegin, const size_t pCapacity, TAllocator& pAllocator)
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

#if _DEBUG
#define itr_checkContainerMismatch(x)				\
	if( getAxis() != x.getAxis() )					\
	{												\
		KS_ASSERT( 0 && "Container mis-match!" );	\
	}												\

#define itr_setBegin(x)		mBegin(x),
#define itr_mBegin			mBegin
#define itr_copyBegin(x)	mBegin = x.mBegin
#else
	#define itr_checkContainerMismatch(x)

#define itr_setBegin(x)
#define itr_mBegin			nullptr
#define itr_copyBegin(x)
#endif

	template<typename T>
	struct ReverseIterator
	{
		ReverseIterator(T* pBegin) : mBegin(pBegin), mEnd(nullptr)
		{}
		ReverseIterator( T* pBegin, T* pEnd ) : mBegin(pBegin), mEnd(nullptr)
		{
			if(pBegin && pEnd > pBegin)
			{
				mBegin	= pBegin;
				mEnd	= pEnd - 1;
			}
		}

		bool operator==( const ReverseIterator& pOther ) const	{ itr_checkContainerMismatch(pOther); return mBegin == pOther.mBegin && mEnd == pOther.mEnd; }
		bool operator!=( const ReverseIterator& pOther ) const	{ return !operator==(pOther); }

		T& operator->()					{ return *mEnd; }
		const T& operator->() const		{ return *mEnd; }
		T& operator*()					{ return *mEnd; }
		const T& operator*() const		{ return *mEnd; }

		ReverseIterator& operator++()
		{
			if (mEnd <= mBegin)
			{
				mEnd = nullptr;
			}
			else
			{
				--mEnd;
			}
			return *this;
		}

		ReverseIterator& operator++(int)
		{
			return operator++();
		}

		ReverseIterator& operator+(const int pCount)
		{
			mEnd = mEnd - pCount;
#if _DEBUG
			if ( mEnd < mBegin || mEnd > (mEnd + pCount) )
			{
				KS_ASSERT( 0 && "Bounds exceeded" );
				mEnd = mBegin;
			}
#endif
			return *this;
		}

	private:
#if _DEBUG
		size_t getAxis() const		{ return size_t(mBegin); }
#endif
		T* mBegin;
		T* mEnd;
	};


	template<typename T>
	class ForwardIterator
	{
#if _DEBUG
#define ITR_CHECK_OUT_OF_BOUNDS( x )	if( mEnd < (mCaret + (x)) || (mCaret + (x)) < mBegin  ) { KS_ASSERT( 0 && "iterator out of bounds!" ); }
#else
#define ITR_CHECK_OUT_OF_BOUNDS( x )
#endif
	public:
		typedef std::forward_iterator_tag	iterator_category;
		typedef T							value_type;
		typedef uint32_t					difference_type;
		typedef uint32_t					size_type;
		typedef value_type*					pointer;
		typedef const value_type*			const_pointer;
		typedef value_type&					reference;
		typedef const value_type&			const_reference;

		ForwardIterator() : itr_setBegin(nullptr) mCaret(nullptr), mEnd(nullptr)
		{}
		ForwardIterator(std::nullptr_t) : itr_setBegin(nullptr) mCaret(nullptr), mEnd(nullptr)
		{}
		ForwardIterator( pointer pBegin, pointer pCaret, pointer pEnd ) : itr_setBegin(pBegin) mCaret(pCaret), mEnd(pEnd)
		{}

		reference operator[](size_type i)								{ ITR_CHECK_OUT_OF_BOUNDS( i ); return mCaret[i]; }
		const_reference operator[](size_type i) const					{ ITR_CHECK_OUT_OF_BOUNDS( i ); return mCaret[i]; }

		ForwardIterator& operator=( const ForwardIterator& pOther )		{ return copy(pOther); }

		bool operator==( const ForwardIterator& pOther ) const			{ itr_checkContainerMismatch(pOther); return mCaret == pOther.mCaret; }
		bool operator!=( const ForwardIterator& pOther ) const			{ return !operator==(pOther); }

		pointer operator->()											{ return mCaret; }
		const_pointer operator->() const								{ return mCaret; }
		reference operator*()											{ return *mCaret; }
		const_reference operator*() const								{ return *mCaret; }

		ForwardIterator& operator++()									{ increment(); return *this; }
		ForwardIterator operator++(int)									{ ForwardIterator prev(*this); increment(); return prev; }

		ForwardIterator& operator--()									{ decrement(); return *this; }
		ForwardIterator operator--(int)									{ ForwardIterator prev(*this); decrement(); return prev; }

		ForwardIterator& operator+=(const int pCount)					{ ITR_CHECK_OUT_OF_BOUNDS( pCount ); mCaret += pCount; return *this; }

		ForwardIterator& operator-=(const int pCount)					{ ITR_CHECK_OUT_OF_BOUNDS(-pCount ); mCaret -= pCount; return *this; }

		ForwardIterator operator+(const int pCount) const				{ ITR_CHECK_OUT_OF_BOUNDS( pCount ); return ForwardIterator( itr_mBegin, mCaret + pCount, mEnd ); }

		ForwardIterator operator-(const int pCount) const				{ ITR_CHECK_OUT_OF_BOUNDS(-pCount ); return ForwardIterator( itr_mBegin, mCaret - pCount, mEnd ); }

		difference_type operator-(const ForwardIterator& pRHS) const	{ itr_checkContainerMismatch(pRHS); return (mCaret - pRHS.mCaret); }
		bool operator<(const ForwardIterator& pRHS) const				{ itr_checkContainerMismatch(pRHS); return mCaret < pRHS.mCaret; }
		bool operator>(const ForwardIterator& pRHS) const				{ itr_checkContainerMismatch(pRHS); return mCaret > pRHS.mCaret; }

	private:

		ForwardIterator& copy( const ForwardIterator& pOther )
		{
			mCaret	= pOther.mCaret;
			mEnd	= pOther.mEnd;
			itr_copyBegin( pOther );

			return *this;
		}
		void increment()
		{
			ITR_CHECK_OUT_OF_BOUNDS( 1 );
			++mCaret;
		}

		void decrement()
		{
			ITR_CHECK_OUT_OF_BOUNDS( -1 );
			--mCaret;
		}

#if _DEBUG
		size_type getAxis() const		{ return size_type(mBegin); }
		pointer	mBegin;
#endif
		pointer	mCaret;
		pointer	mEnd;
	};


	template<typename T, class TAllocator = std::allocator<T> >
	class Array 
	{
#define KS_ARRAY_MOVE_SEMANTICS_ONLY	1		// Before turning this off, have you considered using std::vector? :P

#define KS_USE_FORWARD_ITERATOR_CLASS	_DEBUG

#if _DEBUG
#define CHECK_OUT_OF_BOUNDS( x )	if( (x) >= _size ) { KS_ASSERT( 0 && "Array out of bounds detected" ); }
#else
#define CHECK_OUT_OF_BOUNDS( x )
#endif

	public:
#if KS_USE_FORWARD_ITERATOR_CLASS
		typedef typename ForwardIterator<T>			iterator;
		typedef typename ForwardIterator<T>			const_iterator;		// const*/ ForwardIterator<T>

#else
		typedef typename T*							iterator;
		typedef typename const T*					const_iterator;

#endif
		typedef T									value_type;
		typedef T*									pointer;
		typedef const T*							const_pointer;
		typedef T&									reference;
		typedef const T&							const_reference;
		typedef uint32_t							size_type;
		typedef ptrdiff_t							difference_type;

		typedef typename ReverseIterator<T>			reverse_iterator;
		typedef typename ReverseIterator<T>			const_reverse_iterator;

		size_type size() const			{ return _size;			}
		size_type capacity() const		{ return _capacity;		}
		bool any() const 				{ return _size != 0;	}
		bool empty() const				{ return _size == 0;	}

#if KS_USE_FORWARD_ITERATOR_CLASS
		iterator begin() 				{ return iterator(_begin, _begin, _begin + _size);					}
		iterator end() 					{ return iterator(_begin, _begin + _size, _begin + _size );			}
		const_iterator begin() const	{ return const_iterator(_begin, _begin, _begin + _size);			}
		const_iterator end() const		{ return const_iterator(_begin, _begin + _size, _begin + _size);	}
#else
		iterator begin() 				{ return _begin;			}
		iterator end() 					{ return _begin + _size;	}
		const_iterator begin() const	{ return _begin;			}
		const_iterator end() const		{ return _begin + _size;	}
#endif

		pointer data()					{ return _begin; }
		const pointer data() const		{ return _begin; }

		reverse_iterator rbegin() const	{ return reverse_iterator(_begin, _begin + _size);	}
		reverse_iterator rend() const	{ return reverse_iterator(_begin);					}

		reference front() 				{ CHECK_OUT_OF_BOUNDS(0); return _begin[0];			}
		const_reference front() const	{ CHECK_OUT_OF_BOUNDS(0); return _begin[0];			}
		reference back() 				{ CHECK_OUT_OF_BOUNDS(_size - 1); return _begin[_size - 1];	}
		const_reference back() const	{ CHECK_OUT_OF_BOUNDS(_size - 1); return _begin[_size - 1]; }

		void pop_back()					{ CHECK_OUT_OF_BOUNDS(_size - 1); --_size; }

		void clear()					{ resize(0);}
		void trim()						{ set_capacity(_size); }

		void swap(Array& other)
		{
			auto obegin = other._begin;
			auto osize	= other._size;
			auto ocap	= other._capacity;

			other._begin	= _begin;
			other._size		= _size;
			other._capacity	= _capacity;

			_begin			= obegin;
			_size			= osize;
			_capacity		= ocap;
		}

		void resize(size_type new_size)
		{
			if (new_size > _capacity)
				grow(new_size);
			_size = new_size;
		}

		void resize(size_type new_size, const_reference pFillValue )
		{
			const size_type prev_size = _size;
			resize( new_size );

			if( _size > prev_size )
				details::duff_fill(_begin + prev_size, pFillValue, _size - prev_size );
		}

		void reserve(size_type new_capacity)
		{
			if (new_capacity > _capacity)
				set_capacity(new_capacity);
		}

		void push_back(const_reference item)		{ emplace_back( std::move(value_type(item)) ); }

		void push_back(value_type&& item)			{ emplace_back(std::move(item)); }

		void emplace_back(value_type&& item)
		{
			if (_size + 1 > _capacity)
				grow();
			_begin[_size++] = std::move(item);
		}

		iterator erase( iterator item )
		{
			const size_type new_size = _size - 1;
			CHECK_OUT_OF_BOUNDS( new_size );
			iterator next( item );

			while (item < end() - 1)
			{
				*item = *(item + 1);
				++item;
			}
			resize( new_size );

#if KS_USE_FORWARD_ITERATOR_CLASS
			return iterator( _begin, &(*next), _begin + _size );
#else
			return next;
#endif
		}

		iterator insert( iterator pPos, const_reference pValue )
		{
			const size_type diff = pPos - begin();
			resize( _size + 1 );
			CHECK_OUT_OF_BOUNDS( diff );
			
			pPos = begin() + diff;

			for ( auto i = end() - 1; i > pPos; --i )
			{
				*i = std::move( *(i - 1) );
			}

			*pPos	= std::move(value_type(pValue));

			return pPos;
		}


		Array() : _begin(nullptr), _size(0), _capacity(0), _allocator( TAllocator() )
		{}

		Array( size_type pCapacity ) : _begin(nullptr), _size(0), _capacity(0), _allocator( TAllocator() )
		{
			set_capacity( pCapacity );
		}
		Array( size_type pCapacity, const_reference pFillValue ) : _begin(nullptr), _size(0), _capacity(0), _allocator( TAllocator() )
		{
			resize( pCapacity, pFillValue );
		}

		~Array()
		{
			details::deallocate<value_type>(_begin, _capacity, _allocator);
		}

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
				details::deallocate<value_type>(_begin, _capacity, _allocator);

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
		reference operator[](size_type i)				{ CHECK_OUT_OF_BOUNDS( i ); return _begin[i]; }
		const_reference operator[](size_type i) const	{ CHECK_OUT_OF_BOUNDS( i ); return _begin[i]; }

		reference at(size_type i)						{ return operator[](i); }
		const_reference at(size_type i) const			{ return operator[](i); }

#if !KS_ARRAY_MOVE_SEMANTICS_ONLY

		Array(const Array &other) : _allocator(other._allocator), _size(0), _capacity(0), _begin(nullptr)
		{
			set_capacity( other._capacity );
			resize( other._size );
			details::copy_into(_begin, other._begin, other._size );
		}


		Array &operator=(const Array &other)
		{
			const size_type n = other._size;
			resize(n);
			details::copy_into(_begin, other._begin, n );
			return *this;
		}
#else
		void explicit_copy( const Array & pSource )
		{
			const size_type n = pSource._size;
			resize(n);
			details::copy_into( _begin, pSource._begin, n );
		}

		Array(const Array &) = delete;
		Array &operator=(const Array &) = delete;
#endif

	private:
		pointer		_begin;
		size_type	_size;
		size_type	_capacity;
		TAllocator	_allocator;		// don't use directly, rather redirect via details::


		void grow(size_type min_capacity = 0)
		{
			size_type new_capacity = _capacity + (_capacity >> 2) + 8;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			set_capacity(new_capacity);
		}

		void set_capacity(size_type new_capacity)
		{
			if (new_capacity == _capacity)
				return;

			if (new_capacity < _size)
				resize(new_capacity);

			pointer new_data = 0;
			if (new_capacity > 0)
			{
				details::allocate<value_type>( new_data, new_capacity, _allocator );
				details::copy_into( new_data, _begin, _size );
			}
			details::deallocate<value_type>(_begin, _capacity, _allocator);
			_begin		= new_data;
			_capacity	= new_capacity;
		}
	};


}	//namespace KS


#endif	// KS_ARRAY