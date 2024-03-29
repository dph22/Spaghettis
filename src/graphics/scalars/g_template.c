
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

t_class *template_class;    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_isValid (t_template *x)
{
    if (!x) { return 0; }
    else {
    //
    t_dataslot *v = x->tp_slots;
    int i;
    
    for (i = 0; i < x->tp_size; i++, v++) {
    //
    if (v->ds_type == DATA_ARRAY) {

        t_template *y = template_findByIdentifier (v->ds_templateIdentifier);
        
        if (!y)     { return 0; }                           /* Element's template must exist. */
        if (y == x) { return 0; }                           /* Forbid circular dependencies. */
        if (template_containsArray (y)) { return 0; }       /* Forbid nested arrays. */
    }
    //
    }
    //
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int template_containsArray (t_template *x)
{
    t_dataslot *v = x->tp_slots;
    int i;
    
    for (i = 0; i < x->tp_size; i++, v++) { 
        if (v->ds_type == DATA_ARRAY) { 
            return 1; 
        } 
    }
    
    return 0;
}

int template_containsTemplate (t_template *x, t_symbol *templateIdentifier)
{
    if (x->tp_templateIdentifier == templateIdentifier) { return 1; }
    else {
    //
    t_dataslot *v = x->tp_slots;
    int i;

    for (i = 0; i < x->tp_size; i++, v++) { 
        if (v->ds_type == DATA_ARRAY) { 
            if (v->ds_templateIdentifier == templateIdentifier) { 
                return 1; 
            }
        } 
    }
    
    return 0;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_hasField (t_template *x, t_symbol *fieldName)
{
    int i, t; t_symbol *dummy = NULL;
    
    return template_getRaw (x, fieldName, &i, &t, &dummy);
}

int template_getIndexOfField (t_template *x, t_symbol *fieldName)
{
    int i, t; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &t, &dummy)) { return i; }
    else {
        return -1;
    }
}

t_symbol *template_getFieldAtIndex (t_template *x, int n)
{
    if (n >= 0 && n < x->tp_size) { return x->tp_slots[n].ds_fieldName; }
    else {
        return NULL;
    }
}

int template_getRaw (t_template *x,
    t_symbol *fieldName,
    int *position,
    int *type,
    t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_slots[i].ds_fieldName == fieldName) {
    
        *position           = i;
        *type               = x->tp_slots[i].ds_type;
        *templateIdentifier = x->tp_slots[i].ds_templateIdentifier;
        
        return 1;
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int template_fieldIsFloat (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_FLOAT); }
    
    return 0;
}

int template_fieldIsSymbol (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_SYMBOL); }
    
    return 0;
}

int template_fieldIsArray (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_ARRAY); }
    
    return 0;
}

