/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama (2015)
///	@about		Experimental managed pointer with simple memory management
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

#include "DataPointer.h"
#include <Reflection\TypeUID.h>

namespace ks {

	namespace mem {

		template<typename T>
		class DataPool
		{
		public:
			static void release(DataPointer<T>& pointer);
			static DataPointer<T> construct();
			static DataPointer<T> construct(T&& obj);
			static DataPointer<T> construct(const Handle<T>& source);

			static void on_assign(Handle<T>& pDest, const Handle<T>& pSource);

			// debug only
			static void incref(const Handle<T>& h);
			static void decref(const Handle<T>& h);

			static void OnDestroy();

		private:
			DataPool() {};
			static T* fetch(const Handle<T>& pointer);

			static u32 availableIndex();

			static Array<T>		sDataBuffer;
			static Array<u32>	sUIDs;
#if _DEBUG
			static Array<char>	sRefCount;				// @TODO, handles up to 255 ptr refs, surely that's enough, no?
#endif
		};

		///////////////////////////////////////////////////////////////////////////////////
		// DataPointer (impl)
		///////////////////////////////////////////////////////////////////////////////////

		template<typename T>
		typename DataPointer<T>::REQ_FUNC DataPointer<T>::sMemRequisitor = nullptr;

		template<typename T>
		DataPointer<T>::DataPointer()
		{}

		template<typename T>
		DataPointer<T>::DataPointer(const DataPointer& other)
		{
			*this = other;
		}

		template<typename T>
		DataPointer<T>::DataPointer(const Handle<T>& pKey)
		{
			mHandle = pKey;
		}

		template<typename T>
		DataPointer<T>::~DataPointer()
		{
			DataPool<T>::decref(mHandle);
		}

		template<typename T>
		Handle<T>& DataPointer<T>::operator*()				{ return mHandle; }

		template<typename T>
		const Handle<T>& DataPointer<T>::operator*() const	{ return mHandle; }

		template<typename T>
		void DataPointer<T>::operator=(const DataPointer& other)
		{
			DataPool<T>::on_assign(mHandle, other.mHandle);
			mHandle = other.mHandle;
		}

		template<typename T>
		T* DataPointer<T>::operator->() const
		{
			T* data = sMemRequisitor( mHandle );
#if _DEBUG
			if (data == nullptr)
			{
				KS_ASSERT(0 && "Invalid pointer detected");
				static T sCrashPrevention;
				return &sCrashPrevention;
			}
#endif
			return data;
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		// DataPool
		///////////////////////////////////////////////////////////////////////////////////////////

		template<typename T>
		Array<T> DataPool<T>::sDataBuffer;
		template<typename T>
		Array<u32> DataPool<T>::sUIDs;
#if _DEBUG
		template<typename T>
		Array<char> DataPool<T>::sRefCount;
#endif

		template<typename T>
		void DataPool<T>::release(DataPointer<T>& p)
		{
			auto& h = p.mHandle;
			if_ks_dbg (sUIDs[h.index] == h.uid)
				sUIDs[h.index] = INVALID_UID;

			h.uid	= INVALID_UID;
			h.index = INVALID_INDEX;
		}

		template<typename T>
		DataPointer<T> DataPool<T>::construct()
		{
			DataPointer<T>::sMemRequisitor = &DataPool::fetch;		// TODO: init once

			u32 index		= availableIndex();
			sUIDs[index]	= InstanceUIDGenerator<DataPool<T> >::Get();

			Handle<T> handle;
			handle.index = index; handle.uid = sUIDs[index];
			incref( handle );
			return DataPointer<T>( handle );
		}

		template<typename T>
		DataPointer<T> DataPool<T>::construct(T&& obj)
		{
			DataPointer<T> p = construct();
			sDataBuffer[p.mHandle.index] = ks::move(obj);
			return p;
		}

		template<typename T>
		DataPointer<T> DataPool<T>::construct(const Handle<T>& source)
		{
			DataPointer<T> p = construct();
			if_ks_dbg ( source.index != INVALID_INDEX )
				sDataBuffer[p.mHandle.index] = sDataBuffer[source.index];

			return p;
		}

		template<typename T>
		void DataPool<T>::incref(const Handle<T>& h)
		{
#if _DEBUG
			if (h.index != INVALID_INDEX) ++sRefCount[h.index];
#endif
		}

		template<typename T>
		void DataPool<T>::decref(const Handle<T>& h)
		{
#if _DEBUG
			if (h.index != INVALID_INDEX)
			{
				KS_ASSERT(sRefCount[h.index] != 0 && "memory leak");
				--sRefCount[h.index];
			}
#endif
		}

		template<typename T>
		void DataPool<T>::on_assign(Handle<T>& pDest, const Handle<T>& pSource)
		{
#if _DEBUG
			incref(pSource);
			decref(pDest);
			KS_ASSERT( sRefCount[pSource.index] != 0xFF && "Refcount maxed out" );
#endif
		}

		template<typename T>
		T* DataPool<T>::fetch(const Handle<T>& h)
		{
			if_ks_dbg ( h.uid != INVALID_UID && h.uid == sUIDs[ h.index ] )
				return &sDataBuffer[ h.index ];
			
			return nullptr;
		}

		template<typename T>
		void DataPool<T>::OnDestroy()
		{}


		template<typename T>
		u32 DataPool<T>::availableIndex()
		{
			u32 i = 0;
			for (; i < sUIDs.size() && sUIDs[i] != INVALID_UID; ++i)
			{}

			if (i >= sUIDs.size())
			{
				sUIDs.resize(i + 2, INVALID_UID);
				sDataBuffer.resize(i + 2);
#if _DEBUG
				sRefCount.resize(i + 2, 0);
#endif
			}

			return i;
		}

		////////////////////////////////////////////////////////////////////////////////
		//
		////////////////////////////////////////////////////////////////////////////////

		template<typename T>
		DataPointer<T> New()					{ return DataPool<T>::construct(); }

		template<typename T>
		DataPointer<T> New(T&& obj)				{ return DataPool<T>::construct(obj); }

		template<typename T>
		void Delete(DataPointer<T>& p)			{ return DataPool<T>::release(p); }

	}	// namespace mem

}	// namespace ks
