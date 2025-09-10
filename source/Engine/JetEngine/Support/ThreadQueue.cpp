/****************************************************************************************/
/*  THREADQUEUE.C                                                                       */
/*                                                                                      */
/*  Author:  Eli Boling                                                                 */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include	<windows.h>

#include	<assert.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<process.h>

#include	"ThreadQueue.h"
#include	"mempool.h"
#include	"log.h"
#include	"threadlog.h"

#ifdef	__BORLANDC__
#define jeRam_Allocate malloc
#define jeRam_Free free
#else
#include	"ram.h"
#endif

#define	THREADSTACKSIZE	(0x10000)
#define	MAX_THREADS		(100)	// this is just the static array size

/*}{******** The Types **********/

typedef	enum
{
	TS_NOTSTARTED,
	TS_FREE=0,
	TS_PENDING,
	TS_OUGHTTOBERUNNING,
	TS_RUNNING,
	TS_DONE,
}	Thread_State;

//typedef	void (*Thread_Function)(Thread *, void *);
typedef	void (*Thread_Function)(void *);

typedef	struct	Thread
{
	Thread_State		State;
	HANDLE				ThreadStallingEvent;
	Thread_Function		Function;
	void *				Context;
	jeBoolean			Terminate;
//	HANDLE				Handle;
}	Thread;

typedef	struct	ThreadPool
{
	Thread				Threads[MAX_THREADS];
//	int					MaxThreads;
	int					NumThreads;
	CRITICAL_SECTION	CS;
}	ThreadPool;

#define JOB_SIGNATURE 0xFEEDFEED

typedef	struct	jeThreadQueue_Job
{
//	HANDLE				ThreadHandle;
	uint32				Signature;
	jeThreadQueue_JobStatus	Status;
	int					RefCount;
//	jeAsyncStatus		Status;
//	jeBoolean			Active;

	jeThreadQueue_JobFunction	Function;
	void *				Context;
	jeErrorLog *		ErrorLog;
//	uint32				StackLimit;

	jeThreadQueue_Job *	Next;
	jeThreadQueue_Job *	Prev;
	Thread *			Thread;

}	jeThreadQueue_Job;


/*}{******** The Statics that represent the active Pool **********/

/*
		ActiveJobCount is the count of jobs that have threads assigned.
*/
static	int					ActiveJobCount = 0,MaxActiveJobs = MAX_THREADS;

static	ThreadPool *		GlobalThreadPool = NULL;

/*
		JobList is a circular list of jobs.  Jobs at the front are high
		priority.  Jobs at the back are low priority.
*/
#pragma warning (disable:4152)	// nonstandard extension, function/data pointer conversion in expression
static	jeThreadQueue_Job 	JobList =
{
//	(HANDLE)-1,
	JOB_SIGNATURE,
	JE_THREADQUEUE_STATUS_COMPLETED,
	1,
	//(void *)0xBEEFFACE,
	//(void *)0xCAFEDEAD,
	NULL,
	NULL,
	0,
};
#pragma warning (default:4152)

/*
		QueueLock is a critical section to guard access to the queue and
		status information.
*/
static	CRITICAL_SECTION	QueueLock;
//static	jeBoolean			QueueLockFlag;

/*
		TQInitialized is whether we've initialized the system.
*/
static	jeBoolean			TQInitialized = JE_FALSE;

/*}{******** Functions **********/

static	void	LockQueue(void)
{
	EnterCriticalSection(&QueueLock);
//	QueueLockFlag = JE_TRUE;
}

static	void	UnlockQueue(void)
{
//	QueueLockFlag = JE_FALSE;
	LeaveCriticalSection(&QueueLock);
}

static	void __cdecl ThreadFunction(void *Context)
{
	Thread *	T;

	T = (Thread*)Context;

	for	(;;)
	{
		WaitForSingleObject(T->ThreadStallingEvent, INFINITE);

		ActiveJobCount ++;
		T->State = TS_RUNNING;

		(T->Function)(T->Context);

		T->State = TS_DONE;
		ActiveJobCount --;
	}
}

