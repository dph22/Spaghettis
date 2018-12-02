
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_MT_h_
#define __s_MT_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_32BIT

    #include "s_MT_32.h"
    typedef MTState32 t_randMT;

#endif // PD_32BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_64BIT

    #include "s_MT_64.h"
    typedef MTState64 t_randMT;

#endif // PD_64BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that integer is 32-bit or 64-bit according to the data model. */

t_randMT    *randMT_new         (void);

void        randMT_free         (t_randMT *x);
double      randMT_getDouble    (t_randMT *x);             // -- Random float on [0, 1) interval.
long        randMT_getInteger   (t_randMT *x, long v);     // -- Random integer on [0, v) interval.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_MT_h_
