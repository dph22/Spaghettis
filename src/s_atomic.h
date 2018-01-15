
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_atomic_h_
#define __s_atomic_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Assume than 32-bit integer load and store are atomic. */

/* < http://preshing.com/20130618/atomic-vs-non-atomic-operations/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_32BIT
    #define PD_LOAD_STORE_32_IS_ATOMIC      1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_64BIT
    #define PD_LOAD_STORE_32_IS_ATOMIC      1
    #define PD_LOAD_STORE_64_IS_ATOMIC      1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Alignment may not work on stack (don't use local atomic variables). */

/* < http://gcc.gnu.org/bugzilla/show_bug.cgi?id=16660 > */
/* < http://stackoverflow.com/questions/841433/gcc-attribute-alignedx-explanation > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- t_int32Atomic;
// -- t_uint32Atomic;
// -- t_uint64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- PD_ATOMIC_INT32_INCREMENT             Relaxed increment and fetch.
// -- PD_ATOMIC_INT32_DECREMENT(q)          Relaxed decrement and fetch.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- PD_ATOMIC_INT32_READ(q)               Relaxed load.
// -- PD_ATOMIC_UINT32_READ(q)
// -- PD_ATOMIC_UINT64_READ(q)

// -- PD_ATOMIC_INT32_WRITE(value, q)       Relaxed store.
// -- PD_ATOMIC_UINT32_WRITE
// -- PD_ATOMIC_UINT64_WRITE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- PD_ATOMIC_UINT32_SET(mask, q)         Relaxed bitmask set and fetch.
// -- PD_ATOMIC_UINT32_UNSET(mask, q)       Relaxed bitmask unset and fetch.

// -- PD_ATOMIC_UINT32_TRUE(mask, q)        Relaxed bitmask test.
// -- PD_ATOMIC_UINT32_FALSE(mask, q)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- PD_MEMORY_BARRIER                     A full memory barrier.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_MAC_ATOMIC
    #include "s_atomicMac.h"
#endif

#if PD_POSIX_ATOMIC
    #include "s_atomicPosix.h"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_atomic_h_
