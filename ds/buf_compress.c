// requires #included stdarc.c beforehand
// - rlyeh, public domain

#ifndef COMPRESS_H
#define COMPRESS_H
#include "buf_compress.h"

enum {
    NONE = RAW|0,   // no compressor
    FASTC = ULZ|0,  // fastest compressor
    FASTD = ULZ|0,  // fastest decompressor
    FAST2 = ULZ|0,  // both fastest compressor + decompressor
    BESTC = LZMA|9, // best compressor (decompression time does not matter)
    BEST2 = DEFL|6, // both best compressor + fastest decompressor
    MEMFC = ULZ|0,  // most mem-friendly compressor
    MEMFD = ULZ|0,  // most mem-friendly decompressor
    MEMF2 = ULZ|0,  // most mem-friendly (both) de+compressor
    AUTOC = 0,      // per platform or per game

    // pick 2?
    GOOD,   // good compression
    NICE,   // nice memory usage
    FAST,   // fast times
};

// mem de/encoder. return 0 if error
#define   compress_len(inlen,flags)               mem_bounds((inlen), (flags))
#define   compress_mem(in,inlen,out,outlen,flags) mem_encode((in), (inlen), (out), (outlen), (flags))
#define decompress_mem(in,inlen,out,outlen)       mem_decode((in), (inlen), (out), (outlen))

// file de/encoder
#define   compress_file(in, out, flags) (compress_list[0] = (flags), file_encode((in),(out),NULL,1,compress_list)
#define decompress_file(in, out)        file_decode((in),(out),NULL)
extern unsigned compress_list[4];

#endif // COMPRESS_H

// -----------------------------------------------------------------------------

#ifdef COMPRESS_C
#pragma once

#define STDARC_C
#define crc32 crc32_
#include "buf_compress.h"
#undef crc32

unsigned compress_list[4];

#ifdef COMPRESS_DEMO
int main() {
    const char *longcopy = "Hello world! Hello world! Hello world! Hello world!";

    char out[128];
    unsigned outlen = compress_mem(longcopy, strlen(longcopy)+1, out, 128, FAST2 );
    printf("%s %d->%d\n", outlen ? "ok" : "fail", (int)strlen(longcopy)+1, (int)outlen);

    char redo[128];
    unsigned unpacked = decompress_mem(out, outlen, redo, 128);
    printf("%d->%d %s\n", (int)outlen, (int)unpacked, redo);
}
#define main main__
#endif // COMPRESS_DEMO
#endif // COMPRESS_C
