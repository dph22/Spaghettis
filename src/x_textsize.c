
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *textsize_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textsize {
    t_textclient    x_textclient;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_textsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsize_bang (t_textsize *x)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) { outlet_float (x->x_outlet, (t_float)buffer_getNumberOfMessages (b)); }
    else {
        error_undefined (sym_text__space__size, sym_text);
    }
}

static void textsize_float (t_textsize *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
        int start, end;
        if (buffer_getMessageAt (b, f, &start, &end)) { outlet_float (x->x_outlet, (t_float)(end - start)); }
        else {
            outlet_float (x->x_outlet, -1);
        }
        
    } else { error_undefined (sym_text__space__size, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textsize_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textsize *x = (t_textsize *)pd_new (textsize_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        x->x_outlet = outlet_new (cast_object (x), &s_float);
                
        if (argc) { warning_unusedArguments (sym_text__space__size, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
        
    } else {
        
        error_invalidArguments (sym_text__space__size, argc, argv);
        pd_free (x); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textsize_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__size,
            (t_newmethod)textsize_new,
            (t_method)textclient_free,
            sizeof (t_textsize),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)textsize_bang);
    class_addFloat (c, (t_method)textsize_float);
    
    class_setHelpName (c, sym_text);
    
    textsize_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
