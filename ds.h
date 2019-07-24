// data structures: array, map(set), lfqueue, hash, sort, comp,
// async: thread_local (ctor/dtor), thread, mutex, channel, coroutines
// time: sleep/yield, clock, timer(tmr,fn,time)/cancel(tmr)/every(tmr,fn,time)

#pragma once

#if defined _MSC_VER && !defined __thread
#define __thread __declspec(thread)
#endif
