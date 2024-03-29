
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

static void gatom_float                     (t_gatom *, t_float);
static void gatom_set                       (t_gatom *, t_symbol *, int, t_atom *);
static void gatom_motion                    (void *, t_float, t_float, t_float);
static void gatom_behaviorVisibilityChanged (t_gobj *, t_glist *, int);
static void gatom_restoreProceed            (t_gatom *, t_float, t_float, t_symbol *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *gatom_class;                                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gatom {
    t_object        a_obj;                              /* MUST be the first. */
    t_atom          a_atom;
    t_float         a_lowRange;
    t_float         a_highRange;
    int             a_position;                         /* Unused but kept for compatibility. */
    t_glist         *a_owner;
    t_symbol        *a_send;
    t_symbol        *a_receive;
    t_symbol        *a_label;                           /* Unused but kept for compatibility. */
    t_symbol        *a_unexpandedSend;
    t_symbol        *a_unexpandedReceive;
    t_symbol        *a_unexpandedLabel;                 /* Unused but kept for compatibility. */
    t_outlet        *a_outlet;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void text_behaviorGetRectangle      (t_gobj *, t_glist *, t_rectangle *);
void text_behaviorDisplaced         (t_gobj *, t_glist *, int, int);
void text_behaviorSelected          (t_gobj *, t_glist *, int);
void text_behaviorDeleted           (t_gobj *, t_glist *);
void text_behaviorVisibilityChanged (t_gobj *, t_glist *, int);
int  text_behaviorMouse             (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior gatom_widgetBehavior =          /* Shared. */
    {
        text_behaviorGetRectangle,
        text_behaviorDisplaced,
        text_behaviorSelected,
        NULL,
        text_behaviorDeleted,
        gatom_behaviorVisibilityChanged,
        text_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ATOM_WIDTH_FLOAT        5
#define ATOM_WIDTH_SYMBOL       10
#define ATOM_WIDTH_MAXIMUM      80

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_drawJob (t_gobj *z, t_glist *glist)
{
    t_gatom *x = (t_gatom *)z;
    
    box_retext (box_fetch (x->a_owner, cast_object (x)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_symbol *gatom_parse (t_symbol *s)
{
    if (symbol_isNilOrDash (s)) { return &s_; } else { return symbol_hashToDollar (s); }
}

static void gatom_update (t_gatom *x)
{
    buffer_clear (object_getBuffer (cast_object (x)));
    buffer_appendAtom (object_getBuffer (cast_object (x)), &x->a_atom);
    gui_jobAdd ((void *)x, x->a_owner, gatom_drawJob);
}

static int gatom_isFloat (t_gatom *x)
{
    return IS_FLOAT (&x->a_atom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_bang (t_gatom *x)
{
    if (gatom_isFloat (x)) {
    
        outlet_float (x->a_outlet, GET_FLOAT (&x->a_atom));
        
        if (x->a_send != &s_ && symbol_hasThing (x->a_send)) {
            if (x->a_send != x->a_receive) {
                pd_float (symbol_getThing (x->a_send), GET_FLOAT (&x->a_atom));
            } else {
                error_sendReceiveLoop (x->a_unexpandedSend);
            }
        }
        
    } else {
    
        outlet_symbol (x->a_outlet, GET_SYMBOL (&x->a_atom));
        
        if (x->a_send != &s_ && symbol_hasThing (x->a_send)) {
            if (x->a_send != x->a_receive) {
                pd_symbol (symbol_getThing (x->a_send), GET_SYMBOL (&x->a_atom));
            } else {
                error_sendReceiveLoop (x->a_unexpandedSend);
            }
        }
    }
}

static void gatom_float (t_gatom *x, t_float f)
{
    t_atom a;
    SET_FLOAT (&a, f);
    gatom_set (x, NULL, 1, &a);
    gatom_bang (x);
}

static void gatom_symbol (t_gatom *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
    gatom_set (x, NULL, 1, &a);
    gatom_bang (x);
}

void gatom_click (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gatom_isFloat (x)) {
    //
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    glist_setMotion (x->a_owner, cast_gobj (x), (t_motionfn)gatom_motion, a, b);
    //
    }
    
    gatom_bang (x);
}

static void gatom_set (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    if (!gatom_isFloat (x)) { SET_SYMBOL (&x->a_atom, atom_getSymbol (argv)); }
    else {
    //
    t_float f = atom_getFloat (argv);
    if (x->a_lowRange != 0.0 || x->a_highRange != 0.0) {
        f = PD_CLAMP (f, x->a_lowRange, x->a_highRange);
    }
    SET_FLOAT (&x->a_atom, f);
    //
    }

    gatom_update (x);
    //
    }
}

static void gatom_range (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gatom_isFloat (x)) {
    //
    t_float minimum = atom_getFloatAtIndex (0, argc, argv);
    t_float maximum = atom_getFloatAtIndex (1, argc, argv);
    
    x->a_lowRange   = PD_MIN (minimum, maximum);
    x->a_highRange  = PD_MAX (minimum, maximum);
    
    gatom_set (x, NULL, 1, &x->a_atom);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_gatom *x = (t_gatom *)z;
    
    PD_ASSERT (gatom_isFloat (x));
    
    if (deltaY != 0.0) { 
    //
    double f = GET_FLOAT (&x->a_atom);
    
    if ((int)modifier & MODIFIER_SHIFT) { f -= 0.01 * deltaY; }
    else {
        f -= deltaY;
    }
    
    gatom_float (x, (t_float)f);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    text_behaviorVisibilityChanged (z, glist, isVisible);
    
    if (!isVisible) { gui_jobRemove ((void *)z); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_gatom *x = (t_gatom *)z;
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, gatom_isFloat (x) ? sym_floatatom : sym_symbolatom);
    buffer_appendFloat (b,  object_getX (cast_object (x)));
    buffer_appendFloat (b,  object_getY (cast_object (x)));
    buffer_appendFloat (b,  object_getWidth (cast_object (x)));
    buffer_appendFloat (b,  x->a_lowRange);
    buffer_appendFloat (b,  x->a_highRange);
    buffer_appendFloat (b,  x->a_position);
    buffer_appendSymbol (b, symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedLabel)));
    buffer_appendSymbol (b, symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedReceive)));
    buffer_appendSymbol (b, symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedSend)));
    if (SAVED_DEEP (flags)) { buffer_appendAtom (b, &x->a_atom); }
    buffer_appendSemicolon (b);
    
    object_serializeWidth (cast_object (x), b);
    
    gobj_saveUniques (z, b, flags);
}

static t_buffer *gatom_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);

    return b;
    //
    }
    
    return NULL;
}

