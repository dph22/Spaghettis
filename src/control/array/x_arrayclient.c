
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

#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error arrayclient_init (t_arrayclient *x, int *ac, t_atom **av)
{
    int argc = *ac;
    t_atom *argv = *av;
    
    x->ac_name               = NULL;
    x->ac_templateIdentifier = NULL;
    x->ac_fieldName          = NULL;
    
    gpointer_init (&x->ac_gpointer);
    
    while (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *t = GET_SYMBOL (argv);
    
    if (t == sym___dash__template) {
        if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
            x->ac_templateIdentifier = template_makeIdentifierWithWildcard (GET_SYMBOL (argv + 1));
            x->ac_fieldName = GET_SYMBOL (argv + 2);
            argc -= 3; argv += 3;
        } else {
            return PD_ERROR;
        }
    } 

    break;
    //
    }

    if (!x->ac_templateIdentifier && argc) {
    
        /* Dollar expansion is zero in abstraction opened as patch. */
        
        if (IS_FLOAT (argv) && (GET_FLOAT (argv) == 0.0)) { x->ac_name = &s_; argc--; argv++; }
        else {
            if (!IS_SYMBOL (argv)) { return PD_ERROR; }
            else {
                x->ac_name = GET_SYMBOL (argv); argc--; argv++;
            }
        }
    }
    
    *ac = argc;
    *av = argv;
    
    return PD_ERROR_NONE;
}

void arrayclient_free (t_arrayclient *x)
{
    gpointer_unset (&x->ac_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *arrayclient_getName (t_arrayclient *x)
{
    return (x->ac_name ? x->ac_name : &s_);
}

void arrayclient_setName (t_arrayclient *x, t_symbol *s)
{
    if (s && s != &s_) { x->ac_name = s; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayclient_restore (t_arrayclient *x, t_arrayclient *old)
{
    if (ARRAYCLIENT_HAS_POINTER (x)) { gpointer_setByCopy (&x->ac_gpointer, &old->ac_gpointer); }
    else {
        arrayclient_setName (x, arrayclient_getName (old));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_garray *arrayclient_fetchGraphicArray (t_arrayclient *x)
{
    PD_ASSERT (x->ac_name); return (t_garray *)symbol_getThingByClass (x->ac_name, garray_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *arrayclient_fetchArray (t_arrayclient *x)
{
    if (x->ac_name) {
    
        t_garray *y = arrayclient_fetchGraphicArray (x);
        
        if (y) { return garray_getArray (y); }
        else {
            error_canNotFind (sym_array, x->ac_name);
        }
        
    } else if (x->ac_templateIdentifier) {
    
        if (gpointer_isValidInstanceOf (&x->ac_gpointer, x->ac_templateIdentifier)) {
            if (gpointer_hasField (&x->ac_gpointer, x->ac_fieldName)) {
                if (gpointer_fieldIsArrayAndValid (&x->ac_gpointer, x->ac_fieldName)) {
                    return gpointer_getArray (&x->ac_gpointer, x->ac_fieldName);
                    
        } else { error_invalid (sym_array, x->ac_fieldName); }
        } else { error_missingField (sym_array, x->ac_fieldName); }
        } else { error_invalid (sym_array, &s_pointer); }
    }
    
    return NULL;
}

t_glist *arrayclient_fetchOwner (t_arrayclient *x)
{
    if (x->ac_name) {
    
        t_garray *y = arrayclient_fetchGraphicArray (x);
        
        if (y) { return garray_getOwner (y); }
        else {
            error_canNotFind (sym_array, x->ac_name);
        }
        
    } else if (x->ac_templateIdentifier) {
    
        if (gpointer_isValidInstanceOf (&x->ac_gpointer, x->ac_templateIdentifier)) {
            if (gpointer_hasField (&x->ac_gpointer, x->ac_fieldName)) {
                if (gpointer_fieldIsArrayAndValid (&x->ac_gpointer, x->ac_fieldName)) {
                    return gpointer_getOwner (&x->ac_gpointer);
                    
        } else { error_invalid (sym_array, x->ac_fieldName); }
        } else { error_missingField (sym_array, x->ac_fieldName); }
        } else { error_invalid (sym_array, &s_pointer); }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayclient_update (t_arrayclient *x)
{
    t_array *array = arrayclient_fetchArray (x);
    t_glist *view  = arrayclient_fetchOwner (x);
    
    PD_ASSERT (array);
    PD_ASSERT (view);
        
    array_redraw (array, view);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
