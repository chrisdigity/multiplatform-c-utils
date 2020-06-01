/* ****************************************************************
 * Multiplatform threading and mutex support.
 *  - mpthread.h (20 February 2020)
 *
 * Original work Copyright (c) 2020 Zalamanda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * ****************************************************************
 * This file is designed as a bridge between platform specific code
 * already present on most systems.
 *
 * The support functions in this file are based on the pthreads API
 * (POSIX Threads). Functionality is extended to Windows systems by
 * wrapping Windows API routines in functions imitating, as much as
 * reasonably possible, their associated pthreads counterparts.
 *
 * NOTES:
 * - Support functions requiring a ThreadID, Mutex or RWLock param,
 *   SHALL be passed as pointers.
 * - A Mutex can be statically initialized using MUTEX_INITIALIZER,
 *   RWLock can be statically initialized using RWLOCK_INITIALIZER.
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
 * Rev.4   2020-05-31
 *   File overhaul and conversion to header file.
 *   Changed functions to MACRO redefinitions where appropriate.
 *
 * ****************************************************************/

#ifndef _MP_THREAD_H_
#define _MP_THREAD_H_  /* include guard */


#ifdef _WIN32
/*********************************************/
/* ---------------- Windows ---------------- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Windows function redefinitions */
#define rwlock_free()  0  /* SRWLock need not be explicitly destroyed */

/* Windows static initializers */
#define MUTEX_INITIALIZER   {0}
#define RWLOCK_INITIALIZER  SRWLOCK_INIT

/* Windows datatypes and structs */
#define ThreadID  DWORD    /* thread identification datatype */
#define Threaded  DWORD    /* thread execution function datatype */
#define Treturn   0        /* thread function return value */
#define RWLock    SRWLOCK  /* shared read / exclusive write lock */

/* A Mutually exclusive lock datatype, utilizing Windows' CRITICAL_SECTION
 * to more closely imitate pthread's pthread_mutex_t element. Since there
 * is no static initialization method for a CRITICAL_SECTION, the struct
 * also holds an initialization variable to indicate the initialization
 * status of the CRITICAL_SECTION. If static initialization is chosen,
 * actual initialization will occur during the first call to mutex_lock(). */
typedef struct _Mutex {
   CRITICAL_SECTION lock;
   volatile unsigned char init;
} Mutex;

/* Create a new thread on Windows and store it's thread identifier.
 * Return 0 on success, else GetLastError(). */
static inline int
thread_create(ThreadID *threadid, LPTHREAD_START_ROUTINE func, void *arg)
{
   if(CreateThread(NULL, 0, func, arg, 0, threadid) == NULL)
      return GetLastError();

   return 0;
}

/* Wait for a thread on Windows to complete. (BLOCKING)
 * Returns 0 on success, else GetLastError(). */
static inline int thread_wait(ThreadID *threadid)
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

   return ecode;
}

/* Initialize a Mutex on Windows and set initialized.
 * Always returns 0 on Windows. */
static inline int mutex_init(Mutex *mutex)
{
   /* initialize critical section inside Mutex */
   InitializeCriticalSection(&mutex->lock);
   /* set Mutex initialized */
   mutex->init = 1;

   return 0;
}

/* Acquire an exclusive lock on Windows.
 * Always returns 0 on Windows. */
static inline int mutex_lock(Mutex *mutex)
{
   /* CRITICAL_SECTION initialization is guarded by an initialization
    * lock which is 32-bit aligned as required by the Windows API's
    * InterlockedCompareExchange() atomic function. */
   __declspec(align(4)) static volatile LONG initlock = 0;

   /* initialize mutex if not already */
   if(!mutex->init) {
      /* acquire exclusive spin lock access */
      while(InterlockedCompareExchange(&initlock, 1, 0));
      /* perform initialization if first */
      if(!mutex->init)
         mutex_init(mutex);
      /* release spin lock access */
      initlock = 0;
   }

   /* acquire exclusive critical section lock */
   EnterCriticalSection(&mutex->lock);

   return 0;
}

