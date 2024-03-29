
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *fields_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _fields {
    t_object x_obj;                 /* Must be the first. */
    t_outlet *x_outlet;
    } t_fields;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void fields_pointer (t_fields *x, t_gpointer *gp)
{
    t_buffer *b   = buffer_new();
    int isElement = gpointer_isWord (gp);
    
    if (isElement) { buffer_appendFloat (b, gpointer_getIndex (gp)); }
    
    if (gpointer_getFields (gp, b)) { error_invalid (sym_fields, &s_pointer); }
    else {
    //
    t_symbol *s = isElement ? sym_element : sym_scalar;
    
    outlet_anything (x->x_outlet, s, buffer_getSize (b), buffer_getAtoms (b));
    //
    }
    
    buffer_free (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *fields_new (t_symbol *s, int argc, t_atom *argv)
{
    t_fields *x = (t_fields *)pd_new (fields_class);
    
    x->x_outlet = outlet_newAnything (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fields_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_fields,
            (t_newmethod)fields_new,
            NULL,
            sizeof (t_fields),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addPointer (c, (t_method)fields_pointer);
    
    fields_class = c;
}

void fields_destroy (void)
{
    class_free (fields_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
