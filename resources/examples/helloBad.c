
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Manage failure at creation time. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _hello {
    t_object x_obj;                         /* MUST be the first. */
    } t_hello;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *hello_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *hello_new (void)
{
    t_hello *x = (t_hello *)pd_new (hello_class);

    t_error err = PD_ERROR_NONE;
    
    err |= PD_ERROR;    /* Something wrong happens. */
    
    if (err) { 
        pd_free ((t_pd *)x);                /* It is safe to call the free method. */
        x = NULL;
    }
    
    return x;
}

static void hello_free (t_hello *x)
{
    post ("Clean your room, now!");         /* Clean already allocated things here. */
    
    /* Take care about fully vs partially constructed instance. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_STUB void helloBad_setup (t_symbol *s)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("helloBad"),
            (t_newmethod)hello_new,
            (t_method)hello_free,
            sizeof (t_hello),
            CLASS_BOX | CLASS_NOINLET,
            A_NULL); 
        
    hello_class = c;
}

PD_STUB void helloBad_destroy (void)
{
    class_free (hello_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
