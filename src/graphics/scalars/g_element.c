
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

static t_class *element_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _element {
    t_object    x_obj;                          /* Must be the first. */
    t_gpointer  x_gpointerWord;
    t_gpointer  x_gpointer;
    t_symbol    *x_templateIdentifier;
    t_symbol    *x_fieldName;
    t_outlet    *x_outlet;
    } t_element;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void element_float (t_element *x, t_float f)
{
    if (gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) {
        if (gpointer_hasField (&x->x_gpointer, x->x_fieldName)) {
            if (gpointer_fieldIsArrayAndValid (&x->x_gpointer, x->x_fieldName)) {
                t_array *array = gpointer_getArray (&x->x_gpointer, x->x_fieldName);
                gpointer_setAsWord (&x->x_gpointerWord, array, (int)f);
                outlet_pointer (x->x_outlet, &x->x_gpointerWord);
        
    } else { error_invalid (sym_element, x->x_fieldName); }
    } else { error_missingField (sym_element, x->x_fieldName); }
    } else { error_invalid (sym_element, &s_pointer); }
}

static void element_set (t_element *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *element_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_element *x = (t_element *)z;
    t_buffer *b  = buffer_new();
    
    buffer_appendSymbol (b, sym_set);
    buffer_appendSymbol (b, symbol_stripTemplateIdentifier (x->x_templateIdentifier));
    buffer_appendSymbol (b, x->x_fieldName);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

static void element_restore (t_element *x)
{
    t_element *old = (t_element *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { gpointer_setByCopy (&x->x_gpointer, &old->x_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *element_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_element *x = (t_element *)pd_new (element_class);
    
    gpointer_init (&x->x_gpointerWord);
    gpointer_init (&x->x_gpointer);
    
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
    x->x_outlet             = outlet_newPointer (cast_object (x));
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    
    return x;
}

static void element_free (t_element *x, t_gpointer *gp)
{
    gpointer_unset (&x->x_gpointerWord);
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void element_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_element,
            (t_newmethod)element_new,
            (t_method)element_free,
            sizeof (t_element),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, (t_method)element_float);
     
    class_addMethod (c, (t_method)element_set,      sym_set,        A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)element_restore,  sym__restore,   A_NULL);
    
    class_setDataFunction (c, element_functionData);
    class_requirePending (c);
    
    element_class = c;
}

void element_destroy (void)
{
    class_free (element_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
