
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		24/09/2016
///	@brief		Generates trace data that can be loaded & viewed via chrome://tracing/
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

namespace ks {

	struct trace_event	// 32 bytes on all architectures. values read in order for optimal performance
	{
		unsigned __int64		tid;
		unsigned __int64		start_time;
		unsigned __int64		duration;
		union
		{
			const char* 		name;
			unsigned __int64	naddress;
		};
	};
	struct tracemarker
	{
		tracemarker(const char* pTag);
		tracemarker::~tracemarker();
		void tracemarker::onClose();
	private:
		const char*	mTag;
		__int64		mTime;
	};

	class TraceAPI
	{
		friend tracemarker;
	public:
		static void Init(const unsigned sizeInKilobytes);
		static void Destroy();

		static void Start();
		static void Stop(bool asyncWriteToFile = true);

		template<typename StringClass>
		static void OutputAsJSON(StringClass& out);

	private:
		static void Append(const trace_event& e);

		template<typename StringClass>
		static void BeginJSON(StringClass& out);

		template<typename StringClass>
		static void EndJSON(StringClass& out);

		template<typename StringClass>
		static void AppendAsJSON(StringClass& out, const trace_event& e);

		static void FileWriteRoutine(const char* filename);

		static trace_event*	mBuffer;
		static unsigned		mCapacity;
		static unsigned		mIndex;
		static unsigned		mProcessID;
	};
}

#define KSTR_ENABLE_TRACING		0

#if KSTR_ENABLE_TRACING

#define TOKENCONCAT0(x, y)				x ## y
#define TOKENCONCAT(x, y)				TOKENCONCAT0(x, y)
#define TRACE_SCOPE(tag)				ks::tracemarker TOKENCONCAT(tcpfl, __LINE__) (tag)

#define TRACE_FUNC()					ks::tracemarker tcpfl (__FUNCTION__)

#define TRACE_BEGIN(tag, trace_id)		ks::tracemarker trace_id(tag)
#define TRACE_END(trace_id)				trace_id.onClose()

#else

#define TRACE_SCOPE(tag)
#define TRACE_FUNC()
#define TRACE_BEGIN(tag, trace_id)
#define TRACE_END(trace_id)

#endif