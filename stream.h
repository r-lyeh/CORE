// stream: read/write/sync, muxed data, stats/sim
// implementations: mem/list(str), file/dir, tcp/udp, stdin/stdout, sql/kissdb

/*
#pragma once
#define STREAM FILE
#define sopen  fopen
#define sclose fclose
#define swrite fwrite
#define sread  fread
#define sflush fflush

bool exists( const char *filename ) {
#ifdef _WIN32
    return _access( filename, 0 ) != -1;
#else
    return access( filename, F_OK ) != -1;
#endif
}
*/
