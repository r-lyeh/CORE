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

#include "ds.h"     // 4/17
#include "error.h"  // 4/ 4
#include "memory.h" // 4/ 4
#include "object.h" // 0/ 5
#include "stream.h" // 0/11
#include "types.h"  // 7/19
#include "vm.h"     // 0/18

// demo

#define TITLE "demo"
#define VERSION 0100

int main() {
    // demo: error.c
    int passed = 0; assert(++passed); // check that assert() works in release builds
    LOGINFO(#main #init, "%s v%o", TITLE, VERSION);
    LOGINFO(#main #init, " %s assert() - %s", passed ? "working" : "faulty", ERRORTEXT);
    hexdump("hi", 2);

    // demo: memory.c
    int *t = stack(123); *t = 42;
    int *p = xrealloc(0, sizeof(int)); *p = 42; assert( xlen(p) == sizeof(int) );
    int *v = vrealloc(0, sizeof(int)); *v = 42; assert( vlen(v) >= sizeof(int) );
    char*s = va("hello world %d", 123);
    printf("str:%s hash:%x len:%d cmp:%d\n", s, va_hash(s), va_len(s), va_cmp(s, va("hello world %d", 123)));

    // demo: quarks
    int id1 = quark_intern("hello world");
    int id2 = quark_intern("hello world");
    printf("#%d: %s (%d bytes) (%x hash) (%d cmp)\n", id1, quark_str(id1), quark_len(id1), quark_hash(id1), quark_cmp(id1, id2));

    LOGEXIT(#main #quit, "exiting...");
}
