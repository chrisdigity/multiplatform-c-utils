# Multiplatform Utilities
Convenient multiplatform utilities for C; including multithreading, mutex locks, millisecond sleep and high resolution milli/microsecond timestamp support.

### Available Usage
[Threading & Mutex](src/thread.c)...
```c
int thread_create(ThreadID *threadid, void *func, void *arg);
int thread_wait(ThreadID *threadid);
int thread_multiwait(ThreadID *tidlist, int len);
int mutex_init(Mutex *mutex);
int mutex_lock(Mutex *mutex);
int mutex_unlock(Mutex *mutex);
int mutex_end(Mutex *mutex);
```

[High Resolution Time & Sleep](src/time.c)...
```c
void millisleep(uint32_t ms);
uint64_t milliseconds(void);
uint64_t microseconds(void);
uint64_t millielapsed(uint64_t ms); /* via MACRO */
uint64_t microelapsed(uint64_t ms); /* via MACRO */
```

### Example usage
The [Multiplatform Utility tests](test/mputilstest.c) file is provided as an example of basic usage and testing, which validates the correct operation of functions (within operating tolerances where applicable).

#### Self Compilation and Execution:
Self compilation helper files, [testWIN.bat](testWIN.bat) & [testUNIX.sh](testUNIX.sh), are provided for easy compilation and execution of the [Multiplatform Utility tests](test/mputilstest.c) file.  
> testWIN.bat; Requires `Microsoft Visual Studio 2017 Community Edition` installed. Tested on x86_64 architecture running Windows 10 Pro v10.0.18362.  
> testUNIX.sh; Requires the `build-essential` package installed. Tested on x86_64 architecture running Ubuntu 16.04.1.

### More information
See individual source files for descriptions of the file, data type structures, compiler MACRO expansions, and function operations.
