
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "d_delay.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *delwrite_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void delwrite_tilde_updateDelayLine (t_delwrite_tilde *x, t_float sampleRate, int blockSize)
{
    int n = (int)(PD_MILLISECONDS_TO_SECONDS (x->dw_delayLineInMilliseconds) * sampleRate);
    
    n = PD_MAX (1, n);
    n += ((- n) & (DELAY_ROUND_SAMPLES - 1));       /* Snap to the next multiple of DELAY_ROUND_SAMPLES. */
    n += INTERNAL_BLOCKSIZE;
    
    x->dw_space.c_sampleRate = sampleRate;
    x->dw_space.c_vectorSize = blockSize;
    
    if (x->dw_space.c_size != n) {
    //
    size_t oldSize = sizeof (t_sample) * (x->dw_space.c_size + DELAY_EXTRA_SAMPLES);
    size_t newSize = sizeof (t_sample) * (n + DELAY_EXTRA_SAMPLES);
    
    x->dw_space.c_vector = (t_sample *)PD_MEMORY_RESIZE (x->dw_space.c_vector, oldSize, newSize);
    x->dw_space.c_size   = n;
    x->dw_space.c_phase  = DELAY_EXTRA_SAMPLES;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *delwrite_tilde_perform (t_int *w)
{
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    int phase = c->c_phase;
    
    PD_RESTRICTED p = c->c_vector + phase;
    
    phase += n;

    while (n--) {
    //
    t_sample f = *in++;
    
    if (PD_FLOAT32_IS_BIG_OR_SMALL (f)) { f = 0.0; }
    
    *p++ = f;
    
    /* Make a copy of the last four ending points at start. */
    /* Required for 4-points interpolation. */
    
    if (p == c->c_vector + (c->c_size + DELAY_EXTRA_SAMPLES)) {
    //
    t_sample f1 = *(p - 4);
    t_sample f2 = *(p - 3);
    t_sample f3 = *(p - 2);
    t_sample f4 = *(p - 1);
    
    p = c->c_vector;
    
    *p++ = f1;
    *p++ = f2;
    *p++ = f3;
    *p++ = f4;  // DELAY_EXTRA_SAMPLES
    
    phase -= c->c_size;
    //
    }
    //
    }
    
    c->c_phase = phase;
    
    return (w + 4);
}

static void delwrite_tilde_dsp (t_delwrite_tilde *x, t_signal **sp)
{
    x->dw_buildIdentifier = instance_getDspChainIdentifier();
    
    delwrite_tilde_updateDelayLine (x, sp[0]->s_sampleRate, sp[0]->s_vectorSize);
    
    dsp_add (delwrite_tilde_perform, 3, &x->dw_space, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *delwrite_tilde_new (t_symbol *s, t_float milliseconds)
{
    t_delwrite_tilde *x = (t_delwrite_tilde *)pd_new (delwrite_tilde_class);

    x->dw_delayLineInMilliseconds = milliseconds;
    x->dw_space.c_size            = 0;
    x->dw_space.c_vector          = (t_sample *)PD_MEMORY_GET ((0 + DELAY_EXTRA_SAMPLES) * sizeof (t_sample));
    x->dw_name                    = (s == &s_) ? sym_delwrite__tilde__ : s;
    
    pd_bind (cast_pd (x), x->dw_name);
    
    return x;
}

static void delwrite_tilde_free (t_delwrite_tilde *x)
{
    pd_unbind (cast_pd (x), x->dw_name);
    
    PD_MEMORY_FREE (x->dw_space.c_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void delwrite_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_delwrite__tilde__, 
            (t_newmethod)delwrite_tilde_new,
            (t_method)delwrite_tilde_free,
            sizeof (t_delwrite_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_delwrite_tilde, dw_f);
    
    class_addDSP (c, (t_method)delwrite_tilde_dsp);
    
    delwrite_tilde_class = c;
}

void delwrite_tilde_destroy (void)
{
    class_free (delwrite_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
