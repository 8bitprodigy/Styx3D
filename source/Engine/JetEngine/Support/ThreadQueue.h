/****************************************************************************************/
/*  THREADQUEUE.H                                                                       */
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
#ifndef	THREADQUEUE_H
#define THREADQUEUE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include	"basetype.h"
#include	"ErrorLog.h"

typedef	enum
{
	JE_THREADQUEUE_STATUS_WAITINGFORTHREAD,
	JE_THREADQUEUE_STATUS_WAITINGTOBEGIN,
	JE_THREADQUEUE_STATUS_RUNNING,
	JE_THREADQUEUE_STATUS_COMPLETED,
}	jeThreadQueue_JobStatus;
		// these are gauranteed to be in order of execution :
		//  if you want to know if a job is not running yet, you can do
		//		(Status < RUNNING) ?

typedef	struct jeThreadQueue_Job		jeThreadQueue_Job;
typedef struct jeThreadQueue_Semaphore	jeThreadQueue_Semaphore;

typedef	void (*jeThreadQueue_JobFunction)(jeThreadQueue_Job *, void *);

typedef	enum
{
	JE_THREADQUEUE_PRIORITY_HIGH,
	JE_THREADQUEUE_PRIORITY_LOW,
}	jeThreadQueue_Priority;

JETAPI	jeThreadQueue_Job *	JETCC jeThreadQueue_JobCreate(
	jeThreadQueue_JobFunction		Function,
	void *		Context,
	jeErrorLog *ErrorLog,
	uint32		StackLimit); // <> remove the StackLimit
				// note that when you call this, the job does not actually
				// start until someone calls a PollJos.

JETAPI	void JETCC jeThreadQueue_JobCreateRef(jeThreadQueue_Job *Job);
JETAPI	void JETCC jeThreadQueue_JobDestroy(jeThreadQueue_Job **Job);

JETAPI	jeThreadQueue_JobStatus	JETCC jeThreadQueue_JobGetStatus(const jeThreadQueue_Job *Job);

JETAPI	jeBoolean JETCC jeThreadQueue_JobSetPriority(
	jeThreadQueue_Job *		Job,
	jeThreadQueue_Priority	Priority);
				// will return false if the priority could not be changed 
				//	(eg. the thread was already running)
				// (not a fatal error)

JETAPI	jeThreadQueue_Priority JETCC jeThreadQueue_JobGetPriority(jeThreadQueue_Job * Job);
				// this does *not* return the result of SetPriority , but instead it
				//	returns the actual realized priority due to location in the queue

JETAPI	jeBoolean	JETCC jeThreadQueue_SetThreadLimit(int MaxThreads);
JETAPI	int			JETCC jeThreadQueue_GetThreadLimit(void);

JETAPI	void JETCC jeThreadQueue_Sleep(int Milliseconds);
				// don't use the Windows Sleep() use this

JETAPI	void JETCC jeThreadQueue_PollJobs(void);
				// warning : do NOT call this unless you are the master of the threads!
				//	calling this function too often will under-represent high priority threads!
				//	try to use WaitOnJob instead!

JETAPI jeBoolean JETCC jeThreadQueue_WaitOnJob(jeThreadQueue_Job * Job,
											jeThreadQueue_JobStatus WaitForStatus);
				//can wait for JE_THREADQUEUE_STATUS_RUNNING or JE_THREADQUEUE_STATUS_COMPLETED
				// waits for Status *or higher* !

// ----- use these Semaphores to lock data that ThreadQueue_Jobs may peek at.

JETAPI jeThreadQueue_Semaphore * JETCC jeThreadQueue_Semaphore_Create(void);
JETAPI void JETCC jeThreadQueue_Semaphore_Lock(		jeThreadQueue_Semaphore * S);
JETAPI void JETCC jeThreadQueue_Semaphore_UnLock(	jeThreadQueue_Semaphore * S);
JETAPI void JETCC jeThreadQueue_Semaphore_Destroy(	jeThreadQueue_Semaphore ** pS);

#ifndef NDEBUG
JETAPI	void JETCC jeThreadQueue_DumpQueue(void);			// uses stdio !
#else
#define jeThreadQueue_DumpQueue()
#endif

#ifndef NDEBUG
JETAPI void JETCC jeThreadQueue_GetDebugInfo(int * pActiveJobCount,int *pSemaphoreCount, int * pNumThreads);
#else
#define jeThreadQueue_GetDebugInfo(a,s,n)
#endif


#ifdef	__cplusplus
}
#endif

#endif

