
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

static t_class *touchout_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _touchout {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_channel;
    } t_touchout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void touchout_float (t_touchout *x, t_float f)
{
    outmidi_afterTouch (x->x_channel, (int)f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *touchout_new (t_float channel)
{
    t_touchout *x = (t_touchout *)pd_new (touchout_class);
    
    x->x_channel = channel;
    
    inlet_newFloat (cast_object (x), &x->x_channel);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void touchout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_touchout, 
            (t_newmethod)touchout_new,
            NULL,
            sizeof (t_touchout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)touchout_float);
    
    class_setHelpName (c, sym_midiout);
    
    touchout_class = c;
}

void touchout_destroy (void)
{
    class_free (touchout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
