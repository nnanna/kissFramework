
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		11/01/2015
///	@brief		Allocation-free Job container
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

#include "Job.h"
#include "Service.h"
#include "JobScheduler.h"

namespace ks {

	JobHandle::JobHandle() : mJob(nullptr), mJobID(UIDGenerator::INVALID_UID)
	{}

	JobHandle::JobHandle(const Job* const pJob) : mJob(pJob), mJobID(mJob->UID())
	{}

	JobState JobHandle::Cancel()
	{
		const Job* job(mJob);
		if (validate(job))
			job->Cancel(JOB_SIG_ENCODE(JS_WAITING, mJobID));
		return GetState();
	}

	bool JobHandle::IsValid() const			{ return validate(mJob); }

	JobState JobHandle::GetState() const	{ auto job(mJob); return validate(job) ? job->State() : JS_INVALID; }

	bool JobHandle::StealExecute()
	{
		bool success = false;
		auto job(mJob);
		if (validate(job))
		{
			Job job_copy;
			job_copy.explicit_copy(*job);
			if (job->Cancel(JOB_SIG_ENCODE(JS_WAITING, mJobID)))
			{
				job_copy.Execute();
				success = true;
			}
		}
		return success;
	}

	void JobHandle::Sync()
	{
		if (StealExecute() == false)
		{
			Service<JobScheduler>::Get()->Wait(mJobID);
		}
	}
}