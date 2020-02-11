/* 
 * File:   globals_extern.h
 * Author: trbot
 *
 * Created on March 9, 2015, 1:32 PM
 */

#ifndef GLOBALS_EXTERN_H
#define	GLOBALS_EXTERN_H

// enable USE_TRACE if you want low level functionality tracing using std::cout
//#define USE_TRACE

#include <string>

#include "debugprinting.h"
#include "plaf.h"
#include <atomic>

#ifndef __rtm_force_inline
#define __rtm_force_inline __attribute__((always_inline)) inline
#endif

#ifndef DEBUG
#define DEBUG if(0)
#define DEBUG1 if(0)
#define DEBUG2 if(0)
#define DEBUG3 if(0) /* rarely used */
#endif

#ifdef __unix__
#define POSIX_SYSTEM
#else
#error NOT UNIX SYSTEM
#endif

#ifdef USE_TRACE
extern std::atomic_bool ___trace;
#define TRACE_TOGGLE {bool ___t = ___trace; ___trace = !___t;}
#define TRACE_ON {___trace = true;}
#define TRACE DEBUG if(___trace)
extern std::atomic_bool ___validateops;
#define VALIDATEOPS_ON {___validateops = true;}
#define VALIDATEOPS DEBUG if(___validateops)
#endif

/**
 * Configure global statistics using gstats_global.h and gstats.h
 */

#include "gstats_global.h"

/**
 * Setup timing code
 */

#include "server_clock.h"

/**
 * Configure record manager: reclaimer, allocator and pool
 */

#define _EVAL(a) a
#define _PASTE2(a, b) a##b
#define PASTE(a, b) _PASTE2(a, b)
#ifdef RECLAIM_TYPE
    #define RECLAIM PASTE(reclaimer_, RECLAIM_TYPE)
#else
    #define RECLAIM reclaimer_debra
#endif
#ifdef ALLOC_TYPE
    #define ALLOC PASTE(allocator_, ALLOC_TYPE)
#else
    #define ALLOC allocator_new
#endif
#ifdef POOL_TYPE
    #define POOL PASTE(pool_, POOL_TYPE)
#else
    #define POOL pool_none
#endif

#endif	/* GLOBALS_EXTERN_H */