static void gatom_functionValue (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_gatom *x  = (t_gatom *)z;
    t_error err = PD_ERROR_NONE;
    t_heapstring *t = heapstring_new (0);
    
    if (gatom_isFloat (x)) {
        err = heapstring_addSprintf (t,
                "::ui_value::show %%s atom float %.9g\n",       // --
                GET_FLOAT (&x->a_atom));
        
    } else {
        err = heapstring_addSprintf (t,
                "::ui_value::show %%s atom symbol {%s}\n",      // --
                symbol_dollarToHash (GET_SYMBOL (&x->a_atom))->s_name);
    }
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, heapstring_getRaw (t));
    
    heapstring_free (t);
}

/* Fake dialog message from interpreter. */

static void gatom_functionUndo (t_gobj *z, t_buffer *b)
{
    t_gatom *x = (t_gatom *)z;
    
    buffer_appendSymbol (b, sym__gatomdialog);
    buffer_appendFloat (b,  object_getWidth (cast_object (x)));
    buffer_appendFloat (b,  x->a_lowRange);
    buffer_appendFloat (b,  x->a_highRange);
    buffer_appendSymbol (b, symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedSend)));
    buffer_appendSymbol (b, symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedReceive)));
}

static void gatom_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_gatom *x  = (t_gatom *)z;
    t_error err = PD_ERROR_NONE;
    t_heapstring *t = heapstring_new (0);
    
    t_symbol *symSend    = symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedSend));
    t_symbol *symReceive = symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedReceive));
    
    err = heapstring_addSprintf (t,
                "::ui_atom::show %%s %d %.9g %.9g %s {%s} {%s}\n",      // --
                object_getWidth (cast_object (x)),
                x->a_lowRange,
                x->a_highRange,
                gatom_isFloat (x) ? sym_floatatom->s_name : sym_symbolatom->s_name,
                symSend->s_name,
                symReceive->s_name);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, heapstring_getRaw (t));
    
    heapstring_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_fromValue (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gatom_isFloat (x)) { gatom_float (x, atom_getFloatAtIndex (0, argc, argv)); }
    else {
        gatom_symbol (x, symbol_hashToDollar (atom_getSymbolAtIndex (0, argc, argv)));
    }
}

