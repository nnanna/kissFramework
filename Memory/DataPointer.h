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

#ifndef DATA_POINTER_H
#define DATA_POINTER_H

#include <Debug.h>
#include <defines.h>
#include <Containers\Array.h>

namespace ks {

namespace mem {
		
#define	INVALID_INDEX		0XFFFFFFFF
#define INVALID_UID			0

	template<typename T>
	class DataPool;

	template<typename T>
	struct Handle
	{
		friend class DataPool<T>;

		Handle::Handle() : index(INVALID_INDEX), uid(INVALID_UID)
		{}
	private:
		u32 index;						// could potentially use 16 bit indices/UIDs?
		u32 uid;
	};


	template<typename T>
	struct DataPointer
	{
		typedef T* (*REQ_FUNC)(const Handle<T>&);
		friend class DataPool<T>;

		DataPointer();
		DataPointer(const DataPointer& other);
		DataPointer(const Handle<T>& pKey);
		~DataPointer();

		T* operator->() const;

		void operator=(const DataPointer& other);

		Handle<T>& operator*();
		const Handle<T>& operator*() const;

	private:
		Handle<T> mHandle;
		static REQ_FUNC sMemRequisitor;
	};

}	// namespace mem

}	// namespace ks


#endif