int template_fieldIsArrayAndValid (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *templateIdentifier = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &templateIdentifier)) { 
        if (type == DATA_ARRAY) {
            PD_ASSERT (template_isValid (template_findByIdentifier (templateIdentifier))); 
            return 1;
        }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void template_anything (t_template *x, t_symbol *s, int argc, t_atom *argv)
{
    #if PD_WITH_DEBUG
    
    post ("%s", symbol_stripTemplateIdentifier (x->tp_templateIdentifier)->s_name);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void template_registerInstanceProceed (t_template *x, t_struct *o, int release)
{
    paint_erase();
    
    x->tp_pending = 0; x->tp_instance = o;
    
    if (!o) {
        if (release) { instance_destroyAllScalarsByTemplate (x); }
        else {
            x->tp_pending = 1;
        }
    }
    
    paint_draw();
}

void template_registerInstance (t_template *x, t_struct *o)
{
    template_registerInstanceProceed (x, o, 1);
}

void template_unregisterInstance (t_template *x, t_struct *o)
{
    template_registerInstanceProceed (x, NULL, 1);
    
    pd_free (cast_pd (x));
}

void template_forgetPendingInstance (t_template *x, t_struct *o)
{
    template_registerInstanceProceed (x, NULL, 0);
}

int template_hasInstance (t_template *x)
{
    return (x->tp_instance != NULL);
}

int template_hasPending (t_template *x)
{
    return (x->tp_pending != 0);
}

int template_hasInhibit (t_template *x)
{
    if (x->tp_instance) { return struct_hasInhibit (x->tp_instance); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void template_getInstancePaintersAppend (t_buffer *b, t_gobj *y)
{
    int m = object_getY (cast_object (y));
    int n = object_getX (cast_object (y));
    int i;
    
    t_atom a; SET_OBJECT (&a, y);
    
    for (i = 0; i < buffer_getSize (b); i++) {
    //
    int tY = object_getY (cast_object (buffer_getObjectAt (b, i)));
    int tX = object_getX (cast_object (buffer_getObjectAt (b, i)));
    
    if (tY > m) { break; } else if (tY == m && tX > n) { break; }
    //
    }
    
    buffer_insertAtIndex (b, i, &a);
}

/* Return the painters ordered if it contains at least one painter. */
/* Return NULL otherwise. */
/* Note that caller must free the buffer in the first case. */

t_buffer *template_getInstancePaintersIfAny (t_template *x)
{
    t_buffer *painters = NULL;
    
    t_glist *view = template_getInstanceOwner (x);

    if (view) {
    //
    t_gobj *y = NULL;
        
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    if (class_hasPainterBehavior (pd_class (y))) {
    //
    if (painters == NULL) { painters = buffer_new(); } template_getInstancePaintersAppend (painters, y);
    //
    }
    //
    }
    //
    }
    
    return painters;
}

/* Return the instance default contructor for a given field. */

t_constructor *template_getInstanceConstructorIfAny (t_template *x, t_symbol *field)
{
    t_glist *view = template_getInstanceOwner (x);
    
    if (view) {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
            if (pd_class (y) == constructor_class) {
                t_constructor *ctor = (t_constructor *)y;
                if (constructor_getField (ctor) == field) { return ctor; }
            }
        }
    }
    
    return NULL;
}

t_glist *template_getInstanceOwner (t_template *x)
{
    PD_ASSERT (x);
    
    if (!x->tp_instance) { return NULL; } else { return struct_getOwner (x->tp_instance); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *template_getUnexpandedName (t_template *x)
{
    if (!x->tp_instance) { return NULL; } else { return struct_getUnexpandedName (x->tp_instance); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_template *template_findByIdentifier (t_symbol *s)
{
    return ((t_template *)symbol_getThingByClass (s, template_class));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void template_addMenu (t_template *x)
{
    t_symbol *t = symbol_stripTemplateIdentifier (x->tp_templateIdentifier);

    if (!template_isPrivate (x->tp_templateIdentifier)) {
    if (template_fieldIsFloat (x, sym_x)) {
    if (template_fieldIsFloat (x, sym_y)) {
        gui_vAdd ("::ui_menu::appendTemplate {%s}\n", t->s_name);   // --
    }
    }
    }
}

static void template_removeMenu (t_template *x)
{
    t_symbol *t = symbol_stripTemplateIdentifier (x->tp_templateIdentifier);

    if (!template_isPrivate (x->tp_templateIdentifier)) {
    if (template_fieldIsFloat (x, sym_x)) {
    if (template_fieldIsFloat (x, sym_y)) {
        gui_vAdd ("::ui_menu::removeTemplate {%s}\n", t->s_name);   // --
    }
    }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error template_newParse (t_template *x, int *ac, t_atom **av)
{
    int argc = *ac; t_atom *argv = *av;
    
    while (argc > 0) {

        if ((argc >= 2) && IS_SYMBOL (argv + 0) && IS_SYMBOL (argv + 1)) {
        //
        t_symbol *type               = GET_SYMBOL (argv + 0);
        t_symbol *fieldName          = GET_SYMBOL (argv + 1);
        t_symbol *templateIdentifier = &s_;
        
        int k = -1;
        
        if (type == &s_float)        { k = DATA_FLOAT;  }
        else if (type == &s_symbol)  { k = DATA_SYMBOL; }
        else if (type == sym_array)  {
            if (argc >= 3 && IS_SYMBOL (argv + 2)) {
                templateIdentifier = symbol_makeTemplateIdentifier (GET_SYMBOL (argv + 2));
                k = DATA_ARRAY;
                argc--;
                argv++;
            } else {
                return PD_ERROR;
            }
        }
        
        if (k < 0) { error_invalid (sym_template, sym_type); }
        else {
        //
        int oldSize = x->tp_size;
        int newSize = x->tp_size + 1;
        size_t m = oldSize * sizeof (t_dataslot);
        size_t n = newSize * sizeof (t_dataslot);
        
        x->tp_slots = (t_dataslot *)PD_MEMORY_RESIZE (x->tp_slots, m, n);
        x->tp_size  = newSize;
        
        x->tp_slots[newSize - 1].ds_type               = k;
        x->tp_slots[newSize - 1].ds_fieldName          = fieldName;
        x->tp_slots[newSize - 1].ds_templateIdentifier = templateIdentifier;
        //
        }
        //
        } else {
            return PD_ERROR;
        }

    argc -= 2; *ac = argc;
    argv += 2; *av = argv;
    //
    }
    
    return PD_ERROR_NONE;
}
    
t_template *template_new (t_symbol *templateIdentifier, int argc, t_atom *argv)
{
    t_template *x = (t_template *)pd_new (template_class);
    
    PD_ASSERT (templateIdentifier);
    
    /* Empty template should be managed appropriately elsewhere. */
    
    PD_ASSERT (symbol_stripTemplateIdentifier (templateIdentifier) != &s_);
        
    x->tp_size               = 0;
    x->tp_error              = PD_ERROR_NONE;
    x->tp_slots              = (t_dataslot *)PD_MEMORY_GET (0);
    x->tp_templateIdentifier = templateIdentifier;
    x->tp_instance           = NULL;
    
    pd_bind (cast_pd (x), x->tp_templateIdentifier);
    
    if (template_newParse (x, &argc, &argv)) {      /* It may consume arguments. */
    //
    error_invalidArguments (symbol_stripTemplateIdentifier (templateIdentifier), argc, argv);
    x->tp_error = PD_ERROR; pd_free (cast_pd (x)); x = NULL;
    //
    } else { template_addMenu (x); }
    
    return x;
}

void template_free (t_template *x)
{
    if (x->tp_error == PD_ERROR_NONE) { template_removeMenu (x); }
    
    pd_unbind (cast_pd (x), x->tp_templateIdentifier);
    
    PD_MEMORY_FREE (x->tp_slots);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void template_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_template,
        NULL, 
        (t_method)template_free,
        sizeof (t_template),
        CLASS_INVISIBLE,
        A_NULL);
    
    class_addAnything (c, (t_method)template_anything);
    
    template_class = c;
}

void template_destroy (void)
{
    class_free (template_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