static void gatom_fromDialog (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    int undoable = glist_undoIsOk (x->a_owner);
    
    t_float t1   = x->a_lowRange;
    t_float t2   = x->a_highRange;
    int t3       = object_getWidth (cast_object (x));
    t_symbol *t4 = x->a_send;
    t_symbol *t5 = x->a_receive;
    
    PD_ASSERT (argc == 5);
    
    t_undosnippet *s1 = NULL;
    t_undosnippet *s2 = NULL;
    
    if (undoable) { s1 = undosnippet_newProperties (cast_gobj (x), x->a_owner); }

    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 0);
    
    {
    //
    t_float width        = atom_getFloatAtIndex (0, argc, argv);
    t_float lowRange     = atom_getFloatAtIndex (1, argc, argv);
    t_float highRange    = atom_getFloatAtIndex (2, argc, argv);
    t_symbol *symSend    = gatom_parse (atom_getSymbolAtIndex (3, argc, argv));
    t_symbol *symReceive = gatom_parse (atom_getSymbolAtIndex (4, argc, argv));

    gatom_restoreProceed (x, lowRange, highRange, symSend, symReceive);
    object_setWidth (cast_object (x), PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM));
    gatom_update (x);
    //
    }
    
    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 1);
    
    if (undoable) { s2 = undosnippet_newProperties (cast_gobj (x), x->a_owner); }
    
    isDirty |= (t1 != x->a_lowRange);
    isDirty |= (t2 != x->a_highRange);
    
    if (isDirty && gatom_isFloat (x)) { gatom_set (x, NULL, 1, &x->a_atom); }
    
    isDirty |= (t3 != object_getWidth (cast_object (x)));
    isDirty |= (t4 != x->a_send);
    isDirty |= (t5 != x->a_receive);
    
    if (isDirty) { glist_setDirty (x->a_owner, 1); }
    
    if (undoable) {
    //
    if (isDirty) {
        glist_undoAppend (x->a_owner, undoproperties_new (cast_gobj (x), s1, s2));
        glist_undoAppendSeparator (x->a_owner);
        
    } else {
        undosnippet_free (s1);
        undosnippet_free (s2);
    }
    
    s1 = NULL;
    s2 = NULL;
    //
    }
    
    PD_ASSERT (s1 == NULL);
    PD_ASSERT (s2 == NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Encapsulation. */

static void gatom_restoreProceed (t_gatom *x, t_float low, t_float high, t_symbol *send, t_symbol *receive)
{
    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }

    x->a_lowRange           = PD_MIN (low, high);
    x->a_highRange          = PD_MAX (low, high);
    x->a_unexpandedSend     = send;
    x->a_unexpandedReceive  = receive;
    x->a_send               = dollar_expandSymbol (x->a_unexpandedSend, x->a_owner);
    x->a_receive            = dollar_expandSymbol (x->a_unexpandedReceive, x->a_owner);
    
    if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }
}

static void gatom_restore (t_gatom *x)
{
    t_gatom *old = (t_gatom *)instance_pendingFetch (cast_gobj (x));
    
    if (old) {
    //
    t_float low       = old->a_lowRange;
    t_float high      = old->a_highRange;
    t_symbol *send    = old->a_unexpandedSend;
    t_symbol *receive = old->a_unexpandedReceive;
    
    gatom_restoreProceed (x, low, high, send, receive); gatom_set (x, NULL, 1, &old->a_atom);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_makeObjectFile (t_gatom *x, int argc, t_atom *argv)
{
    int width    = (int)atom_getFloatAtIndex (2, argc, argv);
    int position = (int)atom_getFloatAtIndex (5, argc, argv);

    width        = PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM);
    
    object_setX (cast_object (x), atom_getFloatAtIndex (0, argc, argv));
    object_setY (cast_object (x), atom_getFloatAtIndex (1, argc, argv));
    object_setWidth (cast_object (x), width);

    x->a_lowRange           = atom_getFloatAtIndex (3, argc, argv);
    x->a_highRange          = atom_getFloatAtIndex (4, argc, argv);
    x->a_position           = position;
    x->a_unexpandedLabel    = gatom_parse (atom_getSymbolAtIndex (6, argc, argv));
    x->a_unexpandedReceive  = gatom_parse (atom_getSymbolAtIndex (7, argc, argv));
    x->a_unexpandedSend     = gatom_parse (atom_getSymbolAtIndex (8, argc, argv));
    x->a_send               = dollar_expandSymbol (x->a_unexpandedSend, x->a_owner);
    x->a_receive            = dollar_expandSymbol (x->a_unexpandedReceive, x->a_owner);
    x->a_label              = dollar_expandSymbol (x->a_unexpandedLabel, x->a_owner);
            
    if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }
    
    if (argc > 9) { gatom_set (x, NULL, 1, argv + 9); }
    else {
        t_atom a; SET_SYMBOL (&a, &s_symbol); gatom_set (x, NULL, 1, &a);
    }
}

