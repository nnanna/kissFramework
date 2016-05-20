
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		11/01/2015
///	@brief		Type storage interface for correct forwarding of anonymous jobs
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

#ifndef KS_JOB_DECODER_H
#define KS_JOB_DECODER_H


namespace ks {


	class IJobDecoder
	{
	public:
		virtual ~IJobDecoder()									{}
		virtual size_t operator ()(char* pFuncPtr) const = 0;
		virtual void operator ()(char* pFuncPtr, const char* pArg) const = 0;
		virtual void Create(char* pDest, const char* pSource) = 0;
		virtual void Create(char* pFuncDest, const char* pFuncSource, char* pCompDest, const char* pCompSource) = 0;
		virtual void Release(char* pFuncPtr, char* pCompPtr) = 0;
	};

	template<typename _FN>
	class JobDecoder : public IJobDecoder
	{
	public:
		~JobDecoder()	{}

		static IJobDecoder* GetInterface()
		{
			static JobDecoder<_FN> sInstance;
			return &sInstance;
		}

		inline size_t operator ()(char* pFuncPtr) const override
		{
			_FN* func = (_FN*)(pFuncPtr);
			return (*func)();
		}

		inline void operator ()(char* pFuncPtr, const char* pArg) const override	{}

		inline void Create(char* pDest, const char* pSource) override
		{
			_FN* func = (_FN*)pSource;
			new(pDest)_FN(std::move(*func));
		}

		void Create(char* pFuncDest, const char* pFuncSource, char* pCompDest, const char* pCompSource) override
		{
			struct illegal_call {};
			throw illegal_call();
		}

		inline void Release(char* pFuncPtr, char* pCompPtr) override
		{
			_FN* func = (_FN*)(pFuncPtr);
			func->~_FN();
		}
	private:
		JobDecoder()	{};
	};


	template<typename _FN, typename _CT, typename ResultType>
	class CompletionForwarder : public IJobDecoder
	{
	public:
		~CompletionForwarder()								{}

		static IJobDecoder* GetInterface()
		{
			static CompletionForwarder<_FN, _CT, ResultType> sInstance;
			return &sInstance;
		}

		inline size_t operator ()(char* pFuncPtr) const override
		{
			_FN* func = (_FN*)(pFuncPtr);
			return (size_t)(*func)();
		}

		inline void operator ()(char* pFuncPtr, const char* pArg) const override
		{
			_CT* func = (_CT*)(pFuncPtr);
			(*func)(*(ResultType*)(pArg));
		}

		inline void Create(char* pDest, const char* pSource) override
		{
			_FN* func = (_FN*)(pSource);
			new(pDest)_FN(std::move(*func));
		}

		void Create(char* pFuncDest, const char* pFuncSource, char* pCompDest, const char* pCompSource) override
		{
			_FN* func = (_FN*)(pFuncSource);
			new(pFuncDest)_FN(std::move(*func));

			_CT* comp = (_CT*)(pCompSource);
			new(pCompDest)_CT(std::move(*comp));

		}

		inline void Release(char* pFuncPtr, char* pCompPtr) override
		{
			_FN* func = (_FN*)(pFuncPtr);
			func->~_FN();

			_CT* comp = (_CT*)(pCompPtr);
			comp->~_CT();
		}
	private:
		CompletionForwarder()	{};
	};


}	// namespace ks

#endif