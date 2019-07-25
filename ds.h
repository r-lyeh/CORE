// data structures: array, map(set), lfqueue, hash, sort, comp,
// async: thread_local (ctor/dtor), thread, mutex, channel, coroutines
// time: sleep/yield, clock, timer(tmr,fn,time)/cancel(tmr)/every(tmr,fn,time)

#ifndef DS_H
#define DS_H

// thread-local ---------------------------------------------------------------

#if !defined __thread && defined _MSC_VER
#define __thread __declspec(thread)
#endif

// time -----------------------------------------------------------------------

uint64_t time_ss();
uint64_t time_ms();
uint64_t time_us();
uint64_t time_ns();

// hash -----------------------------------------------------------------------

uint64_t hash_f(double dbl);
uint64_t hash_i(uint64_t key);
uint64_t hash_s(const char* str);
uint64_t hash_p(const void* ptr);
uint64_t hash_2(const float pt[2]);
uint64_t hash_3(const float pt[3]);
uint64_t hash_4(const float pt[4]);

// array library --------------------------------------------------------------

#ifdef __cplusplus
#define array_cast(x) (decltype(x))
#else
#define array_cast(x) (void *)
#endif

#define array(t) t*
#define array_init(t) ( (t) = 0 )
#define array_resize(t, n) ( memset( array_count(t) + ((t) = array_cast(t) vrealloc((t), (n) * sizeof(0[t]) )), 0, ((n)-array_count(t)) * sizeof(0[t]) ), (t) )
#define array_push(t, ...) ( (t) = array_cast(t) vrealloc((t), (array_count(t) + 1) * sizeof(0[t]) ), (t)[ array_count(t) - 1 ] = (__VA_ARGS__) )
#define array_pop(t) ( (t) = array_cast(t) vrealloc((t), (array_count(t) - 1) * sizeof(0[t]) ) )
#define array_back(t) ( (t) ? &(t)[ array_count(t)-1 ] : NULL )
#define array_data(t) (t)
#define array_at(t,i) (t[i])
#define array_count(t) (int)( (t) ? vlen(t) / sizeof(0[t]) : 0u )
#define array_bytes(t) (int)( (t) ? vlen(t) : 0u )
#define array_sort(t, cmpfunc) qsort( t, array_count(t), sizeof(t[0]), cmpfunc )
#define array_empty(t) ( !array_count(t) )
#define array_clear(t) ( array_cast(t) vrealloc((t), 0), (t) = 0 )
#define array_free(t) array_clear(t)

#define array_reverse(t) \
    do if( array_count(t) ) { \
        for(int l = array_count(t), e = l-1, i = (array_push(t, 0[t]), 0); i <= e/2; ++i ) \
            { l[t] = i[t]; i[t] = (e-i)[t]; (e-i)[t] = l[t]; } \
        array_pop(t); \
    } while(0)

#define array_foreach(t,val_t,v) \
    for( val_t *v = &0[t]; v < (&0[t] + array_count(t)); ++v )

#define array_search(t, key, cmpfn) /* requires sorted array beforehand */ \
    bsearch(&key, t, array_count(t), sizeof(t[0]), cmpfn )

#define array_insert(t, i, n) do { \
    int ac = array_count(t); \
    if( i >= ac ) { \
        array_push(t, n); \
    } else { \
        (t) = vrealloc( (t), (ac + 1) * sizeof(t[0]) ); \
        memmove( &(t)[(i)+1], &(t)[i], (ac - (i)) * sizeof(t[0]) ); \
        (t)[ i ] = (n); \
    } \
} while(0)

#define array_copy(t, src) do { \
    array_free(t); \
    (t) = vrealloc( (t), array_count(src) * sizeof(0[t])); \
    memcpy( (t), src, array_count(src) * sizeof(0[t])); \
} while(0)

#define array_erase(t, i) do { \
    memcpy( &(t)[i], &(t)[array_count(t) - 1], sizeof(0[t])); \
    (t) = vrealloc((t), (array_count(t) - 1) * sizeof(0[t])); \
} while(0)

#define array_unique(t, cmpfunc) do { \
    int cnt = array_count(t), dupes = 0; \
    if( cnt > 1 ) { \
        const void *prev = &(t)[0]; \
        array_sort(t, cmpfunc); \
        for( int i = 1; i < cnt; ) { \
            if( cmpfunc(&t[i], prev) == 0 ) { \
                memmove( &t[i], &t[i+1], (cnt - 1 - i) * sizeof(t[0]) ) ; \
                --cnt; \
                ++dupes; \
            } else { \
                prev = &(t)[i]; \
                ++i; \
            } \
        } \
        if( dupes ) { \
            (t) = vrealloc((t), (array_count(t) - dupes) * sizeof(0[t])); \
        } \
    } \
} while(0)

