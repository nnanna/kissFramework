//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		28/01/2016
///	@about		AsyncResource : wraps a conventional object to enforce class-wide read/write synchronisation
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

#include "ReadWriteLock.h"

namespace ks {


	template<class T>
	class AsyncResource
	{
	public:
		struct Reader
		{
			Reader(const T& pResource, ReadWriteLock& pGuard) : mResource(&pResource), mLck(pGuard.Read())
			{}
			Reader(Reader&& o) : mResource(o.mResource), mLck(ks::move(o.mLck))
			{
				o.mResource = nullptr;
			}

			const T* operator->()			{ return mResource; }
			operator const T*() const		{ return mResource; }
			Reader& operator=(Reader&& o)	{ mResource = o.mResource; mLck = ks::move(o.mLck); o.mResource = nullptr; }
			void Release()					{ mLck.Release(); mResource = nullptr; }

		private:
			Reader(const Reader&) = delete;
			Reader& operator=(const Reader&) = delete;
			const T* mResource;
			ReadGuard mLck;
		};

		struct Writer
		{
			Writer(T& pResource, ReadWriteLock& pGuard) : mResource(&pResource), mLck(pGuard.Write())
			{}
			Writer(Writer&& o) : mResource(o.mResource), mLck(ks::move(o.mLck))
			{
				o.mResource = nullptr;
			}

			T* operator->()					{ return mResource; }
			operator T*() const				{ return mResource; }
			Writer& operator=(Writer&& o)	{ mResource = o.mResource; mLck = ks::move(o.mLck); o.mResource = nullptr; }
			void Release()					{ mLck.Release(); mResource = nullptr; }

		private:
			Writer(const Writer&) = delete;
			Writer& operator=(const Writer&) = delete;
			T* mResource;
			WriteGuard mLck;
		};

		operator T*() const 		{ return &mResource; }
		Reader Read()				{ return Reader(mResource, mLock); }
		Writer Write()				{ return Writer(mResource, mLock); }

	private:
		T						mResource;
		ReadWriteLock			mLock;
		//static ReadWriteLock	sClassDefaultLock;
	};


	//template<class T> ReadWriteLock AsyncResource<T>::sClassDefaultLock;
}