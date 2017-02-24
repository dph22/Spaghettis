
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* That file implement behaviors for comments. */
/* Note that they are used also by every text object (that means rather ALL objects). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *canvas_class;
extern t_pd     *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *text_class;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_widgetbehavior text_widgetBehavior =              /* Shared. */
    {
        text_behaviorGetRectangle,
        text_behaviorDisplaced,
        text_behaviorSelected,
        text_behaviorActivated,
        text_behaviorDeleted,
        text_behaviorVisibilityChanged,
        text_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_object *x     = cast_object (z);
    t_boxtext *text = boxtext_fetch (glist, x);
    int width       = boxtext_getWidth (text);
    int height      = boxtext_getHeight (text);
    int a           = text_getPixelX (x, glist);
    int b           = text_getPixelY (x, glist);
    
    rectangle_set (r, a, b, a + width, b + height);
}

void text_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_object *x = cast_object (z);
    
    x->te_xCoordinate += deltaX;
    x->te_yCoordinate += deltaY;
    
    if (canvas_isMapped (glist)) {
    //
    t_boxtext *text = boxtext_fetch (glist, x);
    boxtext_displace (text, deltaX, deltaY);
    canvas_drawBox (glist, x, boxtext_getTag (text), 0);
    canvas_updateLinesByObject (glist, x);
    //
    }
}

void text_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_object *x = cast_object (z);

    if (canvas_isMapped (glist)) {
    //
    t_boxtext *text = boxtext_fetch (glist, x);
    
    boxtext_select (text, isSelected);
    
    if (gobj_isVisible (z, glist)) {
    //
    sys_vGui (".x%lx.c itemconfigure %sBORDER -fill #%06x\n",
                    glist, 
                    boxtext_getTag (text),
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
    //
    }
    //
    }
}

void text_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    boxtext_activate (boxtext_fetch (glist, cast_object (z)), isActivated);
}

void text_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    canvas_deleteLinesByObject (glist, cast_object (z));
}

void text_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_object *x = cast_object (z);
    
    if (gobj_isVisible (z, glist)) {
    //
    t_boxtext *text = boxtext_fetch (glist, x);
    
    if (isVisible) { boxtext_draw (text); canvas_drawBox (glist, x, boxtext_getTag (text), 1); } 
    else {
        canvas_eraseBox (glist, x, boxtext_getTag (text)); boxtext_erase (text);
    }
    //
    }
}

int text_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_object *x = cast_object (z);
    
    t_float f1 = (t_float)m->m_x;
    t_float f2 = (t_float)m->m_y;
    t_float f3 = (t_float)m->m_shift;
    t_float f4 = (t_float)m->m_ctrl;
    t_float f5 = (t_float)m->m_alt;
    
    if (x->te_type == TYPE_OBJECT) {
        if (class_hasMethod (pd_class (x), sym_click)) {
        //
        if (m->m_clicked) { pd_vMessage (cast_pd (x), sym_click, "fffff", f1, f2, f3, f4, f5); }
        return 1;
        //
        }
        
    } else if (x->te_type == TYPE_ATOM) {
        if (m->m_clicked) { gatom_click ((t_gatom *)x, f1, f2, f3, f4, f5); }
        return 1;
        
    } else if (x->te_type == TYPE_MESSAGE) {
        if (m->m_clicked) { message_click ((t_message *)x, f1, f2, f3, f4, f5); }
        return 1;
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_functionSave (t_gobj *z, t_buffer *b)
{
    t_object *x = cast_object (z);
    
    if (x->te_type == TYPE_COMMENT) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_text, x->te_xCoordinate, x->te_yCoordinate);
        
    } else if (x->te_type == TYPE_OBJECT) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_obj,  x->te_xCoordinate, x->te_yCoordinate);
        
    } else if (x->te_type == TYPE_MESSAGE) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_msg,  x->te_xCoordinate, x->te_yCoordinate);
        
    } else { 
        PD_BUG;
    }
    
    buffer_serialize (b, x->te_buffer);
    buffer_appendSemicolon (b);
    object_saveWidth (x, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_set (t_object *x, t_glist *glist, char *s, int size)
{
    if (x->te_type != TYPE_OBJECT) { buffer_withStringUnzeroed (x->te_buffer, s, size); }
    else {
    //
    t_buffer *t = buffer_new();
    buffer_withStringUnzeroed (t, s, size);
    
    {
    //
    int m = (utils_getFirstAtomOfObjectAsSymbol (x) == sym_pd);
    int n = (utils_getFirstAtomOfBufferAsSymbol (t) == sym_pd);
    
    if (m && n) {
        pd_message (cast_pd (x), sym_rename, buffer_size (t) - 1, buffer_atoms (t) + 1);
        buffer_free (x->te_buffer);
        x->te_buffer = t;
        
    } else {
        int w = x->te_width;
        int a = x->te_xCoordinate;
        int b = x->te_yCoordinate;
        
        canvas_removeObject (glist, cast_gobj (x));
        canvas_makeTextObject (glist, a, b, w, 0, t);
        canvas_restoreCachedLines (canvas_getView (glist));
        
        if (pd_newest) {
            if (pd_class (pd_newest) == canvas_class) { canvas_loadbang (cast_glist (pd_newest)); }
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int text_getPixelX (t_object *x, t_glist *glist)
{
    if (canvas_canHaveWindow (glist)) { return x->te_xCoordinate; }
    else {
        int n = canvas_valueToPixelX (glist, glist->gl_valueLeft) - glist->gl_graphMarginLeft;
        return (n + x->te_xCoordinate);
    }
}

int text_getPixelY (t_object *x, t_glist *glist)
{
    if (canvas_canHaveWindow (glist)) { return x->te_yCoordinate; }
    else {
        int n = canvas_valueToPixelY (glist, glist->gl_valueTop) - glist->gl_graphMarginTop;
        return (n + x->te_yCoordinate);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void text_anything (t_object *x, t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text, 
            NULL,
            NULL, 
            sizeof (t_object),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
        
    class_addAnything (c, (t_method)text_anything);
    
    text_class = c;
}

void text_destroy (void)
{
    CLASS_FREE (text_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
