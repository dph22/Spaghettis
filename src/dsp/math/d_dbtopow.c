
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

static t_class *dbtopow_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct dbtopow_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_dbtopow_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int *dbtopow_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = math_decibelToPower (*in++); }
    
    return (w + 4);
}

static void dbtopow_tilde_dsp (t_dbtopow_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (dbtopow_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dbtopow_tilde_new (void)
{
    t_dbtopow_tilde *x = (t_dbtopow_tilde *)pd_new (dbtopow_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dbtopow_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_dbtopow__tilde__,
            (t_newmethod)dbtopow_tilde_new,
            NULL,
            sizeof (t_dbtopow_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_dbtopow_tilde, x_f);
    
    class_addDSP (c, (t_method)dbtopow_tilde_dsp);
    
    class_setHelpName (c, sym_acoustic__tilde__);
    
    dbtopow_tilde_class = c;
}

void dbtopow_tilde_destroy (void)
{
    class_free (dbtopow_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------