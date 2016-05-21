
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



#ifndef KS_JOB_H
#define KS_JOB_H

#include "atomics.h"
#include "JobDecoder.h"
#include <Reflection/TypeUID.h>

namespace ks {


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	enum JobState
	{
		JS_INVALID		= 0,
		JS_WAITING		= 1,
		JS_RUNNING		= 2,
		JS_COMPLETED	= 3,
	};

#define JOB_UID_MASK				0x3fffffff
#define JOB_UID_UNMASK				~JOB_UID_MASK
#define JOB_UID_BITSIZE				30
#define JOB_SIG_ENCODE( s, id )		((s) << JOB_UID_BITSIZE) | ((id) & JOB_UID_MASK)
#define JOB_DECODE_STATE( sig )		(((sig) & JOB_UID_UNMASK) >> JOB_UID_BITSIZE)

	//////////////////////////////////////////////////////////////////////////
	// Job
	//////////////////////////////////////////////////////////////////////////
	class Job
	{
#define PROCESS_MAX_SIZE		10 * sizeof(size_t)
#define COMPLETOR_MAX_SIZE		2 * sizeof(size_t)

	public:

		Job()
			: mProcessDispatcher( nullptr )
			, mID( UIDGenerator::INVALID_UID )
			, mSignature( JS_INVALID )
		{
			setName( nullptr );
		}

		template<typename ProcessFunction>
		Job( ProcessFunction&& pProcess, const char* pName ) :
			mProcessDispatcher( nullptr ),
			mID( InstanceUIDGenerator<Job>::GetAsync( JOB_UID_UNMASK ) ),
			mSignature( JOB_SIG_ENCODE(JS_WAITING, mID) )
		{
			static_assert( PROCESS_MAX_SIZE >= sizeof(ProcessFunction), "That's a pretty huge lambda you've got there!" );

			mProcessDispatcher = JobDecoder<ProcessFunction>::GetInterface();
			allocDispatcher(mProcessDispatcher, mProcess, reinterpret_cast<char*>(&pProcess));
			setName( pName );
		}

		template<typename ProcessFunction, typename CompletionFunction>
		Job( ProcessFunction&& pProcess, CompletionFunction&& pCompletion, const char* pName ) :
			mProcessDispatcher( nullptr ),
			mID( InstanceUIDGenerator<Job>::GetAsync( JOB_UID_UNMASK ) ),
			mSignature( JOB_SIG_ENCODE(JS_WAITING, mID) )
		{
			static_assert( PROCESS_MAX_SIZE >= sizeof(ProcessFunction), "That's a pretty huge lambda you've got there" );
			static_assert( COMPLETOR_MAX_SIZE >= sizeof(CompletionFunction), "Whoa, what's in that completion callback!?" );

			mProcessDispatcher = CompletionForwarder<ProcessFunction, CompletionFunction, decltype(pProcess())>::GetInterface();
			allocDispatcher(mProcessDispatcher, mProcess, (char*)(&pProcess), mOnCompletion, (char*)(&pCompletion));
			setName( pName );
		}

		Job( Job&& pOther ) : mProcessDispatcher( nullptr )
		{
			*this = ks::move( pOther );
		}

		~Job()
        {
            destroyDispatcher();
        }

        Job& operator=( Job&& pRHS )
		{
			if( this != &pRHS )
			{
				const ksU32 rhsSig	= atomic_or( &pRHS.mSignature, 0 );
				
				// atomic validate & invalidate other job before moving. Else, it's already executing or invalid.
				if( rhsSig != JS_INVALID && atomic_compare_and_swap( &pRHS.mSignature, rhsSig, JS_INVALID ) == rhsSig )
				{
					destroyDispatcher();
					mSignature					= rhsSig;
					mProcessDispatcher			= pRHS.mProcessDispatcher;
					pRHS.mProcessDispatcher		= nullptr;
					mID							= pRHS.mID;
					memcpy( mProcess, pRHS.mProcess, PROCESS_MAX_SIZE );
					memcpy( mOnCompletion, pRHS.mOnCompletion, COMPLETOR_MAX_SIZE );
				}
				else
				{
					mSignature					= JS_INVALID;
				}
				setName( pRHS.Name() );

			}
            return *this;
        }

