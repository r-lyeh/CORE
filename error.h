// error: code, text
// logger: info, warn, fail, exit
// assert: that works in release builds.

#ifndef ERROR_H
#define ERROR_H

// ERROR API
#define ERRORSET(rc, ...)   (ERRORTEXT = !(rc) ? "No error 0":"Error -- " ERROR_STRINGIZE(rc) ": " __VA_ARGS__)
#define ERRORCODE           (atoi(ERRORTEXT+9))

// ERROR IMPL
#define ERROR_STRINGIZE(rc)  ERROR_STRINGIZ3(rc)
#define ERROR_STRINGIZ3(rc)  #rc
extern __thread char *ERRORTEXT;

#endif

#ifdef ERROR_C
#pragma once
__thread char* ERRORTEXT = "No error 0";
#endif

// ----------------------------------------------------------------------------

#ifndef LOGGER_H
#define LOGGER_H

#ifndef LOGLEVEL
#   define LOGLEVEL 4 // 0(off).. 4(max)
#endif

#ifdef SHIPPING
#   undef  LOGLEVEL
#   define LOGLEVEL 0
#endif

#if LOGLEVEL <= 0
#   define LOG(tags, fmt, ...) (void)0
#else
#   define LOG(tags, fmt, ...) (fprintf(stderr,fmt,__VA_ARGS__), fprintf(stderr," (%s:%d) %s\n",__FILE__,__LINE__,#tags))
#endif

#if LOGLEVEL >= 1
#   define LOGEXIT(tags, ...)  do { LOG(tags, __VA_ARGS__); exit(-__LINE__); } while(0)
#else
#   define LOGEXIT(tags, ...)  exit(-__LINE__)
#endif

#if LOGLEVEL >= 2
#   define LOGFAIL LOG
#else
#   define LOGFAIL(tags, ...)  (void)0
#endif

#if LOGLEVEL >= 3
#   define LOGWARN LOG
#else
#   define LOGWARN(tags, ...)  (void)0
#endif

#if LOGLEVEL >= 4
#   define LOGINFO LOG
#else
#   define LOGINFO(tags, ...)  (void)0
#endif

// EXTRA
#define TODO(...)  do { static int todo  = 0; if(!todo ) { ++todo ; LOGINFO(TODO, __VA_ARGS__); } } while(0)
#define FIXME(...) do { static int fixme = 0; if(!fixme) { ++fixme; LOGINFO(FIXME, __VA_ARGS__); } } while(0)

#endif

// ----------------------------------------------------------------------------

#ifndef ASSERT_H
#define ASSERT_H <assert.h>
#endif

#ifndef NDEBUG
#   include ASSERT_H
#else
#   undef NDEBUG
#   include ASSERT_H
#   define NDEBUG
#endif

#ifdef SHIPPING
#   undef  assert
#   define assert(expr) (void)0
#endif