/*
#define array_reserve(t, n) do { \
    int osz = array_count(t); \
    (t) = vrealloc( (t), (n) * sizeof(t[0]) ); \
    (t) = vresize( (t), osz * sizeof(t[0]) ); \
} while(0)
*/

#endif

// ----------------------------------------------------------------------------


#ifdef DS_C
#pragma once

#define TIME_E3 1000ULL
#define TIME_E6 1000000ULL
#define TIME_E9 1000000000ULL

#ifdef                 CLOCK_MONOTONIC_RAW
#define TIME_MONOTONIC CLOCK_MONOTONIC_RAW
#elif defined          CLOCK_MONOTONIC
#define TIME_MONOTONIC CLOCK_MONOTONIC
#else
#define TIME_MONOTONIC CLOCK_REALTIME // untested
#endif

uint64_t time_ns() {
    uint64_t nanotimer;
    #if defined _WIN
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        nanotimer = (uint64_t)li.QuadPart;
    #elif defined _AND
        nanotimer = (uint64_t)clock();
    #elif defined TIME_MONOTONIC
        struct timespec ts;
        clock_gettime(TIME_MONOTONIC, &ts);
        nanotimer = (TIME_E9 * (uint64_t)ts.tv_sec) + ts.tv_nsec;
    #else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        nanotimer = (TIME_E6 * (uint64_t)tv.tv_sec) + tv.tv_usec;
    #endif

    static uint64_t epoch = 0, nanofreq = 0;
    if( !nanofreq ) {
        #if defined _WIN
            LARGE_INTEGER li;
            QueryPerformanceFrequency(&li);
            nanofreq = li.QuadPart;
        #elif defined _AND
            nanofreq = CLOCKS_PER_SEC;
        #elif defined TIME_MONOTONIC
            nanofreq = TIME_E9;
        #else
            nanofreq = TIME_E6;
        #endif
        epoch = nanotimer;
    }

    // [ref] https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
    // Computes (a*b)/c without overflow, as long as both (a*b) and the overall result fit into 64-bits.
    uint64_t a = nanotimer - epoch;
    uint64_t b = TIME_E9;
    uint64_t c = nanofreq;
    uint64_t q = a / c;
    uint64_t r = a % c;
    return q * b + r * b / c;
}
uint64_t time_us() {
    return time_ns() / TIME_E3;
}
uint64_t time_ms() {
    return time_ns() / TIME_E6;
}
uint64_t time_ss() {
    return time_ns() / TIME_E9;
}

// ----------------------------------------------------------------------------

uint64_t hash_i(uint64_t key) {
    /* Thomas Wang's 64 bit Mix Function (public domain) http://www.cris.com/~Ttwang/tech/inthash.htm */
    key += ~(key << 32);
    key ^=  (key >> 22);
    key += ~(key << 13);
    key ^=  (key >>  8);
    key +=  (key <<  3);
    key ^=  (key >> 15);
    key += ~(key << 27);
    key ^=  (key >> 31);
    return key;
}

uint64_t hash_p(const void *ptr) {
    uint64_t key;
    memcpy(&key, ptr, sizeof(uint64_t));
    return hash_i(key >> 3);
}

uint64_t hash_f(double dbl) {
    union { uint64_t i; double d; } u; u.d = dbl;
    return hash_i( u.i );
}

uint64_t hash_2(const float xy[2]) {
    return hash_f(xy[0]) ^ hash_f(xy[1]);
}

uint64_t hash_3(const float xyz[3]) {
    return hash_f(xyz[0]) ^ hash_f(xyz[1]) ^ hash_f(xyz[2]);
}

uint64_t hash_4(const float xyzw[4]) {
    return hash_f(xyzw[0]) ^ hash_f(xyzw[1]) ^ hash_f(xyzw[2]) ^ hash_f(xyzw[3]);
}

uint64_t hash_s(const char* str) {
   uint64_t hash = 0; // fnv1a: 14695981039346656037ULL;
   while( *str ) {
        hash = ( *str++ ^ hash ) * 131; // fnv1a: 0x100000001b3ULL;
   }
   return hash;
}

#endif
