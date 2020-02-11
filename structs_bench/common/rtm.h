#ifndef _RTM_H
#define _RTM_H 1

//#include <immintrin.h>

/*
 * Copyright (c) 2012,2013 Intel Corporation
 * Author: Andi Kleen
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Official RTM intrinsics interface matching gcc/icc, but works
   on older gcc compatible compilers and binutils. */

#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
//#define _XABORT_CODE(x)		(((x) >> 24) & 0xff)

#ifndef __rtm_force_inline
#define __rtm_force_inline __attribute__((__always_inline__)) inline
#endif

#ifndef SOFTWARE_BARRIER
#   define SOFTWARE_BARRIER asm volatile("": : :"memory")
#endif

#include <iostream>

static __rtm_force_inline int XBEGIN(void)
{
    int ret = _XBEGIN_STARTED;
    asm volatile(".byte 0xc7,0xf8 ; .long 0" : "+a" (ret) ::"memory");
    return ret;
}

static __rtm_force_inline void XEND(void) {
    asm volatile(".byte 0x0f,0x01,0xd5" :: : "memory");
}

static __rtm_force_inline void XABORT(const unsigned int status) {
    asm volatile(".byte 0xc6,0xf8,%P0" ::"i" (status) : "memory");
}

static __rtm_force_inline int XTEST(void) {
    unsigned char out;
    asm volatile(".byte 0x0f,0x01,0xd6 ; setnz %0" : "=r" (out) ::"memory");
    return out;
}
#endif
