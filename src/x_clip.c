
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *clip_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _clip {
    t_object    x_obj;
    t_float     x_f1;
    t_float     x_f2;
    t_float     x_f3;
    t_outlet    *x_outlet;
    } t_clip;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void clip_bang (t_clip *x)
{
    t_float a = PD_MIN (x->x_f2, x->x_f3);
    t_float b = PD_MAX (x->x_f2, x->x_f3);
    
    outlet_float (x->x_outlet, PD_CLAMP (x->x_f1, a, b));
}

static void clip_float (t_clip *x, t_float f)
{
    x->x_f1 = f; clip_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *clip_new (t_float f2, t_float f3)
{
    t_clip *x = (t_clip *)pd_new (clip_class);
    
    x->x_f2     = f2;
    x->x_f3     = f3;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->x_f2);
    inlet_newFloat (cast_object (x), &x->x_f3);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clip_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_clip,
            (t_newmethod)clip_new,
            NULL,
            sizeof (t_clip),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addBang (c, (t_method)clip_bang);
    class_addFloat (c, (t_method)clip_float); 
    
    clip_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
