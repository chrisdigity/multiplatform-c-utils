/* ****************************************************************
 * Test multiplatform utilities.
 *  - mptest.c (1 May 2020)
 *
 * Multiplatform utilities:
 * - Threading and Mutex locks
 * - Millisecond sleep and milli/microsecond high res time stamps
 *
 * NOTES:
 * - The "Timing tests w/ subsecond timing comparisons" is known to
 *   intermittently fail precision tests. This is presumed (but not
 *   confirmed) to be related to system load during testing.
 *
 * ****************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/thread.c"
#include "../src/time.c"

#define THREADS  1000
#define ROUNDS   100000
#define COUNT    100000000

#define MILLITEST_PRECISION  1
#define MICROTEST_PRECISION  10

/* Checks a value is within tolerance of an expected value. */
#define WITHIN_TOLERANCE(v,e,t)  ( v > (e - t) && v < (e + t) )

/****************************************************************/

/* Struct for passing multiple arugments to thread function. */
typedef struct {
   Mutex *mutexlock;
   int lockmethod;
   int nonvol_count;
   volatile int count;
} MTState;

/* Thread function testing various methods of mutex use to protect
 * multithreaded incrementing of a single variable. */
Threaded mts_inc(void *arg)
{
   MTState *mts;
   int i;

   mts = (MTState *) arg;

   if(mts->lockmethod > 1 && mts->lockmethod < 4)
      mutex_lock(mts->mutexlock);

   for(i = 0; i < ROUNDS; i++) {
      if(mts->lockmethod == 0) mts->nonvol_count++;
      else if(mts->lockmethod < 4) mts->count++;
   }

   if(mts->lockmethod > 1 && mts->lockmethod < 4)
      mutex_unlock(mts->mutexlock);

   if(mts->lockmethod == 4) {
      mutex_lock(mts->mutexlock);
      mts->count += i;
      mutex_unlock(mts->mutexlock);
   }

   return Treturn;
}

/****************************************************************/

int main()
{
   MTState mts;
   Mutex mts_mutex;
   Mutex mts_mutex_static = MUTEX_INITIALIZER;
   ThreadID threadlist[THREADS];
   uint64_t mstart, mexpected, mresult;
   uint64_t ustart, uexpected, uresult;
   float elapsed;
   time_t begin;
   int i, j, res, min, max, avg;

   printf("\n___________________\n");
   printf("Begin Multiplatform Utility tests...\n");


   printf("\nTiming tests w/ subsecond timing comparisons - time.c;\n");
   printf("  Synchronizing milli/microsecond... ");
   begin = time(NULL) + 1;

   mstart = milliseconds();
   ustart = microseconds();
   while(time(NULL) <= begin);
   uexpected = microelapsed(ustart);
   mexpected = millielapsed(mstart);
   printf("millisync: %dms, ", (int) mexpected - MILLISECONDS);
   printf("microsync: %dus\n", (int) uexpected - MICROSECONDS);

   for(i = 1; i < 6; i++) {
      if(i == 5) {
         mstart += 10 * MILLISECONDS;
         ustart += 10 * MICROSECONDS;
         mexpected -= 9 * MILLISECONDS;
         uexpected -= 9 * MICROSECONDS;
      } else {
         mexpected += MILLISECONDS;
         uexpected += MICROSECONDS;
      }
      if(i == 3) continue;
      if(i < 5)
         printf("  Timing test (+%d second)... ", i);
      else printf("  Timing test (overflow)...  ");

      while(time(NULL) <= begin + i);
      uresult = microelapsed(ustart);
      mresult = millielapsed(mstart);
      if(WITHIN_TOLERANCE(mresult, mexpected, MILLITEST_PRECISION))
         printf("milli: Pass! / ");
      else printf("milli: Failed. sec= %.03lf, exp= %.03lf / ",
                  (double) mresult / MILLISECONDS,
                  (double) mexpected / MILLISECONDS);
      if(WITHIN_TOLERANCE(uresult, uexpected, MICROTEST_PRECISION))
         printf("micro: Pass!\n");
      else printf("micro: Failed. sec= %.06lf, exp= %.06lf\n",
                  (double) uresult / MICROSECONDS,
                  (double) uexpected / MICROSECONDS);
   }


   printf("\nSleep accuracy tests w/ millisecond intervals - time.c;\n");
   printf("  Sleep duration (ms)... ");
   avg = max = 0;
   min = INT32_MAX;
   for(i = 0, j = 1000; j > 0; i++, j >>= 1) {
      if(i) printf("/");
      printf("%d", j);
      ustart = microseconds();
      millisleep(j);
      uresult = microelapsed(ustart);
      uexpected = j * MILLISECONDS;
      if(uresult > uexpected)
         res = (int) (uresult - uexpected);
      else res = (int) (uexpected - uresult);
      if(min > res) min = res;
      if(max < res) max = res;
      avg += res;
   }
   avg /= i;
   printf("\n");
   printf("  Sleep accuracy (us)... min/avg/max= %d/%d/%d, ", min, avg, max);
   if(avg < 1000)
      printf("Pass!\n");
   else printf("Failed.\n");


   printf("\nThreading and mutex tests w/ %d threads - thread.c;\n", THREADS);
   for(i = 0; i < 5; i++) {
      mts.count = 0;
      mts.nonvol_count = 0;
      mts.lockmethod = i;
      switch(i) {
         case 0:
            printf("  Non-volatile count, no Mutex guard... ");
            break;
         case 1:
            printf("  Volatile count, no Mutex guard...     ");
            break;
         case 2:
            printf("  Manually initialized Mutex guard...   ");
            mts.mutexlock = &mts_mutex;
            mutex_init(&mts_mutex);
            break;
         case 3:
            printf("  Statically initialized Mutex guard... ");
            mts.mutexlock = &mts_mutex_static;
            break;
         case 4:
            printf("  Intermediate counter, Mutex guard...  ");
            break;
         default:
            printf("Unknown Threading and Mutex test...\n");
            continue;
      }

      ustart = microseconds();
      for(j = 0; j < THREADS; j++)
         thread_create(&threadlist[j], mts_inc, &mts);
      thread_multiwait(threadlist, THREADS);
      elapsed = (float) microelapsed(ustart) / MICROSECONDS;

      printf("%9d in %.03fs, ", mts.count, elapsed);
      if(mts.count == COUNT)
         printf("Pass!\n");
      else if(mts.lockmethod > 1)
         printf("Failed.\n");
      else printf("Expected.\n");
   }

   return 0;
}
