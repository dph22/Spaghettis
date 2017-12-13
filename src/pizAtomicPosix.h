
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_ATOMIC_POSIX_H
#define PIZ_ATOMIC_POSIX_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/_005f_005fatomic-Builtins.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef volatile int32_t    __attribute__ ((__aligned__ (4)))   PIZInt32Atomic; 
typedef volatile uint32_t   __attribute__ ((__aligned__ (4)))   PIZUInt32Atomic;
typedef volatile uint64_t   __attribute__ ((__aligned__ (8)))   PIZUInt64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_MEMORY_BARRIER                       __sync_synchronize()

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_ATOMIC_INT32_INCREMENT(q)           __atomic_add_fetch ((q), 1, __ATOMIC_RELAXED)
#define PIZ_ATOMIC_INT32_DECREMENT(q)           __atomic_sub_fetch ((q), 1, __ATOMIC_RELAXED)

#define PIZ_ATOMIC_INT32_READ(q)                pizAtomicInt32Read (q)
#define PIZ_ATOMIC_INT32_WRITE(value, q)        pizAtomicInt32Write (value, q)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_ATOMIC_UINT32_SET(mask, q)          __atomic_or_fetch ((q), (mask), __ATOMIC_RELAXED)
#define PIZ_ATOMIC_UINT32_UNSET(mask, q)        __atomic_and_fetch ((q), (~(mask)), __ATOMIC_RELAXED)

#define PIZ_ATOMIC_UINT32_READ(q)               pizAtomicUInt32Read (q)
#define PIZ_ATOMIC_UINT32_WRITE(value, q)       pizAtomicUInt32Write (value, q)

#define PIZ_ATOMIC_UINT32_TRUE(mask, q)         ((mask) & PIZ_ATOMIC_UINT32_READ (q)) 
#define PIZ_ATOMIC_UINT32_FALSE(mask, q)        !((mask) & PIZ_ATOMIC_UINT32_READ (q)) 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_ATOMIC_UINT64_READ(q)               pizAtomicUInt64Read (q)
#define PIZ_ATOMIC_UINT64_WRITE(value, q)       pizAtomicUInt64Write (value, q)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int32_t     pizAtomicInt32Read      (PIZInt32Atomic *q);
void        pizAtomicInt32Write     (int32_t n, PIZInt32Atomic *q);

uint32_t    pizAtomicUInt32Read     (PIZUInt32Atomic *q);
void        pizAtomicUInt32Write    (uint32_t n, PIZUInt32Atomic *q);

uint64_t    pizAtomicUInt64Read     (PIZUInt64Atomic *q);
void        pizAtomicUInt64Write    (uint64_t n, PIZUInt64Atomic *q);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_ATOMIC_POSIX_H
