
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"
#include "d_fftOOURA.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int      *ooura_ip;     /* Static. */
double   *ooura_w;      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void ooura_release (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Next squarable power of two. */

static int ooura_getNextSize (int n)
{
    int i = 1; while (i && i < n) { i = i << 2; } return i;
}

/* Initialize the tables with a dummy fft. */

static void ooura_dummy (int n)
{
    double *t = (double *)PD_MEMORY_GET (n * sizeof (double));
    
    int i; for (i = 0; i < n; i++) { t[i] = 0.0; }
        
    rdft (n, 1, t, ooura_ip, ooura_w);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void ooura_initialize (int n)
{
    static int ooura_maximum = 0;   /* Static. */
    
    n = ooura_getNextSize (n);
    
    PD_ASSERT (n != 0);
    
    if (n > ooura_maximum) {
    //
    int t    = (int)sqrt (n);
    size_t a = 2 + t;
    size_t b = n * 2;
    
    PD_ASSERT (PD_IS_POWER_2 (n));
    PD_ASSERT (sqrt (n) == (double)t);
    
    ooura_release();
    
    ooura_ip = (int *)PD_MEMORY_GET (a * sizeof (int));
    ooura_w  = (double *)PD_MEMORY_GET (b * sizeof (double));
    
    ooura_maximum = n;
    
    PD_ASSERT (ooura_ip[0] == 0);
    
    ooura_dummy (n);
    //
    }
}

static void ooura_release (void)
{
    if (ooura_w)  { PD_MEMORY_FREE (ooura_w); }
    if (ooura_ip) { PD_MEMORY_FREE (ooura_ip); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fft_stateRelease (t_FFTState *x)
{
    if (x->ooura_cache) { PD_MEMORY_FREE (x->ooura_cache); }
}

void fft_stateInitialize (t_FFTState *x, int n)
{
    fft_stateRelease (x); x->ooura_cache = (double *)PD_MEMORY_GET (n * 2 * sizeof (double));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fft_initialize (void)
{
    ooura_initialize (AUDIO_DEFAULT_BLOCKSIZE);
}

void fft_release (void)
{
    ooura_release();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
