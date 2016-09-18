
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arraymin_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraymin {
    t_arrayrange    x_arrayrange;
    t_outlet        *x_outLeft;
    t_outlet        *x_outRight;
    } t_arraymin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void arraymin_bang (t_arraymin *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__min, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    
    t_float minIndex = -1.0;
    t_float minValue = PD_FLT_MAX;
    
    for (i = 0; i < n; i++) {
        t_float t = array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange));
        if (t < minValue) { minValue = t; minIndex = start + i; }
    }
    
    outlet_float (x->x_outRight, minIndex);
    outlet_float (x->x_outLeft, minValue);
    //
    }
}

static void arraymin_float (t_arraymin *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arraymin_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *arraymin_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraymin *x = (t_arraymin *)arrayrange_new (arraymin_class, argc, argv, 0, 1);

    if (x) {
        x->x_outLeft  = outlet_new (cast_object (x), &s_float);
        x->x_outRight = outlet_new (cast_object (x), &s_float);

    } else {
        error_invalidArguments (sym_array__space__min, argc, argv);
        pd_free (x); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraymin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__min,
            (t_newmethod)arraymin_new,
            (t_method)arrayclient_free,
            sizeof (t_arraymin),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, arraymin_bang);
    class_addFloat (c, arraymin_float);

    class_setHelpName (c, sym_array);
    
    arraymin_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
