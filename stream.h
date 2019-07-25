// stream: read/write/sync, stats, sim
// implementations: mem/list(str), file/dir, tcp/udp, stdin/stdout, sql/kissdb

#ifndef STREAM_H
#define STREAM_H

// stream is an interface to something that reads and writes bytes, like simplified files (no fseek, no freopen).
// could have used funopen() instead, but it is not available everywhere.
// ---
// [x] transports: null://, file://, mem://.
// [ ] extra transports: udp://, tcp://, http://, ftp://, ipc://, rpc://
// [x] pipes: chain of transforms to apply after reading/before writing.
// [ ] transactions: txbegin(id) + content + txend(id)
// [x] linked streams -[hub]< (multicast write)
// [ ] linked streams: >[sink]- >[multi]<) (multicast read, multicast read/write)

typedef struct STREAM STREAM;

// basic usage
//int   stream_stat(const char *spec); // check for presence
STREAM* stream_open(const char *spec, ...);
int     stream_pull(STREAM *s, void *ptr, int sz);
int     stream_push(STREAM *s, const void *ptr, int sz);
int     stream_tell(STREAM *s);
int     stream_sync(STREAM *s);
int     stream_shut(STREAM *s);
int     stream_link(STREAM *s, STREAM *other);
int     stream_pipe(STREAM *s, int(*pipe)(void *ptr, int sz) );
int     stream_puts(STREAM *s, char *t);

char   *stream_info(STREAM *s);

// create and register new stream protocol
int     stream_make(
        const char *spec,
        int (*open)(void *self, const char *spec, va_list vl),
        int (*pull)(void *self, void *ptr, int sz),
        int (*push)(void *self, const void *ptr, int sz),
        int (*tell)(void *self),
        int (*sync)(void *self),
        int (*shut)(void *self)
);

#define stream_puts(s,...) stream_puts(s,va(__VA_ARGS__))

#endif

// -----------------------------------------------------------------------------

#ifdef STREAM_C
#pragma once

// null:// interface ----------------------------------------------------------

static int nil_open(STREAM *s, const char *spec, va_list vl) { return 0; }
static int nil_read(STREAM *s, void *ptr, int sz) { return sz; }
static int nil_write(STREAM *s, const void *ptr, int sz) { return sz; }
static int nil_tell(STREAM *s) { return 0; }
static int nil_sync(STREAM *s) { return 0; }
static int nil_close(STREAM *s) { return 0; }

// file:// interface ----------------------------------------------------------

typedef struct file {
    FILE *fp;
} file;

static int file_open(STREAM *s, const char *spec, va_list vl) {
    return (((file*)s)->fp = (fopen)(spec, va_arg(vl, const char *))) ? 0 : -1;
}

static int file_read(STREAM *s, void *ptr, int sz) {
    return (fread)(ptr, 1, sz, ((file*)s)->fp);
}

static int file_write(STREAM *s, const void *ptr, int sz) {
    return (fwrite)(ptr, 1, sz, ((file*)s)->fp);
}

static int file_tell(STREAM *s) {
    return (int)(ftell)(((file*)s)->fp);
}

static int file_sync(STREAM *s) {
    return (fflush)(((file*)s)->fp), 0;
}

static int file_close(STREAM *s) {
    return (fclose)(((file*)s)->fp), 0;
}

static int file_stat( const char *filename ) {
#ifdef _WIN
    return _access( filename, 0 ) != -1 ? 0 : -1;
#else
    return access( filename, F_OK ) != -1 ? 0 : -1;
#endif
}

// mem:// interface -----------------------------------------------------------

typedef struct membuffer {
    char *buffer;
    int pointer;
    int avail;
    int owned;
} membuffer;

static int mem_open(STREAM *s, const char *spec, va_list vl) {
    char *outbuf = (char*)va_arg(vl, void *);
    char *outend = (char*)va_arg(vl, void *);

    membuffer *mb = (membuffer*)s;
    mb->owned = !outbuf;
    mb->buffer = outbuf;
    mb->pointer = 0;
    mb->avail = outend - outbuf;
    return 0;
}

static int mem_read(STREAM *s, void *ptr, int sz) {
    membuffer *mb = (membuffer*)s;
    if( mb->avail < sz ) {
        sz = mb->avail;
    }
    memcpy(ptr, mb->buffer + mb->pointer, sz);
    mb->pointer += sz;
    mb->avail -= sz;
    return sz;
}

static int mem_write(STREAM *s, const void *ptr, int sz) {
    membuffer *mb = (membuffer*)s;
    if( mb->owned && sz >= mb->avail ) { /* used to be > before adding extra zero comment below */ 
        int newlen = (mb->pointer + sz) * 1.5;
        mb->buffer = REALLOC(mb->buffer, newlen);
        mb->avail = newlen - mb->avail;
    }
    /* <-- check below removed because it does not work with streams of unknown size (mb->avail == 0)
    if( mb->avail < sz ) {
        sz = mb->avail;
    }
    */
    memcpy(mb->buffer + mb->pointer, ptr, sz);
    mb->pointer += sz;
    mb->avail -= sz;
    return sz;
}

static int mem_tell(STREAM *s) {
    membuffer *mb = (membuffer*)s;
    return (int)mb->pointer;
}

static int mem_sync(STREAM *s) {
    membuffer *mb = (membuffer*)s;
    return 0;
}

static int mem_close(STREAM *s) {
    membuffer *mb = (membuffer*)s;
    if( mb->owned ) free( mb->buffer );
    return 0;
}

// stream ---------------------------------------------------------------------

