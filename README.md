# Multiplatform Utilities

Convenient multiplatform utilities for C; including multithreading, mutex locks, read/write locks, millisecond sleep and high resolution milli/microsecond timestamp support.

### Available Usage

[Threading & Mutex header](src/mpthread.h)...
```c
int thread_create(ThreadID *threadid, Threaded *func, void *arg);
int thread_wait(ThreadID *threadid);
int thread_multiwait(ThreadID *tidlist, int len);
int mutex_init(Mutex *mutex);
int mutex_lock(Mutex *mutex);
int mutex_unlock(Mutex *mutex);
int mutex_end(Mutex *mutex);
int rwlock_init(RWLock *rwlock);
int rwlock_rdlock(RWLock *rwlock);
int rwlock_wrlock(RWLock *rwlock);
int rwlock_rdunlock(RWLock *rwlock);
int rwlock_wrunlock(RWLock *rwlock);
int rwlock_end(RWLock *rwlock);
```

[High Resolution Time & Sleep header](src/mptime.h)...
```c
void millisleep(unsigned long ms);
long milliseconds(void);
long microseconds(void);
long millielapsed(long ms);
long microelapsed(long ms);
```

### Example usage

The [Multiplatform Utilities](tests/mputils.c) test file is provided as an example of basic usage and testing, which validates the correct operation of functions (within operating tolerances where applicable).

#### Self Compilation and Execution:

On Windows...
> Requires `Microsoft Visual Studio 2017 Community Edition` installed.  
> Double-click the Windows Makefile [makefile.bat](tests/makefile.bat) to compile and run all `*.c` files in the `tests/` directory.

On Debian/Ubuntu...
> Requires the `build-essential` package installed.  
> Open a terminal in the `tests/` directory and run make...
```sh
make        # compile all *.c files
make test   # compile and test all *.c files
make clean  # cleanup all binaries and compilation files
```

### More information

See individual source files for descriptions of the file, usage notes, changelog, data type structures, compiler MACRO expansions, and function operations.
