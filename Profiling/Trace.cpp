
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

#include "Trace.h"
#include <string>
#include <thread>
#include <Concurrency\atomics.h>

#define USE_TICK_COUNTER 1

#define DEFAULT_FILE_OUT	"tracefile.json"

using namespace std::chrono;

#if USE_TICK_COUNTER
__int64 sFreqHires(0xffffffffffffffff);
__int64 sBaseTime(0);
#else
#include <chrono>
system_clock::time_point gInitialTime;
#endif

bool gTracingEnabled = false;

void InitTimer()
{
#if USE_TICK_COUNTER
	LARGE_INTEGER	tf;

	if (QueryPerformanceFrequency(&tf))
	{
		sFreqHires = tf.QuadPart / 1000000;

		QueryPerformanceCounter(&tf);
		sBaseTime = ((__int64)tf.QuadPart / sFreqHires);
	}
#else
	gInitialTime = steady_clock::now();
#endif
}

__int64 GetElapsedUS()
{
#if USE_TICK_COUNTER
	LARGE_INTEGER		tc;
	QueryPerformanceCounter(&tc);

	return ((__int64)tc.QuadPart / sFreqHires) - sBaseTime;
#else
	auto now = steady_clock::now();
	auto duration = duration_cast<microseconds>(now - gInitialTime).count();
	return duration;
#endif
}

namespace ks {


	tracemarker::tracemarker(const char* pTag) : mTag(pTag)
	{
		if (gTracingEnabled)
			mTime = GetElapsedUS();
	}
	tracemarker::~tracemarker()		{ onClose(); }

	void tracemarker::onClose()
	{
		if (mTag && gTracingEnabled)
		{
			__int64 now		= GetElapsedUS();
			trace_event e	= { GetCurrentThreadId(), mTime, now - mTime, mTag };
			TraceAPI::Append(e);
			mTag			= nullptr;
		}
	}


	trace_event*	TraceAPI::mBuffer		= nullptr;
	unsigned		TraceAPI::mCapacity		= 0;
	unsigned		TraceAPI::mIndex		= 0;
	unsigned		TraceAPI::mProcessID	= 0;

	void TraceAPI::Init(const unsigned aizeInKB)
	{
		if (mBuffer == nullptr)
		{
			unsigned sz = aizeInKB * 1024;
			mCapacity	= sz / sizeof(trace_event);
			mBuffer		= (trace_event*)malloc(sz);
			mProcessID	= GetProcessIdOfThread(GetCurrentThread());
			mIndex		= 0;
			InitTimer();
		}
	}

	void TraceAPI::Destroy()
	{
		if (gTracingEnabled)
		{
			Stop(false);
			FileWriteRoutine( DEFAULT_FILE_OUT );
		}
		mCapacity = 0;
		free(mBuffer);
		mBuffer = nullptr;
	}

	void TraceAPI::Start()
	{
		gTracingEnabled = true;
	}

	void TraceAPI::Stop(bool asyncWriteToFile /*= true*/)
	{
		if (gTracingEnabled)
		{
			gTracingEnabled = false;
			if (asyncWriteToFile)
			{
				std::thread mThread{ FileWriteRoutine, DEFAULT_FILE_OUT };
			}
		}
	}

	void TraceAPI::FileWriteRoutine(const char* filename)
	{
		std::string output;
		output.reserve(4096);

		OutputAsJSON(output);

		FILE* handle(nullptr);
		int error = fopen_s(&handle, filename, "wb");

		if (error == 0)
		{
			fwrite(output.c_str(), output.length(), 1, handle);
		}

		fclose(handle);
	}

	void TraceAPI::Append( const trace_event& e )
	{
		unsigned index = atomic_increment(&mIndex) - 1;
		index %= mCapacity;

		mBuffer[index] = e;
	}

	template<typename StringClass>
	void TraceAPI::BeginJSON(StringClass& out)
	{
		out = "{\"traceEvents\":[ \n";
	}


	template<typename StringClass>
	void TraceAPI::EndJSON(StringClass& out)
	{
		out.append("\n] }");
	}

	template<typename StringClass>
	void TraceAPI::OutputAsJSON(StringClass& out)
	{
		BeginJSON(out);

		unsigned end		= mIndex;
		const unsigned size = mCapacity;
		unsigned start		= end > size ? end % size : 0;
		end %= size;

		do
		{
			AppendAsJSON(out, mBuffer[start]);
			start = ++start % size;
			if (start != end)
				out.append(",\n");
		} while (start != end);

		EndJSON(out);
	}

	template<typename StringClass>
	void TraceAPI::AppendAsJSON(StringClass& out, const trace_event& e)
	{
		bool metadata = false;
		int process_id = mProcessID;

		char buff[1024];

		if (metadata)
		{
			sprintf_s(buff,
				"{\"cat\":\"%s\",\"pid\":%i,\"tid\":%i,\"ts\":%lu,"
				"\"ph\":\"M\",\"name\":\"",
				"__metadata",
				process_id,
				e.tid,
				e.start_time);
		}
		else
		{
			sprintf_s(buff,
				"{\"pid\":%d,\"tid\":%llu,\"ts\":%llu,\"dur\":%llu,"
				"\"ph\":\"X\",\"name\":\"",
				process_id,
				e.tid,
				e.start_time,
				e.duration);
		}

		strcat_s(buff, e.name);
		strcat_s(buff, "\" }" );

		out.append(buff);
	}
}