/* Release an exclusive lock on Windows.
 * Always returns 0 on Windows. */
static inline int mutex_unlock(Mutex *mutex)
{
   /* release exclusive critical section lock */
   LeaveCriticalSection(&mutex->lock);

   return 0;
}

/* Uninitalize a Mutex on Windows.
 * Always returns 0 on Windows. */
static inline int mutex_free(Mutex *mutex)
{
   /* destroy critical section */
   DeleteCriticalSection(&mutex->lock);
   /* set Mutex uninitialized */
   mutex->init = 0;

   return 0;
}

/* Read write lock (RWLock) functions on Windows.
 * Always returns 0 on Windows. */
static inline int rwlock_init(RWLock *rwlock)
{ InitializeSRWLock(rwlock); return 0; }

static inline int rwlock_rdlock(RWLock *rwlock)
{ AcquireSRWLockShared(rwlock); return 0; }

static inline int rwlock_wrlock(RWLock *rwlock)
{ AcquireSRWLockExclusive(rwlock); return 0; }

static inline int rwlock_rdunlock(RWLock *rwlock)
{ ReleaseSRWLockShared(rwlock); return 0; }

static inline int rwlock_wrunlock(RWLock *rwlock)
{ ReleaseSRWLockExclusive(rwlock); return 0; }


#else /* end Windows */
/*********************/

/*******************************************/
/* ---------------- POSIX ---------------- */

#include <pthread.h>

/* POSIX function redefinitions... */
   /* ... threading functions, return 0 on success else error code. */
#define thread_create(tid,func,arg)  pthread_create(tid,NULL,func,arg)
#define thread_wait(tid)             pthread_join(*(tid),NULL)  /* BLOCKING */
   /* ... mutex lock functions, return 0 on success else error code. */
#define mutex_init(m)    pthread_mutex_init(m,NULL)
#define mutex_lock(m)    pthread_mutex_lock(m)  /* BLOCKING */
#define mutex_unlock(m)  pthread_mutex_unlock(m)
#define mutex_free(m)    pthread_mutex_destroy(m)
   /* ... read write lock functions, return 0 on success else error code. */
#define rwlock_init(rwl)      pthread_rwlock_init(rwl,NULL)
#define rwlock_rdlock(rwl)    pthread_rwlock_rdlock(rwl)  /* BLOCKING */
#define rwlock_wrlock(rwl)    pthread_rwlock_wrlock(rwl)  /* BLOCKING */
#define rwlock_rdunlock(rwl)  pthread_rwlock_unlock(rwl)
#define rwlock_wrunlock(rwl)  pthread_rwlock_unlock(rwl)
#define rwlock_free(rwl)      pthread_rwlock_destroy(rwl)

/* POSIX static initializers */
#define MUTEX_INITIALIZER   PTHREAD_MUTEX_INITIALIZER
#define RWLOCK_INITIALIZER  PTHREAD_RWLOCK_INITIALIZER

/* POSIX datatypes and structs */
#define ThreadID  pthread_t         /* thread identification datatype */
#define Threaded  void*             /* thread execution function datatype */
#define Treturn   NULL              /* thread function return value */
#define Mutex     pthread_mutex_t   /* mutually exclusive lock */
#define RWLock    pthread_rwlock_t  /* shared read / exclusive write lock */


#endif /* end POSIX */
/********************/

/**********************************************************/
/* ---------------- Platform independant ---------------- */

/* Thread structure containing a thread id, argument pointer and "done"
 * flag. Intended for obtaining thread state without performing a
 * blocking thread_wait() call. */
typedef struct _THREAD_CTX {
   ThreadID id;
   void *arg;
   unsigned char done;
} THREAD_CTX;

/* Wait for multiple threads to complete. (BLOCKING)
 * Expects a pointer to a `len` length array of thread id's.
 * Returns 0 on success, else the first error code. */
static inline int thread_multiwait(ThreadID *tidlist, int len)
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


#endif /* end _MP_THREAD_H_ */