static void gatom_makeObjectMenu (t_gatom *x, int argc, t_atom *argv)
{
    int a = instance_getDefaultX (x->a_owner);
    int b = instance_getDefaultY (x->a_owner);
    
    glist_deselectAll (x->a_owner);
    
    if (atom_getSymbolAtIndex (0, argc, argv) == sym_menu) {
    //
    t_point pt = glist_getPositionForNewObject (x->a_owner);
    
    a = point_getX (&pt);
    b = point_getY (&pt);
    //
    }
    
    object_setSnappedX (cast_object (x), a);
    object_setSnappedY (cast_object (x), b);
}

static void gatom_makeObjectProceed (t_glist *glist, t_atomtype type, int argc, t_atom *argv)
{
    int isMenu  = (argc <= 1);
    
    t_gatom *x  = (t_gatom *)pd_new (gatom_class);
    
    t_buffer *t = buffer_new();
    
    x->a_owner              = glist;
    x->a_lowRange           = 0;
    x->a_highRange          = 0;
    x->a_position           = 1;
    x->a_send               = &s_;
    x->a_receive            = &s_;
    x->a_label              = &s_;
    x->a_unexpandedSend     = &s_;
    x->a_unexpandedReceive  = &s_;
    x->a_unexpandedLabel    = &s_;
    
    if (type == A_FLOAT) {
        t_atom a;
        SET_FLOAT (&x->a_atom, 0.0);
        SET_FLOAT (&a, 0.0);
        buffer_appendAtom (t, &a);
        
    } else {
        t_atom a;
        SET_SYMBOL (&x->a_atom, &s_symbol);
        SET_SYMBOL (&a, &s_symbol);
        buffer_appendAtom (t, &a);
    }
    
    object_setBuffer (cast_object (x), t);
    object_setWidth (cast_object (x),  type == A_FLOAT ? ATOM_WIDTH_FLOAT : ATOM_WIDTH_SYMBOL);
    object_setType (cast_object (x),   TYPE_ATOM);
    
    if (isMenu) { gatom_makeObjectMenu (x, argc, argv); }
    else {
        gatom_makeObjectFile (x, argc, argv);
    }
    
    x->a_outlet = gatom_isFloat (x) ? outlet_newFloat (cast_object (x)) : outlet_newSymbol (cast_object (x));
    
    glist_objectAdd (x->a_owner, cast_gobj (x));
    
    if (isMenu) { glist_objectSelect (x->a_owner, cast_gobj (x)); }
    
    instance_setBoundA (cast_pd (x));
}

void gatom_makeObjectFloat (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    gatom_makeObjectProceed (glist, A_FLOAT, argc, argv);
}

void gatom_makeObjectSymbol (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    gatom_makeObjectProceed (glist, A_SYMBOL, argc, argv);
}

static void gatom_free (t_gatom *x)
{
    gui_jobRemove ((void *)x);
    
    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gatom_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_gatom,
            NULL,
            (t_method)gatom_free,
            sizeof (t_gatom),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, (t_method)gatom_bang);
    class_addFloat (c, (t_method)gatom_float);
    class_addSymbol (c, (t_method)gatom_symbol);
    class_addClick (c, (t_method)gatom_click);
        
    class_addMethod (c, (t_method)gatom_set,        sym_set,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_fromValue,  sym__valuedialog,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_fromDialog, sym__gatomdialog,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_range,      sym_range,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_restore,    sym__restore,       A_NULL);

    class_setWidgetBehavior (c, &gatom_widgetBehavior);
    class_setSaveFunction (c, gatom_functionSave);
    class_setDataFunction (c, gatom_functionData);
    class_setValueFunction (c, gatom_functionValue);
    class_setUndoFunction (c, gatom_functionUndo);
    class_setPropertiesFunction (c, gatom_functionProperties);
    class_requirePending (c);
    
    gatom_class = c;
}

void gatom_destroy (void)
{
    class_free (gatom_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
