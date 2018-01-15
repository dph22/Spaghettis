
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *ctlout_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _ctlout {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_control;
    t_float     x_channel;
    } t_ctlout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void ctlout_float (t_ctlout *x, t_float f)
{
    outmidi_controlChange (x->x_channel, (int)x->x_control, (int)f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *ctlout_new (t_float control, t_float channel)
{
    t_ctlout *x = (t_ctlout *)pd_new (ctlout_class);
    
    x->x_control = control;
    x->x_channel = channel;
    
    inlet_newFloat (cast_object (x), &x->x_control);
    inlet_newFloat (cast_object (x), &x->x_channel);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void ctlout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_ctlout,
            (t_newmethod)ctlout_new,
            NULL,
            sizeof (t_ctlout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)ctlout_float);
    
    ctlout_class = c;
}

void ctlout_destroy (void)
{
    class_free (ctlout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