		void explicit_copy( const Job& pRHS )
		{
			destroyDispatcher();
			mProcessDispatcher		= pRHS.mProcessDispatcher;
			mSignature				= pRHS.mSignature;
			mID						= pRHS.mID;
			if( mProcessDispatcher != nullptr )
				mProcessDispatcher->Create( mProcess, pRHS.mProcess, mOnCompletion, pRHS.mOnCompletion );	// affirm move safety @TODO
			setName( pRHS.Name() );
		}

		JobState State() const						{ ksU32 sig = mSignature; return JobState( JOB_DECODE_STATE(sig) ); }

		const char* Name() const					{ return mProcessName; }

		const ksU32 UID() const						{ return mID; }
		
		bool Cancel( const ksU32 pKey ) const		{ return atomic_compare_and_swap(&mSignature, pKey, JS_INVALID) == pKey; }

		void Execute()
		{
			size_t result		= 0;
			const ksU32 waitKey( JOB_SIG_ENCODE(JS_WAITING, mID) );
			const ksU32 execKey( JOB_SIG_ENCODE(JS_RUNNING, mID) );
			if( atomic_compare_and_swap( &mSignature, waitKey, execKey ) == waitKey )
			{
				if( mProcessDispatcher != nullptr )
				{
					result	= (*mProcessDispatcher)( mProcess );
                }

				atomic_compare_and_swap( &mSignature, execKey, JOB_SIG_ENCODE(JS_COMPLETED, mID) );

				if (mProcessDispatcher != nullptr)
                {
					(*mProcessDispatcher)(mOnCompletion, (const char*)&result);
                }
                destroyDispatcher();
			}
		}

	private:
		Job( const Job& );
		Job& operator=( const Job& );

		void setName( const char* pName )				{ mProcessName	= pName; }


		void allocDispatcher( IJobDecoder*& pDispatcher, char* pDest, char* pSource )
		{
			pDispatcher->Create(pDest, pSource);
		}

		void allocDispatcher(IJobDecoder* pDispatcher, char* pFunc, char* pFuncSource, char* pComp, char* pCompSource)
		{
			pDispatcher->Create(pFunc, pFuncSource, pComp, pCompSource);
		}

        void destroyDispatcher()
        {
            if( mProcessDispatcher != nullptr )
            {
                mProcessDispatcher->Release( mProcess, mOnCompletion );
                mProcessDispatcher = nullptr;
            }
        }


		char				mProcess[ PROCESS_MAX_SIZE ];			// analogous to a union of all the possible functors we could have.
		char				mOnCompletion[ COMPLETOR_MAX_SIZE ];
		IJobDecoder*		mProcessDispatcher;
		ksU32				mID;
		mutable ksU32		mSignature;
		const char*			mProcessName;							// mainly for debug purposes
		// Needs no padding on 32bit configs cos it's 64B. TODO: padding on 64bit, it's 120B
	};

	class JobHandle
	{
	public:
		JobHandle();

		JobHandle(const Job* const pJob);

		JobState Cancel();
		
		bool IsValid() const;

		JobState GetState() const;

		bool StealExecute();

		// wait for job to complete or run job if still pending
		void Sync();

	private:
		const Job*	mJob;
		ksU32		mJobID;
		char cache_line_pad[CACHE_LINE_SIZE - sizeof(Job*) - sizeof(ksU32)];		// don't share this cache with another.

		bool validate( const Job* pJob ) const	{ return pJob && pJob->UID() == mJobID && mJobID != UIDGenerator::INVALID_UID; }
	};
	
	
}

#endif
