
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listinlet_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void listinlet_list (t_listinlet *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listinlet_cachePointer (t_listinlet *x, int i, t_atom *a)
{
    if (IS_POINTER (a)) {
        gpointer_setByCopy (&x->li_vector[i].le_gpointer, GET_POINTER (a));
        SET_POINTER (&x->li_vector[i].le_atom, &x->li_vector[i].le_gpointer);
        x->li_hasPointer = 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_init (t_listinlet *x)
{
    x->li_pd          = listinlet_class;
    x->li_size        = 0;
    x->li_hasPointer  = 0;
    x->li_vector      = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_listSet (t_listinlet *x, int argc, t_atom *argv)
{
    int i;
    
    listinlet_clear (x);
    
    x->li_size   = argc;
    x->li_vector = (t_listinletelement *)PD_MEMORY_GET (x->li_size * sizeof (t_listinletelement));
    
    for (i = 0; i < x->li_size; i++) {
    //
    x->li_vector[i].le_atom = argv[i];
    listinlet_cachePointer (x, i, argv + i);
    //
    }
}

void listinlet_listAppend (t_listinlet *x, int argc, t_atom *argv)
{

}

void listinlet_listPrepend (t_listinlet *x, int argc, t_atom *argv)
{
    
}

int listinlet_hasPointer (t_listinlet *x)
{
    return x->li_hasPointer;
}

int listinlet_getSize (t_listinlet *x)
{
    return x->li_size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_clear (t_listinlet *x)
{
    int i;
    
    for (i = 0; i < x->li_size; i++) {
    //
    if (IS_POINTER (&x->li_vector[i].le_atom)) { gpointer_unset (GET_POINTER (&x->li_vector[i].le_atom)); }
    //
    }
    
    if (x->li_vector) { PD_MEMORY_FREE (x->li_vector); }
    
    listinlet_init (x);
}

void listinlet_clone (t_listinlet *x, t_listinlet *y)
{
    int i;
    
    y->li_pd         = x->li_pd;
    y->li_size       = x->li_size;
    y->li_hasPointer = x->li_hasPointer;
    y->li_vector     = (t_listinletelement *)PD_MEMORY_GET (x->li_size * sizeof (t_listinletelement));

    for (i = 0; i < x->li_size; i++) {
    //
    y->li_vector[i].le_atom = x->li_vector[i].le_atom;
    listinlet_cachePointer (y, i, &x->li_vector[i].le_atom);
    //
    }
}

void listinlet_copyAtomsUnchecked (t_listinlet *x, t_atom *a)
{
    int i; for (i = 0; i < x->li_size; i++) { a[i] = x->li_vector[i].le_atom; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listinlet_list (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    listinlet_listSet (x, argc, argv);
}

static void listinlet_anything (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)listinlet_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_listinlet,
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);

    class_addList (c, (t_method)listinlet_list);
    class_addAnything (c, (t_method)listinlet_anything);
    
    listinlet_class = c;
}

void listinlet_destroy (void)
{
    class_free (listinlet_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

