
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Not implemented yet. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *serial_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _serial {
    t_object    x_obj;              /* Must be the first. */
    t_outlet    *x_outlet;
    } t_serial;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void serial_float (t_serial *x, t_float f)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *serial_new (t_float f)
{
    t_serial *x = (t_serial *)pd_new (serial_class);
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void serial_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_serial,
            (t_newmethod)serial_new,
            NULL,
            sizeof (t_serial),
            CLASS_DEFAULT, 
            A_DEFFLOAT,
            A_NULL);
        
    class_addFloat (c, (t_method)serial_float);
    
    serial_class = c;
}

void serial_destroy (void)
{
    CLASS_FREE (serial_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
