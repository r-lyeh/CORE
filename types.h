// types: vec2, vec3, vec4, half, ubyte, vbyte, zig, quarks
// buffer: interleave/quant/diff/delta/chunk/compress/packet/crypt/escape, serial/(un)pack (<->string)
// string: utils

#ifndef TYPES_H
#define TYPES_H

// types

typedef struct byte2 { uint8_t x,y; } byte2;
typedef struct byte3 { uint8_t x,y,z; } byte3;
typedef struct byte4 { uint8_t x,y,z,w; } byte4;

typedef struct half2 { uint16_t x,y; } half2;
typedef struct half3 { uint16_t x,y,z; } half3;
typedef struct half4 { uint16_t x,y,z,w; } half4;

typedef struct int2 { int x,y; } int2;
typedef struct int3 { int x,y,z; } int3;
typedef struct int4 { int x,y,z,w; } int4;

typedef struct uint2 { unsigned int x,y; } uint2;
typedef struct uint3 { unsigned int x,y,z; } uint3;
typedef struct uint4 { unsigned int x,y,z,w; } uint4;

typedef struct float2 { float x,y; } float2;
typedef struct float3 { float x,y,z; } float3;
typedef struct float4 { float x,y,z,w; } float4;

typedef struct double2 { double x,y; } double2;
typedef struct double3 { double x,y,z; } double3;
typedef struct double4 { double x,y,z,w; } double4;

#define byte2(x,y)         CAST(byte2,   (uint8_t)(x), (uint8_t)(y) )
#define byte3(x,y,z)       CAST(byte3,   (uint8_t)(x), (uint8_t)(y), (uint8_t)(z) )
#define byte4(x,y,z,w)     CAST(byte4,   (uint8_t)(x), (uint8_t)(y), (uint8_t)(z), (uint8_t)(w) )

#define half2(x,y)         CAST(half2,   (uint16_t)(x), (uint16_t)(y) )
#define half3(x,y,z)       CAST(half3,   (uint16_t)(x), (uint16_t)(y), (uint16_t)(z) )
#define half4(x,y,z,w)     CAST(half4,   (uint16_t)(x), (uint16_t)(y), (uint16_t)(z), (uint16_t)(w) )

#define int2(x,y)          CAST(int2,    (int)(x), (int)(y) )
#define int3(x,y,z)        CAST(int3,    (int)(x), (int)(y), (int)(z) )
#define int4(x,y,z,w)      CAST(int4,    (int)(x), (int)(y), (int)(z), (int)(w) )

#define uint2(x,y)         CAST(uint2,   (unsigned)(x), (unsigned)(y) )
#define uint3(x,y,z)       CAST(uint3,   (unsigned)(x), (unsigned)(y), (unsigned)(z) )
#define uint4(x,y,z,w)     CAST(uint4,   (unsigned)(x), (unsigned)(y), (unsigned)(z), (unsigned)(w) )

#define float2(x,y)        CAST(float2,  (float)(x), (float)(y) )
#define float3(x,y,z)      CAST(float3,  (float)(x), (float)(y), (float)(z) )
#define float4(x,y,z,w)    CAST(float4,  (float)(x), (float)(y), (float)(z), (float)(w) )

#define double2(x,y)       CAST(double2, (double)(x), (double)(y) )
#define double3(x,y,z)     CAST(double3, (double)(x), (double)(y), (double)(z) )
#define double4(x,y,z,w)   CAST(double4, (double)(x), (double)(y), (double)(z), (double)(w) )

#define ptr(type)          0[&(type).x]

#ifdef __cplusplus
#define CAST(type, ...)    ( type { __VA_ARGS__ } )
#else
#define CAST(type, ...)    ((type){ __VA_ARGS__ } )
#endif

// quarks ----------------------------------------------------------------------

typedef unsigned quark;

quark quark_intern(char *s);
char* quark_str(quark q);
int   quark_len(quark q);
int   quark_hash(quark q);
int   quark_cmp(quark a, quark b);

#define quark_intern(__VA_ARGS__) quark_intern(va(__VA_ARGS__))

// c string utils (beware of destructive operations) ---------------------------

char*           strcpyf(char**s, const char *fmt, ...);
char*           strcatf(char**s, const char *fmt, ...);
char*           strlower(char *s);
char*           strupper(char *s);
char*           strtrim(char *s);
char*           strrepl(char *s, const char *src, const char *dst);
int             strmatch(const char *s, const char *wildcard);
array(char*)    strsplit(char *s, const char *delimiters);
array(uint32_t) strutf8(const char *utf8);

#endif

// -----------------------------------------------------------------------------

#ifdef TYPES_C
#pragma once

// quarks ---------------------------------------------------------------------

typedef struct quark_dictionary {
    char *block;
    uint64_t block_used;
    array(char*) strings;
    unsigned index;
} quark_dictionary;

static __thread quark_dictionary qd = {0};

quark (quark_intern)(char *s) {
    if( !qd.block ) qd.block = REALLOC(0, 4 * 1024 * 1024 ); // 
    uint64_t n = strlen(s) + 1;
    array_push(qd.strings, qd.block + qd.block_used + 4);
    memcpy(qd.block + qd.block_used, s - 4, n + 4);
    qd.block_used += n + 4;
    return ++qd.index;
}
char *quark_str(quark q) {
    return q > 0 && q <= qd.index ? qd.strings[ q - 1 ] : "";
}
int quark_len(quark q) {
    return va_len(quark_str(q));
}
int quark_hash(quark q) {
    return va_hash(quark_str(q));
}
int quark_cmp(quark q1, quark q2) {
    return va_cmp(quark_str(q1), quark_str(q2));
}

