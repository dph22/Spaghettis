
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pgmout_class;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pgmout {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_channel;
    } t_pgmout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void pgmout_float (t_pgmout *x, t_float f)
{
    outmidi_programChange (x->x_channel, (int)f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *pgmout_new (t_float channel)
{
    t_pgmout *x = (t_pgmout *)pd_new (pgmout_class);
    
    x->x_channel = channel;
    
    inlet_newFloat (cast_object (x), &x->x_channel);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pgmout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pgmout,
            (t_newmethod)pgmout_new,
            NULL,
            sizeof (t_pgmout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)pgmout_float);
    
    class_setHelpName (c, sym_midiout);
    
    pgmout_class = c;
}

void pgmout_destroy (void)
{
    class_free (pgmout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