void	Thread_Run(Thread *T, Thread_Function Function, void *Context)
{
	assert(T->State == TS_PENDING);

	T->Function = Function;
	T->Context = Context;
	T->State = TS_OUGHTTOBERUNNING;

	PulseEvent(T->ThreadStallingEvent);
}

jeBoolean ThreadPool_Destroy(ThreadPool *Pool)
{
	int	i;

	assert(Pool);

	for	(i = 0; i < Pool->NumThreads; i++)
	{
		if	(Pool->Threads[i].State != TS_FREE)
		{
			jeErrorLog_AddString(-1,"ThreadQueue : cannot free threadpool : threads still running!",NULL);
			return JE_FALSE;
		}

		CloseHandle(Pool->Threads[i].ThreadStallingEvent);
	}

	DeleteCriticalSection(&Pool->CS);
	jeRam_Free(Pool);

	return JE_TRUE;
}

jeBoolean InitThread(Thread * T)
{
	assert(T);

	memset(T,0,sizeof(*T));

	T->State = TS_FREE;
	T->ThreadStallingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if ( T->ThreadStallingEvent == NULL )
	{
		return JE_FALSE;
	}

	if ( _beginthread(ThreadFunction, THREADSTACKSIZE, T) == -1 )
	{
		CloseHandle(T->ThreadStallingEvent);
		return JE_FALSE;
	}

return JE_TRUE;
}

ThreadPool *	ThreadPool_Create(void)
{
ThreadPool *	Pool;

	Pool = (ThreadPool*)jeRam_AllocateClear(sizeof(*Pool));
	if	(!Pool)
		return Pool;

	InitializeCriticalSection(&Pool->CS);

	Pool->NumThreads = 0;

return Pool;
}

Thread * ThreadPool_GetFreeThread(ThreadPool *Pool)
{
	Thread *	Result;
	int			i;

	EnterCriticalSection(&Pool->CS);
	Result = NULL;
	for	(i = 0; i < Pool->NumThreads; i++)
	{
	Thread * T;
		T = Pool->Threads + i;
#if 1 // @@
		if ( T->State == TS_FREE || T->State == TS_DONE )
#else
		if ( T->State == TS_FREE )
#endif
		{
			Result = T;
			Result->State = TS_PENDING;
			break;
		}
		// CB : why do we do this *after* the check?
		#pragma message("ThreadPool : GetFreethread : CB wierd DONE -> FREE !")
#if 0 // @@
		if	(Pool->Threads[i].State == TS_DONE)
			Pool->Threads[i].State = TS_FREE;
#endif
	}
	if ( ! Result )
	{
		// with MaxActiveJobCount, we should never hit this condition,
		//	but with all the sync-up problems, what the hey?
		if ( Pool->NumThreads < MAX_THREADS )
		{
			Result =&(Pool->Threads[Pool->NumThreads]);
			if ( InitThread(Result) )
			{
				Pool->NumThreads++;
				Result->State = TS_PENDING;
			}
			else
			{
				Result = NULL;
			}
			Log_Printf("ThreadQueue : extending pool to %d threads\n",Pool->NumThreads);
		}
	}
	LeaveCriticalSection(&Pool->CS);

	return Result;
}

static	jeBoolean	InitTQ(void)
{
	if ( TQInitialized == JE_FALSE )
	{
		JobList.Next = &JobList;
		JobList.Prev = &JobList;
		InitializeCriticalSection(&QueueLock);

		TQInitialized = JE_TRUE;
	}

	if ( ! 	GlobalThreadPool )
	{
		GlobalThreadPool = ThreadPool_Create();
		if ( ! GlobalThreadPool )
			return JE_FALSE;
	}

	return JE_TRUE;
}

typedef	enum
{
	CHECK_FORWARD,
	CHECK_BACKWARD,
}	DebugDirection;

