
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *notein_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _notein {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_channel;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletMiddle;
    t_outlet    *x_outletRight;
    } t_notein;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void notein_list (t_notein *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float pitch    = atom_getFloatAtIndex (0, argc, argv);
    t_float velocity = atom_getFloatAtIndex (1, argc, argv);
    t_float channel  = atom_getFloatAtIndex (2, argc, argv);
    
    if (x->x_channel) {
        if (x->x_channel == channel) { 
            outlet_float (x->x_outletMiddle, velocity);
            outlet_float (x->x_outletLeft, pitch);
        }
        
    } else {
        outlet_float (x->x_outletRight, channel);
        outlet_float (x->x_outletMiddle, velocity);
        outlet_float (x->x_outletLeft, pitch);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *notein_new (t_float f)
{
    t_notein *x = (t_notein *)pd_new (notein_class);
    
    x->x_channel      = PD_ABS (f);
    x->x_outletLeft   = outlet_new (cast_object (x), &s_float);
    x->x_outletMiddle = outlet_new (cast_object (x), &s_float);
    
    if (x->x_channel == 0.0) { x->x_outletRight = outlet_new (cast_object (x), &s_float); }
    
    pd_bind (cast_pd (x), sym__notein);
    
    return x;
}

static void notein_free (t_notein *x)
{
    pd_unbind (cast_pd (x), sym__notein);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void notein_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_notein, 
            (t_newmethod)notein_new,
            (t_method)notein_free,
            sizeof (t_notein),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFFLOAT,
            A_NULL);
            
    class_addList (c, (t_method)notein_list);
    
    class_setHelpName (c, sym_midiout);
    
    notein_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