// c string -------------------------------------------------------------------

#define TEMP
#define HEAP
#define STRDUP strdup

char *strlower( char *copy ) {
    for( char *s = copy; *s; ++s ) *s = tolower(*s); // &= ~32
    return copy;
}
char *strupper( char *copy ) {
    for( char *s = copy; *s; ++s ) *s = toupper(*s); // |= 32
    return copy;
}
char* strtrim(char *str) {
    // trims leading, trailing and excess whitespaces.
    char *ibuf, *obuf;
    if( str ) {
        for( ibuf = obuf = str; *ibuf; ) {
            while( *ibuf && isspace(*ibuf)  )  (ibuf++);
            if(    *ibuf && obuf != str     ) *(obuf++) = ' ';
            while( *ibuf && !isspace(*ibuf) ) *(obuf++) = *(ibuf++);
        }
        *obuf = 0;
    }
    return str;
}
char *strrepl( char *copy, const char *target, const char *replacement ) {
    // replaced only if new text is shorter than old one
    int rlen = strlen(replacement), diff = strlen(target) - rlen;
    if( diff >= 0 ) {
        for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
            if( rlen ) s = (char*)memcpy( s, replacement, rlen ) + rlen;
            if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
        }
    }
    return copy;
}
int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard=='\0' ) return !*s;
    if( *wildcard=='*' )  return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard=='?' )  return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
array(char*) strsplit(char *text, const char *delimiters) {
    array(char *) out = 0;
#if 0
    for( char *token = strtok(text, delimiters); token; token = strtok(NULL, delimiters) ) {
        array_push(out, token);
    }
#else
    int found[256] = {1,0}, i = 0;
    while( *delimiters ) found[(unsigned char)*delimiters++] = 1;
    while( text[i] ) {
        int begin = i; while(text[i] && !found[(unsigned char)text[i]]) ++i;
        int end = i;   while(text[i] &&  found[(unsigned char)text[i]]) ++i;
        if (end > begin) {
            array_push(out, (text + begin));
            (text + begin)[ end - begin ] = 0;
        }
    }
#endif
    return out;
}
array(uint32_t) strutf8( const char *utf8 ) {
    array(uint32_t) out = 0; //array_reserve(out, strlen(utf8) + 1);
    while( *utf8 ) {
        const char **p = &utf8;
        uint32_t unicode = 0;
        /**/ if( (**p & 0x80) == 0x00 ) {
            int a = *((*p)++);
            unicode = a;
        }
        else if( (**p & 0xe0) == 0xc0 ) {
            int a = *((*p)++) & 0x1f;
            int b = *((*p)++) & 0x3f;
            unicode = (a << 6) | b;
        }
        else if( (**p & 0xf0) == 0xe0 ) {
            int a = *((*p)++) & 0x0f;
            int b = *((*p)++) & 0x3f;
            int c = *((*p)++) & 0x3f;
            unicode = (a << 12) | (b << 6) | c;
        }
        else if( (**p & 0xf8) == 0xf0 ) {
            int a = *((*p)++) & 0x07;
            int b = *((*p)++) & 0x3f;
            int c = *((*p)++) & 0x3f;
            int d = *((*p)++) & 0x3f;
            unicode = (a << 18) | (b << 12) | (c << 8) | d;
        }
        array_push(out, unicode);
    }
    return out;
}

TEMP char *strfv( const char *fmt, va_list lst ) {
    // Wrap vsnprintf into a statically allocated buffer. A bunch of buffers are
    // handled internally so that nested calls are safe within reasonable limits.
    static __thread int vl_index = 0;
    static __thread char *vl_buffer[16] = {0};
    int idx = (++vl_index) % 16;
    int sz = 1 + vsnprintf(0, 0, fmt, lst);
    vl_buffer[idx] = (char *)REALLOC( vl_buffer[idx], sz );
    vsnprintf( vl_buffer[idx], sz, fmt, lst );
    return vl_buffer[idx];
}
TEMP char *strf( const char *fmt, ... ) {
    va_list lst;
    va_start(lst, fmt);
    char *rc = strfv(fmt, lst);
    va_end(lst);
    return rc;
}
HEAP char *strcpyfv( char **str, const char *fmt, va_list lst ) {
    TEMP char *buf = strfv( fmt, lst );
    if( str && *str ) {
        int len = strlen(buf) + 1;
        (*str) = (char*)REALLOC( str && (*str) ? (*str) : 0, len );
        memcpy( (*str), buf, len );
        return *str;
    } else {
        return str ? *str = STRDUP(buf) : STRDUP(buf);
    }
}
HEAP char *strcpyf( char **str, const char *fmt, ... ) {
    va_list lst;
    va_start(lst, fmt);
    char *rc = strcpyfv(str, fmt, lst);
    va_end(lst);
    return rc;
}
HEAP char *strcatfv( char **str, const char *fmt, va_list lst ) {
    TEMP char *buf = strfv( fmt, lst );
    if( str && *str ) {
        int l1 = strlen(*str), l2 = strlen(buf);
        (*str) = (char*)REALLOC( (*str), l1 + l2 + 1 );
        memcpy( (*str) + l1, buf, l2 + 1 );
        return *str;
    } else {
        return str ? *str = STRDUP(buf) : STRDUP(buf);
    }
}
HEAP char *strcatf( char **str, const char *fmt, ... ) {
    va_list lst;
    va_start(lst, fmt);
    HEAP char *rc = strcatfv(str, fmt, lst);
    va_end(lst);
    return rc;
}

#endif