static	void	PutBreakPointHere(void)
{
	OutputDebugString("CheckQueue is about to fail\r\n");
}

static	jeBoolean	CheckLockedQueue(DebugDirection Direction)
{
static	jeThreadQueue_Job *	Runner;
static	jeThreadQueue_Job *	Runner2x;

	/*
		Some invariants:
	*/
	if	(JobList.Status != JE_THREADQUEUE_STATUS_COMPLETED)
	{
		PutBreakPointHere();
		return JE_FALSE;
	}
#if 0
	if	(JobList.ThreadHandle != (HANDLE)-1)
	{
		PutBreakPointHere();
		return JE_FALSE;
	}
#endif
	if	(JobList.Function != (jeThreadQueue_JobFunction)0xBEEFFACE)
	{
		PutBreakPointHere();
		return JE_FALSE;
	}
	if	(JobList.Context != (void *)0xCAFEDEAD)
	{
		PutBreakPointHere();
		return JE_FALSE;
	}

//	if	(JobList.StackLimit != 0)
//	{
//		PutBreakPointHere();
//		return JE_FALSE;
//	}

	if	(Direction == CHECK_FORWARD)
	{
		Runner = JobList.Next;
		Runner2x = JobList.Next->Next;
	}
	else
	{
		Runner = JobList.Prev;
		Runner2x = JobList.Prev->Prev;
	}

	//  No jobs in the list?
	if	(JobList.Next == &JobList)
	{
		if	(JobList.Prev != &JobList)
		{
			PutBreakPointHere();
			return JE_FALSE;
		}
		return JE_TRUE;
	}

	do
	{
		if	(Runner->Status < JE_THREADQUEUE_STATUS_WAITINGFORTHREAD ||
			 Runner->Status > JE_THREADQUEUE_STATUS_COMPLETED)
		{
			PutBreakPointHere();
			return JE_FALSE;
		}

		//  Is there a loop?
		if	(Runner == Runner2x)
		{
			PutBreakPointHere();
			return JE_FALSE;
		}

		if	(Direction == CHECK_FORWARD)
		{
			Runner = Runner->Next;
			Runner2x = Runner2x->Next->Next;
		}
		else
		{
			Runner = Runner->Prev;
			Runner2x = Runner2x->Prev->Prev;
		}

	}	while	(Runner != &JobList && Runner2x != &JobList);

	return JE_TRUE;
}

static	jeBoolean	CheckQueue(DebugDirection Direction)
{
	jeBoolean	Result;

	LockQueue();
	Result = CheckLockedQueue(Direction);
	UnlockQueue();
	return Result;
}

static	void ThreadStart(void *Context)
{
	jeThreadQueue_Job *	Job;

	Job = (jeThreadQueue_Job*)Context;

	Job->Status = JE_THREADQUEUE_STATUS_RUNNING;
	(Job->Function)(Job, Job->Context);
	assert(Job->Signature == JOB_SIGNATURE);
	Job->Status = JE_THREADQUEUE_STATUS_COMPLETED;
}

JETAPI	void JETCC jeThreadQueue_Sleep(int Milliseconds)
{
	Sleep(Milliseconds);
}

JETAPI	jeThreadQueue_JobStatus	JETCC jeThreadQueue_JobGetStatus(const jeThreadQueue_Job *Job)
{
	return Job->Status;
}

static	void	ActivateJob(jeThreadQueue_Job *Job)
{
	Thread *			T;

	T = ThreadPool_GetFreeThread(GlobalThreadPool);
	if	(!T)
		return;

	Job->Status = JE_THREADQUEUE_STATUS_WAITINGTOBEGIN;
	Job->Thread = T;
	Thread_Run(T, ThreadStart, Job);
}

