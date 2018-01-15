
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *moses_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _moses {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_moses;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void moses_float (t_moses *x, t_float f)
{
    if (f < x->x_f) { outlet_float (x->x_outletLeft, f); }
    else {
        outlet_float (x->x_outletRight, f);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *moses_new (t_float f)
{
    t_moses *x = (t_moses *)pd_new (moses_class);
    
    x->x_f = f;
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);

    inlet_newFloat (cast_object (x), &x->x_f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void moses_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_moses,
            (t_newmethod)moses_new,
            NULL,
            sizeof (t_moses),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
        
    class_addFloat (c, (t_method)moses_float);
    
    moses_class = c;
}

void moses_destroy (void)
{
    class_free (moses_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
