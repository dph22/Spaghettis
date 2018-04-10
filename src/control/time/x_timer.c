
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *timer_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _timer {
    t_object    x_obj;              /* Must be the first. */
    t_systime   x_start;
    t_float     x_unit;
    int         x_isSamples;        /* Samples or milliseconds. */
    t_outlet    *x_outlet;
    } t_timer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error clock_parseUnit (t_float,  t_symbol *, t_float *, int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void timer_bang (t_timer *x)
{
    x->x_start = scheduler_getLogicalTime();
}

static void timer_bangElapsed (t_timer *x)
{
    outlet_float (x->x_outlet, (t_float)scheduler_getUnitsSince (x->x_start, x->x_unit, x->x_isSamples));
}

/* Note that float arguments are always passed at last. */

static void timer_unit (t_timer *x, t_symbol *unitName, t_float f)
{
    t_error err = clock_parseUnit (f, unitName, &x->x_unit, &x->x_isSamples);
    
    if (err) {
        error_invalid (sym_timer, sym_unit); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that float arguments are always passed at last. */

static void *timer_new (t_symbol *unitName, t_float unit)
{
    t_timer *x = (t_timer *)pd_new (timer_class);
    
    x->x_unit      = 1;
    x->x_isSamples = 0;
    
    timer_bang (x);
        
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    inlet_new2 (x, &s_bang);
    
    if (unit != 0.0 && unitName != &s_) { timer_unit (x, unitName, unit); }
        
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void timer_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_timer,
            (t_newmethod)timer_new,
            NULL,
            sizeof (t_timer),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addBang (c, (t_method)timer_bang);
    
    class_addMethod (c, (t_method)timer_bangElapsed,    sym__inlet2,    A_NULL);
    class_addMethod (c, (t_method)timer_unit,           sym_unit,       A_FLOAT, A_SYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)timer_unit,           sym_tempo,      A_FLOAT, A_SYMBOL, A_NULL);
        
    #endif
    
    timer_class = c;
}

void timer_destroy (void)
{
    class_free (timer_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------