JETAPI	void JETCC jeThreadQueue_PollJobs(void)
{
	jeThreadQueue_Job *	Jobs;

	if	(InitTQ() == JE_FALSE)
	{
#pragma message ("ThreadQueue_PollJobs: Need to be able to propagate err-ors")
		return;
	}

	assert(CheckQueue(CHECK_FORWARD ) == JE_TRUE);
	assert(CheckQueue(CHECK_BACKWARD) == JE_TRUE);

#pragma message ("ThreadQueue_PollJobs: I must not be called from multiple threads!")

	if ( ActiveJobCount >= MaxActiveJobs )
		return;

	LockQueue();
	Jobs = JobList.Next;
	while	(Jobs != &JobList && (ActiveJobCount < MaxActiveJobs))
//	while	(Jobs != &JobList)
	{
		if	(Jobs->Status == JE_THREADQUEUE_STATUS_WAITINGTOBEGIN)
		{
			if	(Jobs->Thread && Jobs->Thread->State == TS_OUGHTTOBERUNNING)
				PulseEvent(Jobs->Thread->ThreadStallingEvent);
		}

		if	(Jobs->Status == JE_THREADQUEUE_STATUS_WAITINGFORTHREAD)
		{
			ActivateJob(Jobs);
			break;
		}
		Jobs = Jobs->Next;
	}
	UnlockQueue();
}

JETAPI jeThreadQueue_Job *	JETCC jeThreadQueue_JobCreate(
	jeThreadQueue_JobFunction		Function,
	void *		Context,
	jeErrorLog *ErrorLog,
	uint32		StackLimit)
{
	jeThreadQueue_Job *	Job;

#pragma message("ThreadQueue_JobCreate : remove StackLimit parameter")

	InitTQ();

#pragma message("ThreadQueue : use MemPool for Jobs (?)")

	Job = (jeThreadQueue_Job*)jeRam_AllocateClear(sizeof(*Job));
	if	(!Job)
		return Job;

	Job->Signature	= JOB_SIGNATURE;
	Job->Function	= Function;
	Job->Context	= Context;
	Job->ErrorLog	= ErrorLog;
//	Job->StackLimit	= StackLimit;
	Job->Status = JE_THREADQUEUE_STATUS_WAITINGFORTHREAD;
	Job->RefCount = 1;

	/*
		New jobs always start out as low priority - they go to the
		back of the list.
	*/
	LockQueue();
	Job->Next = &JobList;
	Job->Prev = JobList.Prev;
	JobList.Prev->Next = Job;
	JobList.Prev = Job;
	UnlockQueue();

	return Job;
}

JETAPI	void JETCC jeThreadQueue_JobCreateRef(jeThreadQueue_Job *Job)
{
	assert(Job);
	LockQueue();
	Job->RefCount++;
	UnlockQueue();
}

JETAPI	void JETCC jeThreadQueue_JobDestroy(jeThreadQueue_Job **pJob)
{
	jeThreadQueue_Job *	Job;

	LockQueue();

	assert(pJob);
	assert(*pJob != &JobList);

	Job = *pJob;

	Job->RefCount--;
	
	if	(Job->RefCount)
	{
		UnlockQueue();
		return;
	}

	Job->Prev->Next = Job->Next;
	Job->Next->Prev = Job->Prev;

	UnlockQueue();

	assert(Job->Signature == JOB_SIGNATURE);
	assert(Job->Status == JE_THREADQUEUE_STATUS_COMPLETED);

	jeRam_Free(Job);

	*pJob = NULL;
}

JETAPI	jeBoolean JETCC jeThreadQueue_JobSetPriority(
	jeThreadQueue_Job *		Job,
	jeThreadQueue_Priority	Priority)
{
	if	(Job->Status != JE_THREADQUEUE_STATUS_WAITINGFORTHREAD)
		return JE_FALSE;

	LockQueue();

	//  Take it out of the list:
	Job->Next->Prev = Job->Prev;
	Job->Prev->Next = Job->Next;

	if	(Priority == JE_THREADQUEUE_PRIORITY_HIGH)
	{
		JobList.Next->Prev = Job;
		Job->Next = JobList.Next;
					JobList.Next = Job;
		Job->Prev = &JobList;
	}
	else
	{
		assert(Priority == JE_THREADQUEUE_PRIORITY_LOW);
		JobList.Prev->Next = Job;
		Job->Prev = JobList.Prev;
					JobList.Prev = Job;
		Job->Next = &JobList;
	}

	UnlockQueue();

	return JE_TRUE;
}

