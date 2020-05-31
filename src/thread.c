/* ****************************************************************
 * Multiplatform threading and mutex support.
 *  - thread.c (20 February 2020)
 *
 * The support functions in this file are based on the pthreads API
 * (POSIX Threads). Functionality is extended to Windows systems by
 * wrapping Windows API routines in functions imitating, as much as
 * reasonably possible, their associated pthreads counterparts.
 *
 * Since the Windows API lacks a static initialization method for a
 * CRITICAL_SECTION, the CRITICAL_SECTION is placed inside a struct
 * with a separate variable to track it's initialization status and
 * initialization will occur during the first mutex_lock() request.
 *
 * NOTES:
 * - Support functions requiring a Mutex or ThreadID type parameter
 *   MUST be passed as pointers.
 * - A Mutex can be statically initialized using MUTEX_INITIALIZER,
 *   RWLOCK can be statically initialized using RWLOCK_INITIALIZER.
 * - A function designed to run in a new thread SHALL be of format:
 *     // If multiple arguments are required, use a struct.
 *     Threaded thread_functionname(void *arg)
 *     {
 *        ... thread routine ...
 *        return Treturn;
 *     }
 *
 * CHANGELOG:
 * Rev.1   2020-02-20
 *   Initial Thread and Mutex implementation.
 * Rev.2   2020-04-30
 *   Added thread wait function for multiple threads.
 *   Added manual initialization method for Mutex.
 *   Initial Github repository release.
 * Rev.3   2020-05-31
 *   Added RWLock for shared read and exclusive write protection.
 *
 * ****************************************************************/

#ifndef _THREAD_C_
#define _THREAD_C_  /* include guard */


#include <stdint.h>


#ifdef _WIN32
/*********************************************/
/* ---------------- Windows ---------------- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define Threaded           DWORD
#define ThreadID           DWORD
#define Treturn            0
#define RWLock             SRWLOCK
#define MUTEX_INITIALIZER  {0}
#define RWLOCK_INITIALIZER SRWLOCK_INIT

/* There is no static initialization method for a CRITICAL_SECTION.
 * Therefore, on Windows, Mutex holds an initialization variable
 * to indicate the initialization status of the Mutex. */
typedef struct {
   CRITICAL_SECTION lock;
   volatile unsigned char init;
} Mutex;

/* Create a new thread on Windows and store it's thread id.
 * Return 0 on success, else error code. */
int thread_create(ThreadID *threadid, LPTHREAD_START_ROUTINE func, void *arg)
{
   if(CreateThread(NULL, 0, func, arg, 0, threadid) == NULL)
      return GetLastError();

   return 0;
}

/* Wait for a Windows thread to complete. (BLOCKING)
 * Returns 0 on success, else error code. */
int thread_wait(ThreadID *threadid)
{
   HANDLE hThread;
   int ecode = 0;

   /* acquire Thread HANDLE from thread id */
   hThread = OpenThread(SYNCHRONIZE, FALSE, *threadid);
   if(hThread == NULL)
      return GetLastError();

   /* wait (indefinitely) for thread to complete */
   if(WaitForSingleObject(hThread, INFINITE))
      ecode = GetLastError();
   /* close handle to thread */
   CloseHandle(hThread);
   /* ensure thread id is clean */
   *threadid = 0;

   return ecode;
}

/* Initialize a Windows Mutex and set initialized. Return 0.*/
int mutex_init(Mutex *mutex)
{
   /* initialize mutex */
   InitializeCriticalSection(&mutex->lock);
   /* set mutex initialized */
   mutex->init = 1;

   return 0;
}

/* Acquire an exclusive (Mutex) lock between Windows threads.
 * Return 0. */
int mutex_lock(Mutex *mutex)
{
   /* Mutex initialization is guarded by an initialization lock
    * which is 32-bit aligned as required by the Windows API
    * InterlockedCompareExchange() atomic function. */
   __declspec(align(4)) static volatile LONG initlock = 0;

   /* initialize mutex if not already */
   if(!mutex->init) {
      /* spin for exclusive init lock */
      while(InterlockedCompareExchange(&initlock, 1, 0));
      if(!mutex->init) {
         /* initialize mutex */
         InitializeCriticalSection(&mutex->lock);
         /* set mutex initialized */
         mutex->init = 1;
      }
      /* release init lock access */
      initlock = 0;
   }

   /* acquire exclusive lock */
   EnterCriticalSection(&mutex->lock);

   return 0;
}

/* Release an exclusive (Mutex) lock between Windows threads.
 * Return 0. */
int mutex_unlock(Mutex *mutex)
{
   LeaveCriticalSection(&mutex->lock);

   return 0;
}

/* Destroy a Windows Mutex and set uninitialized. Return 0. */
int mutex_end(Mutex *mutex)
{
   /* destroy mutex */
   DeleteCriticalSection(&mutex->lock);
   /* set mutex uninitialized */
   mutex->init = 0;

   return 0;
}

