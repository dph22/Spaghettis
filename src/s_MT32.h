
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Mersenne Twister PRNG. */

/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf > */
/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_MT32_h_
#define __s_MT32_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _MTState32 {
    uint32_t    mt_[624];
    int         mti_;
    } MTState32;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

MTState32   *genrand32_new          (void);
MTState32   *genrand32_newByArray   (long argc, uint32_t *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        genrand32_free  (MTState32 *x);
uint32_t    genrand32_int32 (MTState32 *x);  // -- Random number on [0, 0xffffffff] interval. 
double      genrand32_real2 (MTState32 *x);  // -- Random number on [0, 1) interval.
double      genrand32_res53 (MTState32 *x);  // -- Random number on [0, 1) interval with 53-bit resolution.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_MT32_h_