JETAPI	jeThreadQueue_Priority JETCC jeThreadQueue_JobGetPriority(
	jeThreadQueue_Job *		Job)
{
	int					i;
	jeThreadQueue_Job *	Jobs;

	LockQueue();

	Jobs = JobList.Next;
	i = 0;
	while	(Jobs != &JobList)
	{
		if	(Jobs == Job)
		{
			UnlockQueue();
			return JE_THREADQUEUE_PRIORITY_HIGH;
		}
		i++;
		Jobs = Jobs->Next;
#if 1
		if	(i >= MaxActiveJobs)
		{
			UnlockQueue();
			return JE_THREADQUEUE_PRIORITY_LOW;
		}
#endif
	}


	assert(!"Should never get here");
	return JE_THREADQUEUE_PRIORITY_LOW;
}

JETAPI	jeBoolean JETCC jeThreadQueue_SetThreadLimit(int MaxThreads)
{
	if ( MaxThreads >= MAX_THREADS )
		return JE_FALSE;
	MaxActiveJobs = MaxThreads;
	return JE_TRUE;
}

JETAPI	int JETCC jeThreadQueue_GetThreadLimit(void)
{
	return MaxActiveJobs;
//	return MAX_THREADS;
}

#ifndef NDEBUG
JETAPI	void JETCC jeThreadQueue_DumpQueue(void)
{
	jeThreadQueue_Job *	Jobs;

	printf("ThreadQueue: Dump of threads\n");
	printf("------------------------------\n");

	Jobs = JobList.Next;
	while	(Jobs != &JobList)
	{
		if	(Jobs->Status == JE_THREADQUEUE_STATUS_WAITINGFORTHREAD)
			printf("<%08x> Waiting\n", Jobs);
		if	(Jobs->Status == JE_THREADQUEUE_STATUS_RUNNING)
			printf("<%08x> Running\n", Jobs);
		if	(Jobs->Status == JE_THREADQUEUE_STATUS_COMPLETED)
			printf("<%08x> Completed\n", Jobs);
		Jobs = Jobs->Next;
	}
}
#endif

JETAPI jeBoolean JETCC jeThreadQueue_WaitOnJob(jeThreadQueue_Job * Job,
											jeThreadQueue_JobStatus WaitForStatus)
{
jeThreadQueue_JobStatus Status;

	assert( Job );

	if ( WaitForStatus != JE_THREADQUEUE_STATUS_RUNNING &&
		 WaitForStatus != JE_THREADQUEUE_STATUS_COMPLETED )
		return JE_FALSE;

	ThreadLog_Printf("WaitOnJob\n");

	Status = jeThreadQueue_JobGetStatus(Job);

	if ( Status < WaitForStatus )
	{
	int Waits1=0,Waits2=0;

		jeThreadQueue_JobSetPriority(Job,JE_THREADQUEUE_PRIORITY_HIGH);

		while ( Status < JE_THREADQUEUE_STATUS_RUNNING )
		{
			Waits1++;
			assert( Waits1 < 999999 );

			#pragma message("ThreadQueue_WaitOnJob : create emergency threads if all running threads are waiting on non-running threads")

			jeThreadQueue_PollJobs();
			jeThreadQueue_Sleep(1);
			Status = jeThreadQueue_JobGetStatus(Job);
		}
		
		while ( Status < WaitForStatus )
		{
			Waits2++;
			assert( Waits2 < 999999 );
			jeThreadQueue_Sleep(1);
			Status = jeThreadQueue_JobGetStatus(Job);
		}
	}

return JE_TRUE;
}

