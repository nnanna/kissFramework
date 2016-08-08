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

#include <Debug.h>
#include <type_traits>
#include <xmemory>

namespace ks {

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
					pDest[ i ] = pSource[ i ]; ++i;
			case 7: pDest[ i ] = pSource[ i ]; ++i;
			case 6: pDest[ i ] = pSource[ i ]; ++i;
			case 5: pDest[ i ] = pSource[ i ]; ++i;
			case 4: pDest[ i ] = pSource[ i ]; ++i;
			case 3: pDest[ i ] = pSource[ i ]; ++i;
			case 2: pDest[ i ] = pSource[ i ]; ++i;
			case 1: pDest[ i ] = pSource[ i ]; ++i;
				}
			}
		}

		template<typename T>
		void move_into( if_copyable_t<T> *& pDest, const T* pSource, const size_t pSize )
		{
			memcpy( pDest, pSource, sizeof(T) * pSize );
		}

		template<typename T>
		void move_into( if_non_copyable_t<T> *& pDest, T* pSource, const size_t pSize )
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
		void allocate( T*& pDest, const size_t pSize, TAllocator& pAllocator)
		{
			pDest = pAllocator.allocate( pSize );
		}


		template<typename T, class TAllocator>
		void construct( if_copyable_t<T> * pDest, const size_t pSize, TAllocator& pAllocator )
		{}

		template<typename T, class TAllocator>
		void construct( if_non_copyable_t<T> * pDest, const size_t pSize, TAllocator& pAllocator )
		{
			size_t i = 0;
			switch ((pSize - i) & 7)
			{
			case 0:
				while (i != pSize)
				{
					pAllocator.construct( pDest + i ); ++i;
			case 7: pAllocator.construct( pDest + i ); ++i;
			case 6: pAllocator.construct( pDest + i ); ++i;
			case 5: pAllocator.construct( pDest + i ); ++i;
			case 4: pAllocator.construct( pDest + i ); ++i;
			case 3: pAllocator.construct( pDest + i ); ++i;
			case 2: pAllocator.construct( pDest + i ); ++i;
			case 1: pAllocator.construct( pDest + i ); ++i;
				}
			}
		}

		template<typename T, class TAllocator>
		void construct( T * pDest, TAllocator& pAllocator, T&& pFillValue )
		{
			pAllocator.construct( pDest, std::move(pFillValue) );
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

		template<typename T, class TAllocator>
		void copy_construct( if_copyable_t<T> * pDest, TAllocator& pAllocator, const T& pFillValue )
		{
			memcpy(pDest, &pFillValue, sizeof(T));
		}

		template<typename T, class TAllocator>
		void copy_construct(if_copyable_t<T> * pDest, const size_t pSize, TAllocator& pAllocator, const T& pFillValue)
		{
			duff_fill(pDest, pFillValue, pSize);
		}

		template<typename T, class TAllocator>
		void copy_construct(if_non_copyable_t<T> * pDest, TAllocator& pAllocator, const T& pFillValue)
		{
			pAllocator.construct(pDest, pFillValue);
		}

		template<typename T, class TAllocator>
		void copy_construct( if_non_copyable_t<T> * pDest, const size_t pSize, TAllocator& pAllocator, const T& pFillValue )
		{
			size_t i = 0;
			switch ((pSize - i) & 7)
			{
			case 0:
				while (i != pSize)
				{
					pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 7: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 6: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 5: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 4: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 3: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 2: pAllocator.construct( pDest + i, pFillValue ); ++i;
			case 1: pAllocator.construct( pDest + i, pFillValue ); ++i;
				}
			}
		}
		
		template<typename T, class TAllocator>
		void destroy( if_copyable_t<T> * pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{}

		template<typename T, class TAllocator>
		void destroy( if_non_copyable_t<T> * pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{
			for ( size_t i = 0; i < pCapacity; ++i )
				pAllocator.destroy( pBegin + i );
		}


		template<typename T, class TAllocator>
		void deallocate( T *& pBegin, const size_t pCapacity, TAllocator& pAllocator)
		{
			pAllocator.deallocate(pBegin, pCapacity);
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
	class iterator_base
	{
	public:
#if _DEBUG
#define ITR_CHECK_OUT_OF_BOUNDS( x )	if( (x) && (mEnd < (mCaret + (x)) || (mCaret + (x)) < itr_mBegin) ) { KS_ASSERT( 0 && "iterator out of bounds!" ); }
#else
#define ITR_CHECK_OUT_OF_BOUNDS( x )
#endif
		typedef std::random_access_iterator_tag	iterator_category;
		typedef T							value_type;
		typedef ptrdiff_t					difference_type;
		typedef size_t						size_type;
		typedef value_type*					pointer;
		typedef const value_type*			const_pointer;
		typedef value_type&					reference;
		typedef const value_type&			const_reference;

		iterator_base() : mCaret(nullptr), itr_setBegin(nullptr) mEnd(nullptr)
		{}
		iterator_base( pointer pBegin, pointer pCaret, pointer pEnd ) : mCaret(pCaret), itr_setBegin(pBegin) mEnd(pEnd)
		{}
		iterator_base( const iterator_base& pOther )				{ copy(pOther); }

		bool operator==( const iterator_base& pOther ) const		{ return equals(pOther); }
		bool operator!=( const iterator_base& pOther ) const		{ return !equals(pOther); }

	protected:
		void copy( const iterator_base& pOther )
		{
			mCaret	= pOther.mCaret;
			mEnd	= pOther.mEnd;
			itr_copyBegin( pOther );
		}
		void jump( const int pCount )
		{
			ITR_CHECK_OUT_OF_BOUNDS( pCount );
			mCaret += pCount;
		}
		bool equals( const iterator_base& pOther ) const
		{
			itr_checkContainerMismatch(pOther);
			return mCaret == pOther.mCaret;
		}

		difference_type minus(const iterator_base& pRHS) const		{ itr_checkContainerMismatch(pRHS); return (mCaret - pRHS.mCaret); }
		bool lessthan(const iterator_base& pRHS) const				{ itr_checkContainerMismatch(pRHS); return mCaret < pRHS.mCaret; }
		bool morethan(const iterator_base& pRHS) const				{ itr_checkContainerMismatch(pRHS); return mCaret > pRHS.mCaret; }

		reference at(size_type i)									{ ITR_CHECK_OUT_OF_BOUNDS( i ); return mCaret[i]; }
		const_reference at(size_type i) const						{ ITR_CHECK_OUT_OF_BOUNDS( i ); return mCaret[i]; }

		pointer	mCaret;
#if _DEBUG
		size_t getAxis() const										{ return size_t(mBegin); }
		pointer	mBegin;
#endif
		pointer	mEnd;
	};


	template<typename T>
	class ForwardIterator : public iterator_base<T>
	{
	public:
		typedef iterator_base<T>				base;
		typedef typename base::pointer			pointer;
		typedef typename base::size_type		size_type;
		typedef typename base::reference		reference;
		typedef typename base::difference_type	difference_type;
		ForwardIterator() : base()
		{}
		ForwardIterator(std::nullptr_t) : base()
		{}
		ForwardIterator( pointer pBegin, pointer pCaret, pointer pEnd ) : base( pBegin, pCaret, pEnd )
		{}

		pointer operator->()											{ return this->mCaret; }
		reference operator*()											{ return this->at(0); }
		reference operator[](size_type i)								{ return this->at(i); }

		// OVERIDES
		ForwardIterator& operator++()									{ this->jump(1); return *this; }
		ForwardIterator operator++(int)									{ ForwardIterator prev( *this ); this->jump(1); return prev; }

		ForwardIterator& operator--()									{ this->jump(-1); return *this; }
		ForwardIterator operator--(int)									{ ForwardIterator prev( *this ); this->jump(-1); return prev; }

		ForwardIterator& operator+=(const size_type pCount)				{ this->jump((int)pCount); return *this; }
		ForwardIterator& operator-=(const size_type pCount)				{ this->jump(-(int)pCount); return *this; }

		ForwardIterator operator+(const size_type pCount) const			{ ForwardIterator temp(*this); temp.jump((int)pCount); return temp; }
		ForwardIterator operator-(const size_type pCount) const			{ ForwardIterator temp(*this); temp.jump(-(int)pCount); return temp; }


		difference_type operator-(const ForwardIterator& pRHS) const	{ return this->minus(pRHS); }
		bool operator<(const ForwardIterator& pRHS) const				{ return this->lessthan(pRHS); }
		bool operator>(const ForwardIterator& pRHS) const				{ return this->morethan(pRHS); }
	};

	template<typename T>
	class ConstForwardIterator : public iterator_base<T>
	{
	public:
		typedef iterator_base<T>				base;
		typedef typename base::pointer			pointer;
		typedef typename base::size_type		size_type;
		typedef typename base::const_pointer	const_pointer;
		typedef typename base::const_reference	const_reference;
		typedef typename base::difference_type	difference_type;
		ConstForwardIterator() : base()
		{}
		ConstForwardIterator(std::nullptr_t) : base()
		{}
		ConstForwardIterator( pointer pBegin, pointer pCaret, pointer pEnd ) : base( pBegin, pCaret, pEnd )
		{}
		ConstForwardIterator( const ForwardIterator<T>& pOther )			{ this->copy(pOther); }

		const_pointer operator->() const									{ return this->mCaret; }
		const_reference operator*() const									{ return this->at(0); }
		const_reference operator[](size_type i) const						{ return this->at(i); }
		
		// OVERIDES
		ConstForwardIterator& operator=( const ForwardIterator<T>& pOther )	{ this->copy(pOther); return *this; }

		ConstForwardIterator& operator++()									{ this->jump(1); return *this; }
		ConstForwardIterator operator++(int)								{ ConstForwardIterator prev( *this ); this->jump(1); return prev; }

		ConstForwardIterator& operator--()									{ this->jump(-1); return *this; }
		ConstForwardIterator operator--(int)								{ ConstForwardIterator prev( *this ); this->jump(-1); return prev; }

		ConstForwardIterator& operator+=(const size_type pCount)			{ this->jump((int)pCount); return *this; }
		ConstForwardIterator& operator-=(const size_type pCount)			{ this->jump(-(int)pCount); return *this; }

		ConstForwardIterator operator+(const size_type pCount) const		{ ConstForwardIterator temp(*this); temp.jump((int)pCount); return temp; }
		ConstForwardIterator operator-(const size_type pCount) const		{ ConstForwardIterator temp(*this); temp.jump(-(int)pCount); return temp; }

		difference_type operator-(const ConstForwardIterator& pRHS) const	{ return this->minus(pRHS); }
		bool operator<(const ConstForwardIterator& pRHS) const				{ return this->lessthan(pRHS); }
		bool operator>(const ConstForwardIterator& pRHS) const				{ return this->morethan(pRHS); }
	};


	template<typename T, class TAllocator = std::allocator<T> >
	class Array : private TAllocator
	{
#define KS_ARRAY_MOVE_SEMANTICS_ONLY	1		// Before turning this off, have you considered using std::vector? :P


#if _DEBUG
#define KS_USE_FORWARD_ITERATOR_CLASS	1
#define CHECK_OUT_OF_BOUNDS( x )		if( (x) >= _size ) { KS_ASSERT( 0 && "Array out of bounds detected" ); }
#else
#define CHECK_OUT_OF_BOUNDS( x )
#endif

	public:
#if KS_USE_FORWARD_ITERATOR_CLASS
		typedef ForwardIterator<T>					iterator;
		typedef ConstForwardIterator<T>				const_iterator;

#else
		typedef T*									iterator;
		typedef const T*							const_iterator;

#endif
		typedef T									value_type;
		typedef T*									pointer;
		typedef const T*							const_pointer;
		typedef T&									reference;
		typedef const T&							const_reference;
		typedef size_t								size_type;
		typedef ptrdiff_t							difference_type;

		typedef ReverseIterator<T>					reverse_iterator;
		typedef ReverseIterator<T>					const_reverse_iterator;

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

		void pop_back()					{ CHECK_OUT_OF_BOUNDS(_size - 1); details::destroy<value_type>(_begin + --_size, 1, static_cast<TAllocator&>(*this)); }

		void clear()					{ resize(0);}
		void shrink_to_fit()			{ set_capacity(_size); }
		void trim()						{ set_capacity(_size); }
		void trim(size_type toSize)		{ set_capacity(toSize); }

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

			if( new_size > _size)
				details::construct<value_type>( _begin + _size, new_size - _size, static_cast<TAllocator&>(*this) );
			else if( _size > new_size )
				details::destroy<value_type>(_begin + new_size, _size - new_size, static_cast<TAllocator&>(*this) );

			_size = new_size;
		}

		void resize(size_type new_size, const_reference pFillValue )
		{
			if (new_size > _capacity)
				grow(new_size);

			if( new_size > _size)
				details::copy_construct( _begin + _size, new_size - _size, static_cast<TAllocator&>(*this), pFillValue );
			else if( _size > new_size )
				details::destroy<value_type>( _begin + new_size, _size - new_size, static_cast<TAllocator&>(*this) );

			_size = new_size;
		}

		void reserve(size_type new_capacity)
		{
			if (new_capacity > _capacity)
				set_capacity(new_capacity);
		}

		void push_back(const_reference item)
		{
			if ( _size + 1 > _capacity)
				grow();

			details::copy_construct( _begin + _size, static_cast<TAllocator&>(*this), item );
			++_size;
		}

		void push_back(value_type&& item)			{ emplace_back(std::move(item)); }

		void emplace_back(value_type&& item)
		{
			if (_size + 1 > _capacity)
				grow();

			details::construct<value_type>( _begin + _size, static_cast<TAllocator&>(*this), std::move(item) );
			++_size;
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

			*pPos	= pValue;

			return pPos;
		}

		iterator insert( iterator pPos, value_type&& pValue )
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


		Array() : _begin(nullptr), _size(0), _capacity(0)
		{}

		Array( size_type pCapacity ) : _begin(nullptr), _size(0), _capacity(0)
		{
			set_capacity( pCapacity );
		}
		Array( size_type pCapacity, const_reference pFillValue ) : _begin(nullptr), _size(0), _capacity(0)
		{
			resize( pCapacity, pFillValue );
		}

		~Array()
		{
			details::destroy<value_type>(_begin, _size, static_cast<TAllocator&>(*this));
			details::deallocate(_begin, _capacity, static_cast<TAllocator&>(*this));
		}

		Array(Array &&other)
			: _begin(other._begin)
			, _size(other._size)
			, _capacity(other._capacity)
		{
			other._begin	= nullptr;
			other._size		= 0;
			other._capacity	= 0;
		}

		Array &operator=(Array &&other)
		{
			if ( this != &other )
			{
				details::destroy<value_type>(_begin, _size, static_cast<TAllocator&>(*this));
				details::deallocate(_begin, _capacity, static_cast<TAllocator&>(*this));

				_begin		= other._begin;
				_size		= other._size;
				_capacity	= other._capacity;

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

		void explicit_copy( const Array & pSource )
		{
			const size_type n = pSource._size;
			resize(n);
			details::copy_into( _begin, pSource._begin, n );
		}

#if !KS_ARRAY_MOVE_SEMANTICS_ONLY

		Array(const Array &other) : _size(0), _capacity(0), _begin(nullptr)
		{
			explicit_copy(other);
		}


		Array &operator=(const Array &other)
		{
			explicit_copy(other);
			return *this;
		}
#else
	private:
		Array(const Array &);
		Array &operator=(const Array &);
#endif

	private:
		pointer		_begin;
		size_type	_size;
		size_type	_capacity;


		void grow(size_type min_capacity = 0)
		{
			size_type prev_size		= _size;
			size_type new_capacity	= _capacity + (_capacity >> 2) + 2;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			set_capacity(new_capacity);

			details::construct<value_type>( _begin + prev_size, new_capacity - prev_size, static_cast<TAllocator&>(*this) );
		}

		void set_capacity(size_type new_capacity)
		{
			if (new_capacity == _capacity)
				return;

			if (new_capacity < _size)
				resize(new_capacity);

			pointer new_data	= 0;
			auto& _allocator	= static_cast<TAllocator&>(*this);
			if (new_capacity > 0)
			{
				details::allocate<value_type>( new_data, new_capacity, _allocator );
				details::construct<value_type>( new_data, _size, _allocator );
				details::move_into( new_data, _begin, _size );
			}
			details::destroy<value_type>(_begin, _size, static_cast<TAllocator&>(*this));
			details::deallocate(_begin, _capacity, _allocator );
			_begin		= new_data;
			_capacity	= new_capacity;
		}
	};


}	//namespace ks


#endif	// KS_ARRAY