typedef struct STREAM {
    // opaque data always first member in struct.
    // implementations can cast directly to their types as long as their sizes are less than sizeof(dummy).
    char dummy[ 256 - sizeof(const char *) - sizeof(int(*)()) * (6+8) - sizeof(STREAM*) - 8*4 ];
    // members
    const char *spec;
    int (*open)(void *self, const char *spec, ...);
    int (*pull)(void *self, void *ptr, int sz);
    int (*push)(void *self, const void *ptr, int sz);
    int (*tell)(void *self);
    int (*sync)(void *self);
    int (*shut)(void *self);
    int (*pipe[8])(void *ptr, int sz);
    STREAM *next;
    uint64_t rd, wr, rdb, wrb; // rd/wr hits, rd/wr bytes
} STREAM;

typedef int sizeof_stream[ sizeof(STREAM) == 256 ];

static STREAM sprotocols[32] = {0};
static int sprotocols_count = 0;

int stream_make(
    const char *spec,
    int (*open)(void *self, const char *spec, va_list),
    int (*pull)(void *self, void *ptr, int sz),
    int (*push)(void *self, const void *ptr, int sz),
    int (*tell)(void *self),
    int (*sync)(void *self),
    int (*shut)(void *self)
) {
    STREAM protocol = { {0}, spec, open, pull, push, tell, sync, shut };
    sprotocols[sprotocols_count++] = protocol;

    // default to file:// (slot #0) if no explicit protocol is provided
    int special_case = !strcmp(protocol.spec, "file://");
    if( special_case ) {
        STREAM swapped = sprotocols[ 0 ];
        sprotocols[ 0 ] = sprotocols[ sprotocols_count - 1 ];
        sprotocols[ sprotocols_count - 1 ] = swapped;
    }

    return 0;
}

STREAM *stream_open(const char *spec, ...) {
    static int registered = 0;
    if( !registered ) { // auto-register provided interfaces
        registered = 1;
        stream_make( "file://", file_open, file_read, file_write, file_tell, file_sync, file_close );
        stream_make( "mem://", mem_open, mem_read, mem_write, mem_tell, mem_sync, mem_close );
        stream_make( "null://", nil_open, nil_read, nil_write, nil_tell, nil_sync, nil_close );
    }
    STREAM *self = (STREAM*)REALLOC(0, sizeof(STREAM));
    *self = sprotocols[0]; // default file:// interface
    if( strstr(spec, "://") ) {
        for( int i = 0; i < sprotocols_count; ++i ) {
            if( strstr(spec, sprotocols[i].spec) ) {
                *self = sprotocols[i];
                spec += strlen(sprotocols[i].spec);
                break;
            }
        }
    }

    va_list vl;
    va_start(vl, spec);
    if( 0 != self->open( self, spec, vl ) ) {
        REALLOC(self, 0);
        self = 0;
    }
    va_end(vl);

    return self;
}

int stream_pipe(STREAM *self, int(*pipe)(void*,int)) {
    int it = 0; 
    for(;it <= 8 && self->pipe[it]; ++it);
    return it == 8 ? -1 : (self->pipe[it] = pipe, 0);
}

static
int read_loop(STREAM *self, void *buffer, int count) {
    int offset = 0;
    while( count > 0 ) {
        int block = self->pull(self, (char *)buffer + offset, count);

        if( block < 0 ) {
            return block;
        }
        if( block ) {
            offset += block;
            count -= block;

            ++self->rd;
            self->rdb += block;
        }
    }
    return offset;
}

static
int write_loop(STREAM *self, const void *buffer, int count) {
    int offset = 0;
    while( count > 0 ) {
        int block = self->push(self, (const char *)buffer + offset, count);

        if( block < 0 ) {
            return block;
        }
        if( block ) {
            offset += block;
            count -= block;

            ++self->wr;
            self->wrb += block;
        }
    }
    return offset;
}

int stream_pull(STREAM *self, void *ptr, int sz) {
    int rc = 0;
    while( self ) {
        rc = read_loop(self, ptr, sz);
        for(int it=0;it<8 && rc>=0;++it) rc = self->pipe[it] ? self->pipe[it]((void*)ptr, sz) : rc;
        if( rc < 0 ) break;

        self = self->next;
    }
    return rc;
}

int stream_push(STREAM *self, const void *ptr, int sz) {
    int rc = 0;
    while( self ) {
        for(int it=0;it<8 && rc>=0;++it) rc = self->pipe[it] ? self->pipe[it]((void*)ptr, sz) : rc;
        rc = write_loop(self, ptr, sz);
        if( rc < 0 ) break;

        self = self->next;
    }
    return rc;
}

int stream_tell(STREAM *self) {
    int minimum = INT_MAX;
    while( self ) {
        STREAM *next = self->next;
        int at = self->tell(self);
        if( at < minimum ) minimum = at;
        self = next;
    }
    return minimum;
}

int stream_sync(STREAM *self) {
    while( self ) {
        STREAM *next = self->next;
        self->sync(self);
        self = next;
    }
    return 0;
}

int stream_shut(STREAM *self) {
    while( self ) {
        STREAM *next = self->next;
        self->sync(self);
        self->shut(self);
        free(self);
        self = next;
    }
    return 0;
}

int stream_link(STREAM *self, STREAM *other) {
    if( !self->next ) return !!(self->next = other);
    return stream_link(self->next, other);
}

int (stream_puts)(STREAM *self, char *text) {
    return stream_push(self, text, va_len(text));
}

char* stream_info(STREAM *self) {
    return va( "[STREAM:%p]: %llu pulled bytes (%llu hits), %llu pushed bytes (%llu hits)", self, self->rdb, self->rd, self->wrb, self->wr);
}

#endif
