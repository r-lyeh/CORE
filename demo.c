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

#include <errno.h>
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

// tests
int pipe_print( void* buf, int len ) {
    printf("%.*s\n", len, (char*)buf );
    return len;
}
int pipe_lowercase( void* buf0, int len ) {
    char *buf = (char*)buf0;
    for( int i = 0; i < len; ++i ) buf[i] = tolower(buf[i]);
    return len;
}
int pipe_uppercase( void* buf0, int len ) {
    char *buf = (char*)buf0;
    for( int i = 0; i < len; ++i ) buf[i] = toupper(buf[i]);
    return len;
}
int tests() {
    // demo: error.c
    int passed = 0; assert(++passed); // check that assert() works in release builds
    LOGINFO(#main #init, " %s assert() - %s", passed ? "working" : "faulty", ERRORTEXT);
    hexdump("hello world\n", 12+1);

    // demo: memory.c
    int *t = stack(123); *t = 42; assert( *t == 42 );
    int *p = xrealloc(0, sizeof(int)); *p = 42; assert( *p == 42 ); assert( xlen(p) == sizeof(int) );
    int *v = vrealloc(0, sizeof(int)); *v = 42; assert( *v == 42 ); assert( vlen(v) >= sizeof(int) );
    char*s = va("hello world %d", 123); assert( 0 == strcmp(s, "hello world 123") );
    printf("str:%s hash:%x len:%d cmp:%d\n", s, va_hash(s), va_len(s), va_cmp(s, va("hello world %d", 123)));

    // demo: quarks
    int id1 = quark_intern("hello world"); assert(id1 == 1);
    int id2 = quark_intern("hello world"); assert(id2 == 2);
    printf("#%d: %s (%d bytes) (%x hash) (%d cmp)\n", id1, quark_str(id1), quark_len(id1), quark_hash(id1), quark_cmp(id1, id2));

    // demo: streams
    {
        // files
        char data[16] = {0};
        STREAM *fp = stream_open("file://" __FILE__, "rb");
        stream_pull( fp, data, 16 );
        stream_shut( fp );
        printf("%.*s\n",16,data);

        // mem:// to specific buffer
        char buf[16] = {0};
        fp = stream_open( "mem://", buf, buf+16 );
        stream_puts(fp, "hello world %d", 123);
        stream_shut(fp);
        puts( buf );

        // mem:// to self-growing buffer
        fp = stream_open( "mem://", NULL, NULL );
        stream_push(fp, "hello world", 12);
        printf( "%s (%d bytes)\n", 0[(char**)fp], stream_tell(fp) );
        stream_shut(fp);

        // linked streams: one to many
        char buf1[16] = {0}, buf2[16] = {0}, buf3[16] = {0};
        fp = stream_open("mem://", buf1, buf1+16);
        stream_link( fp, stream_open("mem://", buf2, buf2+16) );
        stream_link( fp, stream_open("mem://", buf3, buf3+16) );
        stream_push( fp, "Hi!", 4 );
        stream_shut( fp );
        puts(buf1);
        puts(buf2);
        puts(buf3);

        // pipes: lowercase -> print -> uppercase -> print when pushing
        fp = stream_open( "null://", NULL, NULL );
        stream_pipe( fp, pipe_lowercase );
        stream_pipe( fp, pipe_print );
        stream_pipe( fp, pipe_uppercase );
        stream_pipe( fp, pipe_print );
        const char *text = "This Text Should Be Printed Differently Twice";
        stream_push( fp, text, strlen(text) );
        puts(stream_info(fp));
        stream_shut( fp );
    }

    return 0;
}

// demo

#define TITLE "demo"
#define VERSION 0100

int tests();

int main() {
    LOGINFO(#init, "%s v%o", TITLE, VERSION);

    tests();
    //event_loop();

    LOGEXIT(#quit, "done.");
}
