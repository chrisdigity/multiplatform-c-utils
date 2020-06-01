/* ****************************************************************
 * Multipatform sleep and time stamp support.
 *  - time.c (1 February 2020)
 *
 * This file provides support for sleep with millisecond precision,
 * as well as time stamps with milli and microsecond precision.
 *
 * ****************************************************************/

#ifndef _TIME_C_
#define _TIME_C_  /* include guard */


#include <stdint.h>

#define MILLISECONDS 1000
#define MICROSECONDS 1000000

/* Measure elapsed milliseconds since a previous millisecond time stamp. */
#define millielapsed(ms)  ( milliseconds() - ms )
/* Measure elapsed microseconds since a previous microsecond time stamp. */
#define microelapsed(us)  ( microseconds() - us )


#ifdef _WIN32
/*********************************************/
/* ---------------- Windows ---------------- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static LARGE_INTEGER Freq_timec;

/* Suspend the current Windows thread for specified milliseconds. */
void millisleep(uint32_t ms)
{
   /* Windows API millisecond sleep function */
   Sleep(ms);
}

/* Retrieve a high resolution time stamp in milliseconds,
 * independent of any external time reference. */
uint64_t milliseconds(void)
{
   LARGE_INTEGER count;

   /* performance frequency need only be acquired once */
   if(Freq_timec.QuadPart == 0)
      QueryPerformanceFrequency(&Freq_timec);
   /* obtain performance counter */
   QueryPerformanceCounter(&count);

   /* return high resolution timer calculation */
   return (uint64_t) ((count.QuadPart * MILLISECONDS) / Freq_timec.QuadPart);
}

/* Retrieve a high resolution time stamp in microseconds,
 * independent of any external time reference. */
uint64_t microseconds(void)
{
   LARGE_INTEGER count;

   /* performance frequency need only be acquired once */
   if(Freq_timec.QuadPart == 0)
      QueryPerformanceFrequency(&Freq_timec);
   /* obtain performance counter */
   QueryPerformanceCounter(&count);

   /* return high resolution timer calculation */
   return (uint64_t) ((count.QuadPart * MICROSECONDS) / Freq_timec.QuadPart);
}


#else /* end Windows */
/*********************/

/******************************************/
/* ---------------- UNIX ---------------- */

#include <sys/time.h>

/* Suspend the current UNIX thread for specified milliseconds. */
void millisleep(uint32_t ms)
{
   struct timespec ts;
   uint32_t sec;

   /* obtain seconds from milliseconds */
   sec = ms / MILLISECONDS;
   ts.tv_sec = sec;
   /* derive nanoseconds without whole seconds */
   ts.tv_nsec = (ms - (sec * MILLISECONDS)) * 1000000;

   /* use POSIX compliant sleep */
   nanosleep(&ts, &ts);
   /* for uninterruptible sleep instead...
   while(nanosleep(&ts, &ts) != 0); */
}

/* Retrieve and set the time of the specified clock ID.
 * Returns a struct timespec with the current time. */
static inline struct timespec ts_gettime(void)
{
   struct timespec ts;

   /* Monotonic Clock takes precedence, Realtime Clock
    * is both compiler and runtime fallback */
#ifdef CLOCK_MONOTONIC
   if(clock_gettime(CLOCK_MONOTONIC, &ts))
#endif
      clock_gettime(CLOCK_REALTIME, &ts);

   return ts;
}

/* Retrieve a high resolution time stamp in milliseconds, using
 * some unspecified starting point (default:CLOCK_MONOTONIC) or
 * using system-wide realtime (fallback:CLOCK_REALTIME). */
uint64_t milliseconds(void)
{
   struct timespec ts = ts_gettime();

   /* return high resolution time calculation */
   return ((uint64_t) ts.tv_sec * MILLISECONDS) + (ts.tv_nsec / 1000000);
}

/* Retrieve a high resolution time stamp in microseconds, using
 * some unspecified starting point (default:CLOCK_MONOTONIC) or
 * using system-wide realtime (fallback:CLOCK_REALTIME). */
uint64_t microseconds(void)
{
   struct timespec ts = ts_gettime();

   /* return high resolution time calculation */
   return ((uint64_t) ts.tv_sec * MICROSECONDS) + (ts.tv_nsec / 1000);
}


#endif /* end UNIX */
/*******************/


#endif /* end _TIME_C_ */
