/* ****************************************************************
 * Multiplatform sleep and time stamp support.
 *  - mptime.h (1 February 2020)
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
 * This file provides support for sleep with millisecond precision,
 * as well as time stamps with milli and microsecond precision.
 *
 * CHANGELOG:
 * Rev.1   2020-02-1
 *   Initial millisleep and microseconds timestamp implementation.
 * Rev.2   2020-04-30
 *   Added milliseconds timestamp function.
 *   Added millielapsed and microelapsed function macros.
 *   Initial Github repository release.
 * Rev.4   2020-06-0`
 *   File overhaul and conversion to header file.
 *   Changed functions to MACRO redefinitions where appropriate.
 *   Removed stdint.h in favour of standard datatypes.
 *
 * ****************************************************************/

#ifndef _MP_TIME_H_
#define _MP_TIME_H_  /* include guard */


#define MILLISECONDS 1000L
#define MICROSECONDS 1000000L

/* Measure elapsed milliseconds since a previous millisecond time stamp. */
#define millielapsed(ms)  ( milliseconds() - ms )
/* Measure elapsed microseconds since a previous microsecond time stamp. */
#define microelapsed(us)  ( microseconds() - us )


#ifdef _WIN32
/*********************************************/
/* ---------------- Windows ---------------- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Suspend the current thread for specified milliseconds. */
#define millisleep(ms)  Sleep(ms)

/* Windows performance counter frequency */
static LARGE_INTEGER Freq_mptime;

/* Retrieve a high resolution time stamp, in milliseconds,
 * independent of any external time reference.
 * NOTE: Assuming 4 byte long, time range is ±24.855 days.
 * Returns long integer. */
static inline long milliseconds(void)
{
   LARGE_INTEGER count;

   /* performance frequency need only be acquired once */
   if(Freq_mptime.QuadPart == 0)
      QueryPerformanceFrequency(&Freq_mptime);
   /* obtain performance counter */
   QueryPerformanceCounter(&count);

   /* return high resolution timer calculation */
   return (long) ((count.QuadPart * MILLISECONDS) / Freq_mptime.QuadPart);
}

/* Retrieve a high resolution time stamp, in microseconds,
 * independent of any external time reference.
 * NOTE: Assuming 4 byte long, time range is ±35.791 minutes.
 * Returns long integer. */
static inline long microseconds(void)
{
   LARGE_INTEGER count;

   /* performance frequency need only be acquired once */
   if(Freq_mptime.QuadPart == 0)
      QueryPerformanceFrequency(&Freq_mptime);
   /* obtain performance counter */
   QueryPerformanceCounter(&count);

   /* return high resolution timer calculation */
   return (long) ((count.QuadPart * MICROSECONDS) / Freq_mptime.QuadPart);
}


#else /* end Windows */
/*********************/

/*******************************************/
/* ---------------- POSIX ---------------- */

#include <sys/time.h>

/* Suspend the current thread for specified milliseconds. */
static inline void millisleep(unsigned long ms)
{
   struct timespec ts;

   /* obtain seconds from milliseconds */
   ts.tv_sec = (time_t) (ms / MILLISECONDS);
   /* derive nanoseconds without whole seconds */
   ts.tv_nsec = (long) ((ms - (ts.tv_sec * MILLISECONDS)) * 1000000L);

   /* use POSIX compliant sleep */
   nanosleep(&ts, &ts); /* while(nanosleep(&ts,&ts)!=0); //uninterruptible */
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
 * using system-wide realtime (fallback:CLOCK_REALTIME).
 * NOTE: Assuming 4 byte long, time range is ±24.855 days.
 * Returns long integer. */
static inline long milliseconds(void)
{
   struct timespec ts = ts_gettime();

   /* return high resolution time calculation */
   return ((long) ts.tv_sec * MILLISECONDS) + (ts.tv_nsec / 1000000L);
}

/* Retrieve a high resolution time stamp in microseconds, using
 * some unspecified starting point (default:CLOCK_MONOTONIC) or
 * using system-wide realtime (fallback:CLOCK_REALTIME).
 * NOTE: Assuming 4 byte long, time range is ±35.791 minutes.
 * Returns long integer. */
static inline long microseconds(void)
{
   struct timespec ts = ts_gettime();

   /* return high resolution time calculation */
   return ((long) ts.tv_sec * MICROSECONDS) + (ts.tv_nsec / 1000L);
}


#endif /* end POSIX */
/********************/


#endif /* end _MP_TIME_H_ */
