// if/n/def hell + system headers here, rather than in every header.

#  if defined __ANDROID_API__
#define _AND
#elif defined __APPLE__
#define _OSX
#elif defined __FreeBSD__
#define _BSD
#elif defined _WIN32
#define _WIN
#elif
#define _LIN
#endif

#ifdef _AND
#include <dlfcn.h>
#include <malloc.h>
size_t dlmalloc_usable_size(void*); // (android)
#endif

#ifdef _BSD
#include <dlfcn.h>
#include <malloc/malloc.h>
#endif

#ifdef _IOS
#include <mach-o/dyld.h>
#include <malloc/malloc.h>
#endif

#ifdef _LIN
#include <dlfcn.h>
#include <malloc.h>
#endif

#ifdef _OSX
#include <mach-o/dyld.h>
#include <malloc/malloc.h>
#endif

#ifdef _WIN
#include <malloc.h>
#include <winsock2.h>
#endif

// all required standard headers

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// include core

#define DS_C
#define ERROR_C
#define MEMORY_C
#define OBJECT_C
#define STREAM_C
#define TYPES_C
#define VM_C

#include "ds.h"     //   0%
#include "error.h"  //   0%
#include "memory.h" //   0%
#include "object.h" //   0%
#include "stream.h" //   0%
#include "types.h"  //   0%
#include "vm.h"     //   0%

// demo

#define TITLE "demo"
#define VERSION 0100

int main() {
    // error.c demo
    int passed = 0; assert(++passed); // check that assert() works in release builds
    LOGINFO(#main #init, "%s v%o", TITLE, VERSION);
    LOGINFO(#main #init, " %s assert() - %s", passed ? "working" : "faulty", ERRORTEXT);

    LOGEXIT(#main #quit, "exiting...");
}