/* Initialize a Windows read/write lock. Return 0. */
int rwlock_init(RWLock *rwlock)
{
   InitializeSRWLock(rwlock);

   return 0;
}

/* Acquire a read lock (shared mode). Return 0. */
int rwlock_rdlock(RWLock *rwlock)
{
   AcquireSRWLockShared(rwlock);

   return 0;
}

/* Acquire a write lock (exclusive mode). Return 0. */
int rwlock_wrlock(RWLock *rwlock)
{
   AcquireSRWLockExclusive(rwlock);

   return 0;
}

/* Release a read lock. Return 0. */
int rwlock_rdunlock(RWLock *rwlock)
{
   ReleaseSRWLockShared(rwlock);

   return 0;
}

/* Release a write lock. Return 0. */
int rwlock_wrunlock(RWLock *rwlock)
{
   ReleaseSRWLockExclusive(rwlock);

   return 0;
}

/* SRW locks do not need to be explicitly destroyed. Therefore,
 * this function serves only as a placeholder for multiplatform
 * compatibility, always returning 0. */
int rwlock_end(RWLock *rwlock) { (void *) rwlock; return 0; }


#else /* end Windows */
/*********************/

/******************************************/
/* ---------------- UNIX ---------------- */

#include <pthread.h>

#define Threaded              void*
#define Treturn               NULL
#define ThreadID              pthread_t
#define Mutex                 pthread_mutex_t
#define RWLock                pthread_rwlock_t
#define MUTEX_INITIALIZER     PTHREAD_MUTEX_INITIALIZER
#define RWLOCK_INITIALIZER    PTHREAD_RWLOCK_INITIALIZER

/* Create a new thread on UNIX and store it's thread id.
 * Return 0 on success, else error code. */
int thread_create(ThreadID *threadid, void *func, void *arg)
{ 
   return pthread_create(threadid, NULL, func, arg);
}

/* Wait for a UNIX thread to complete. (BLOCKING)
 * Returns 0 on success, else error code. */
int thread_wait(ThreadID *threadid)
{
   int ecode;

   /* wait (indefinitely) for thread to complete */
   ecode = pthread_join(*threadid, NULL);
   /* ensure thread id is clean */
   *threadid = 0;

   return ecode;
}

/* Initialize a UNIX Mutex. Return 0 on success, else error code. */
int mutex_init(Mutex *mutex)
{
   return pthread_mutex_init(mutex, NULL);
}

/* Acquire an exclusive (Mutex) lock between UNIX threads.
 * Return 0 on success, else error code. */
int mutex_lock(Mutex *mutex)
{
   return pthread_mutex_lock(mutex);
}

/* Release an exclusive (Mutex) lock between UNIX threads.
 * Return 0 on success, else error code. */
int mutex_unlock(Mutex *mutex)
{
   return pthread_mutex_unlock(mutex);
}

/* Destroy a UNIX Mutex. Return 0 on success, else error code. */
int mutex_end(Mutex *mutex)
{
   return pthread_mutex_destroy(mutex);
}

/* Initialize a Windows read/write lock. Return 0. */
int rwlock_init(RWLock *rwlock)
{
   return pthread_rwlock_init(rwlock, NULL);
}

/* Acquire a shared read lock. Return 0. */
int rwlock_rdlock(RWLock *rwlock)
{
   return pthread_rwlock_rdlock(rwlock);
}

/* Acquire an exclusive write lock. Return 0. */
int rwlock_wrlock(RWLock *rwlock)
{
   return pthread_rwlock_wrlock(rwlock);
}

/* Release a read lock. Return 0. */
int rwlock_rdunlock(RWLock *rwlock)
{
   return pthread_rwlock_unlock(rwlock);
}

/* Release a write lock. Return 0. */
int rwlock_wrunlock(RWLock *rwlock)
{
   return pthread_rwlock_unlock(rwlock);
}

/* Destroy a read/write lock. Returns 0, else error code. */
int rwlock_end(RWLock *rwlock)
{
   return pthread_rwlock_destroy(rwlock);
}


#endif /* end UNIX */
/*******************/

/* Thread structure containing the thread id, arg pointer and "done" flag.
 * Useful for obtaining thread state without performing a blocking
 * thread_wait() call. */
typedef struct {
   ThreadID id;
   void *arg;
   unsigned char done;
} THREAD;

/* Wait for multiple threads to complete. (BLOCKING)
 * Expects a pointer to a `len` length array of thread id's.
 * Returns 0 on success, else the first error code. */
int thread_multiwait(ThreadID *tidlist, int len)
{
   int i, temp, ecode;

   ecode = temp =0;
   for(i = 0; i < len; i++) {
      temp = thread_wait(&tidlist[i]);
      if(temp && !ecode)
         ecode = temp;
   }

   return ecode;
}


#endif /* end _THREAD_C_ */