/* }{ **** jeThreadQueue Semaphore ******/

#define SEMAPHORE_SIGNATURE		((uint32)0xFEEDBABE)

struct jeThreadQueue_Semaphore
{
	uint32				Signature1;
	uint32				LockCount;
	CRITICAL_SECTION	CS;
	uint32				Signature2;
};

static MemPool * SemaphorePool = NULL;
static int Semaphores = 0;	// @@ check to see if we have leaks!

JETAPI jeThreadQueue_Semaphore * JETCC
	jeThreadQueue_Semaphore_Create(void)
{
jeThreadQueue_Semaphore * S;

	assert( Semaphores >= 0 );
	if ( ! Semaphores )
	{
		assert( SemaphorePool == NULL );
		SemaphorePool = MemPool_Create(sizeof(jeThreadQueue_Semaphore),64,64);
		if ( ! SemaphorePool )
			return NULL;
	}

	S = (jeThreadQueue_Semaphore*)MemPool_GetHunk(SemaphorePool);
	if ( ! S )
		return NULL;
	#ifndef NDEBUG
	S->Signature1 = SEMAPHORE_SIGNATURE;
	S->Signature2 = SEMAPHORE_SIGNATURE;
	#endif
	Semaphores++;
	S->LockCount = 0;
	InitializeCriticalSection(&(S->CS));
return S;
}

JETAPI void JETCC jeThreadQueue_Semaphore_Lock(jeThreadQueue_Semaphore * S)
{
	/* if ( S->LockCount )
	{
		// we're about to stall! try to prevent catastrophes!
		//	this is pointless; if our semaphore is locked, it must be locked by
		//		a running job
		// there's another problem : the same thread can lcok a semaphore many times!
		jeThreadQueue_PollJobs();
		jeThreadQueue_Sleep(1);
	} */
	assert( S );
	assert( S->Signature1 == SEMAPHORE_SIGNATURE &&
			S->Signature2 == SEMAPHORE_SIGNATURE );
	EnterCriticalSection(&(S->CS));
	
	//assert( ! S->LockCount == 0 || ActiveJobCount > 0 ); // no good, see note above
	S->LockCount ++;
}

JETAPI void JETCC jeThreadQueue_Semaphore_UnLock(jeThreadQueue_Semaphore * S)
{
	assert(S);
	assert( S->Signature1 == SEMAPHORE_SIGNATURE &&
			S->Signature2 == SEMAPHORE_SIGNATURE );
	assert(S->LockCount > 0);
	S->LockCount --;
	LeaveCriticalSection(&(S->CS));
}

JETAPI void JETCC jeThreadQueue_Semaphore_Destroy(jeThreadQueue_Semaphore ** pS)
{
	assert( pS );
	if ( *pS )
	{
	jeThreadQueue_Semaphore * S = *pS;

		assert( S->Signature1 == SEMAPHORE_SIGNATURE &&
				S->Signature2 == SEMAPHORE_SIGNATURE );
		assert( S->LockCount == 0 );

		DeleteCriticalSection(&(S->CS));
		
		MemPool_FreeHunk(SemaphorePool,S);
		assert( Semaphores > 0 );
		Semaphores--;
		if ( Semaphores == 0 )
		{
			MemPool_Destroy(&SemaphorePool);
			SemaphorePool = NULL;
		}
	}
	*pS = NULL;
}

#ifndef NDEBUG
JETAPI void JETCC jeThreadQueue_GetDebugInfo(int * pActiveJobCount,int *pSemaphoreCount, int * pNumThreads)
{
	if ( pActiveJobCount ) *pActiveJobCount = ActiveJobCount;
	if ( pSemaphoreCount ) *pSemaphoreCount = Semaphores;
	if ( pNumThreads ) *pNumThreads = GlobalThreadPool->NumThreads;
}
#endif
