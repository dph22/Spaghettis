
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

#include "d_math.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rsqrt_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _unop_tilde t_rsqrt_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WITH_DEADCODE

t_sample rsqrt_tableMantissa[RSQRT_MANTISSA_SIZE];           /* Static. */
t_sample rsqrt_tableExponential[RSQRT_EXPONENTIAL_SIZE];     /* Static. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void rsqrt_tilde_initialize (void)
{
    #if PD_WITH_DEADCODE
    
    int i;
    
    for (i = 0; i < RSQRT_EXPONENTIAL_SIZE; i++) {

        t_rawcast32 z;
        
        if (i == 0) { z.z_i = 1 << 23; }
        else {
            z.z_i = (i == RSQRT_EXPONENTIAL_SIZE - 1 ? RSQRT_EXPONENTIAL_SIZE - 2 : i) << 23;
        }

        rsqrt_tableExponential[i] = (t_sample)(1.0 / sqrt (z.z_f));
    }
    
    for (i = 0; i < RSQRT_MANTISSA_SIZE; i++) {
    
        /* Exponent is zero with a IEEE float inside the 1.0 to 2.0 range. */
        
        t_sample f = (t_sample)(1.0 + (1.0 / RSQRT_MANTISSA_SIZE) * i);
        
        rsqrt_tableMantissa[i] = (t_sample)(1.0 / sqrt (f));
    }
    
    #endif // PD_WITH_DEADCODE
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void rsqrt_tilde_dsp (t_rsqrt_tilde *x, t_signal **sp)
{
    dsp_addInverseSquareRootPerform (sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *rsqrt_tilde_new (void)
{
    t_rsqrt_tilde *x = (t_rsqrt_tilde *)pd_new (rsqrt_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void rsqrt_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rsqrt__tilde__,
            (t_newmethod)rsqrt_tilde_new,
            NULL,
            sizeof (t_rsqrt_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rsqrt_tilde, x_f);
    
    class_addDSP (c, (t_method)rsqrt_tilde_dsp);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)rsqrt_tilde_new, sym_q8_rsqrt__tilde__, A_NULL);
    
    #endif
    
    class_addMethod (c, (t_method)unop_tilde_signals, sym__signals, A_FLOAT, A_NULL);
    
    class_setDataFunction (c, unop_tilde_functionData);
    class_setHelpName (c, sym_math__tilde__);
    
    rsqrt_tilde_class = c;
}

void rsqrt_tilde_destroy (void)
{
    class_free (rsqrt_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
