
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

/* Two-pole band-pass filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/Two_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *bp_tilde_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bp_tilde_control {
    t_sample            c_real1;
    t_sample            c_real2;
    t_sample            c_a1;
    t_sample            c_a2;
    t_sample            c_gain;
    } t_bp_tilde_control;

typedef struct _bp_tilde {
    t_object            x_obj;                  /* Must be the first. */
    t_float             x_f;
    t_float             x_sampleRate;
    t_float             x_frequency;
    t_float             x_q;
    t_bp_tilde_control  x_space;
    t_outlet            *x_outlet;
    } t_bp_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline double bp_tilde_coefficientsProceedQCosine (double f)
{
    if (f < -PD_HALF_PI || f > PD_HALF_PI) { return 0.0; }
    else {
        return cos (f);
    }
}

static void bp_tilde_coefficientsProceed (t_bp_tilde *x, t_float f, t_float q)
{
    x->x_frequency  = (f < 0.001) ? (t_float)10.0 : f;
    x->x_q          = (t_float)PD_MAX (0.0, q);
    
    {
        double omega      = x->x_frequency * PD_TWO_PI / x->x_sampleRate;
        double omegaPerQ  = PD_MIN ((x->x_q < 0.001) ? 1.0 : (omega / x->x_q), 1.0);
        double r          = 1.0 - omegaPerQ;
        
        x->x_space.c_a1   = (t_sample)(2.0 * bp_tilde_coefficientsProceedQCosine (omega) * r);
        x->x_space.c_a2   = (t_sample)(- r * r);
        x->x_space.c_gain = (t_sample)(2.0 * omegaPerQ * (omegaPerQ + r * omega));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bp_tilde_frequency (t_bp_tilde *x, t_float f)
{
    bp_tilde_coefficientsProceed (x, f, x->x_q);
}

static void bp_tilde_q (t_bp_tilde *x, t_float q)
{
    bp_tilde_coefficientsProceed (x, x->x_frequency, q);
}

static void bp_tilde_clear (t_bp_tilde *x)
{
    x->x_space.c_real1 = 0.0;
    x->x_space.c_real2 = 0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *bp_tilde_perform (t_int *w)
{
    t_bp_tilde_control *c = (t_bp_tilde_control *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);

    t_sample last1  = c->c_real1;
    t_sample last2  = c->c_real2;
    t_sample a1     = c->c_a1;
    t_sample a2     = c->c_a2;
    t_sample gain   = c->c_gain;
    
    while (n--) {
        t_sample f = (*in++) + a1 * last1 + a2 * last2;
        *out++ = gain * f; 
        last2  = last1;
        last1  = f;
    }
    
    if (PD_FLOAT32_IS_BIG_OR_SMALL (last1)) { last1 = 0.0; }
    if (PD_FLOAT32_IS_BIG_OR_SMALL (last2)) { last2 = 0.0; }
    
    c->c_real1 = last1;
    c->c_real2 = last2;
    
    return (w + 5);
}

static void bp_tilde_dsp (t_bp_tilde *x, t_signal **sp)
{
    x->x_sampleRate = sp[0]->s_sampleRate;
    
    bp_tilde_coefficientsProceed (x, x->x_frequency, x->x_q);
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (bp_tilde_perform, 4, &x->x_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *bp_tilde_new (t_float f, t_float q)
{
    t_bp_tilde *x = (t_bp_tilde *)pd_new (bp_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_new2 (x, &s_float);
    inlet_new3 (x, &s_float);
    
    bp_tilde_coefficientsProceed (x, f, q);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bp_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bp__tilde__,
            (t_newmethod)bp_tilde_new,
            NULL,
            sizeof (t_bp_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_bp_tilde, x_f);
    
    class_addDSP (c, (t_method)bp_tilde_dsp);
    
    class_addMethod (c, (t_method)bp_tilde_frequency,   sym__inlet2,    A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bp_tilde_q,           sym__inlet3,    A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bp_tilde_clear,       sym_clear,      A_NULL);
    
    bp_tilde_class = c;
}

void bp_tilde_destroy (void)
{
    class_free (bp_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------