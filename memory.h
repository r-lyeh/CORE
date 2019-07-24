// memory: realloc, vrealloc, stack, va
// @todo: gc, pool, game mark/rewind+game stack |level|game|>---<stack|

#ifndef MEMORY_H
#define MEMORY_H

// default allocator
void*  xrealloc(void* p, size_t sz);
size_t xlen(void* p);

// vector based allocator (x1.75 enlarge factor)
void*  vrealloc(void* p, size_t sz);
size_t vlen(void* p);

// stack based allocator
void*  stack(int bytes);

// local strings
char*  va(const char *fmt, ...);
int    va_hash( char *s );
int    va_len( char *s );
int    va_cmp( char *a, char *b );

#endif

// vector ----------------------------------------------------------------------

#ifdef MEMORY_C
#pragma once

#ifndef REALLOC
#define REALLOC realloc
#endif

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            REALLOC( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret = (size_t*)p - 2;
        if( !p ) {
            ret = (size_t*)REALLOC( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)REALLOC( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

// realloc ---------------------------------------------------------------------

#ifndef MSIZE
#   if defined _OSX || defined _BSD
#       define MSIZE  malloc_size
#   elif defined _AND
#       define MSIZE dlmalloc_usable_size
#   elif defined _WIN
#       define MSIZE _msize
#   else
#       define MSIZE malloc_usable_size // glibc
#   endif
#endif

void* xrealloc(void* ptr, size_t size) {
    ptr = REALLOC(ptr, size);
    if( size > 0 ) if( !ptr ) {
        // free(out_of_mem_guard_buffer), out_of_mem_guard_buffer = 0;
        exit( ENOMEM );
    }
#ifdef REALLOC_POISON
    if( size > 0 ) if( ptr ) {
        memset(ptr, 0xCD, size);
    }
#endif
    return ptr;
}
size_t xlen(void* p) {
    if( p ) return MSIZE(p);
    return 0;
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) {
    static __thread uint8_t *stack_mem = 0;
    static __thread uint64_t stack_size = 0, stack_max = 0;
    if( bytes < 0 ) {
        if( stack_size > stack_max ) stack_max = stack_size;
        return (stack_size = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xlen(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_size += bytes) - bytes ];
}

// local strings --------------------------------------------------------------

char* va( const char *fmt, ... ) {
    va_list vl;
    va_start(vl, fmt);
    int sz = vsnprintf( 0, 0, fmt, vl ) + 1;
    char* ptr = 4 + (char*)stack( sz +4 );
    vsnprintf( ptr, sz, fmt, vl );
    va_end(vl);

    uint64_t hash = 0; // fnv1a: 14695981039346656037ULL;
    for( int i = 0; i < sz; ++i ) { hash = ( ptr[i] ^ hash ) * 131; } // fnv1a: 0x100000001b3ULL; }
    union { uint64_t u; struct { uint8_t a,b,c,d,e,f,g,h; }; } x; x.u = hash;
    hash = ((x.a ^ x.b ^ x.c ^ x.d ^ x.e ^ x.f ^ x.g ^ x.h) << 24) | (sz - 1);
    *(uint32_t*)(ptr-4) = (uint32_t)hash;

    return (char *)ptr;
}
int va_hash( char *s ) {
    return *(uint32_t*)(&s[-4]);
}
int va_len( char *s ) {
    return ( (uint32_t)va_hash(s) ) & 0x00FFFFFF;
}
int va_cmp( char *a, char *b ) {
    uint32_t ha = va_hash(a), hb = va_hash(b);
    return ha == hb ? strcmp(a,b) : (hb & 0x00FFFFFF) - (ha & 0x00FFFFFF); // strncmp(a,b,len)
}

#endif
