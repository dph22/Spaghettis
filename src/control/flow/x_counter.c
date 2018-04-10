
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *counter_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _counter {
    t_object    x_obj;                  /* Must be the first. */
    int         x_count;
    int         x_maximum;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_counter;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void counter_output (t_counter *x)
{
    if (x->x_count <= x->x_maximum) { outlet_float (x->x_outletLeft, (t_float)x->x_count); }
    else {
        if (x->x_outletRight) { outlet_bang (x->x_outletRight); }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void counter_reset (t_counter *x)
{
    x->x_count = 0;
}

static void counter_set (t_counter *x, t_float f)
{
    x->x_count = (int)f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void counter_bang (t_counter *x)
{
    x->x_count++; counter_output (x);
}

static void counter_float (t_counter *x, t_float f)
{
    counter_set (x, f); counter_output (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *counter_new (t_symbol *s, int argc, t_atom *argv)
{
    t_counter *x = (t_counter *)pd_new (counter_class);
    
    x->x_maximum     = PD_INT_MAX;
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    
    if (argc) {
        x->x_outletRight = outlet_newBang (cast_object (x));
        x->x_maximum     = atom_getFloat (argv);
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void counter_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_counter,
            (t_newmethod)counter_new,
            NULL,
            sizeof (t_counter),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)counter_bang);
    class_addFloat (c, (t_method)counter_float);
    
    class_addMethod (c, (t_method)counter_reset,    sym_reset,  A_NULL);
    class_addMethod (c, (t_method)counter_set,      sym_set,    A_DEFFLOAT, A_NULL);
    
    counter_class = c;
}

void counter_destroy (void)
{
    class_free (counter_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------