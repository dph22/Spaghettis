
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "d_filters.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Real one-pole filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rpole_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _real_raw_tilde t_rpole_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void rpole_tilde_clear (t_rpole_tilde *x)
{
    x->x_real = 0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static t_int *rpole_tilde_perform (t_int *w)
{
    t_rpole_tilde *x  = (t_rpole_tilde *)(w[1]);
    PD_RESTRICTED in1 = (t_sample *)(w[2]);
    PD_RESTRICTED in2 = (t_sample *)(w[3]);
    PD_RESTRICTED out = (t_sample *)(w[4]);
    int n = (int)(w[5]);
    
    t_sample last = x->x_real;
    
    while (n--) {
        t_sample a = (*in2++);
        t_sample f = (*in1++) + a * last;
        *out++ = last = f;
    }
    
    if (PD_FLOAT32_IS_BIG_OR_SMALL (last)) { last = 0.0; }
        
    x->x_real = last;
    
    return (w + 6);
}

static void rpole_tilde_dsp (t_rpole_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    dsp_add (rpole_tilde_perform, 5, x,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector,
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *real_raw_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    struct _real_raw_tilde *x = (struct _real_raw_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendFloat (b,  (t_float)x->x_real);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__signals);
    object_getSignalValues (cast_object (x), b, 2);
    
    return b;
    //
    }
    
    return NULL;
}

void real_raw_restore (struct _real_raw_tilde *x, t_float f)
{
    x->x_real = (t_sample)f;
}

void real_raw_signals (struct _real_raw_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    object_setSignalValues (cast_object (x), argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *rpole_tilde_new (t_float f)
{
    t_rpole_tilde *x = (t_rpole_tilde *)pd_new (rpole_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void rpole_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rpole__tilde__,
            (t_newmethod)rpole_tilde_new,
            NULL,
            sizeof (t_rpole_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rpole_tilde, x_f);
    
    class_addDSP (c, (t_method)rpole_tilde_dsp);
        
    class_addMethod (c, (t_method)rpole_tilde_clear,    sym_clear,      A_NULL);
    class_addMethod (c, (t_method)real_raw_restore,     sym__restore,   A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)real_raw_signals,     sym__signals,   A_GIMME, A_NULL);
    
    class_setDataFunction (c, real_raw_functionData);
    
    rpole_tilde_class = c;
}

void rpole_tilde_destroy (void)
{
    class_free (rpole_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
