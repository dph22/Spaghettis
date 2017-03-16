
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
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* That file implement behaviors for comments. */
/* Note that they are used also by every text object (that means rather ALL objects). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *text_class;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_behaviorGetRectangle      (t_gobj *, t_glist *, t_rectangle *);
void text_behaviorDisplaced         (t_gobj *, t_glist *, int, int);
void text_behaviorSelected          (t_gobj *, t_glist *, int);
void text_behaviorActivated         (t_gobj *, t_glist *, int);
void text_behaviorDeleted           (t_gobj *, t_glist *);
void text_behaviorVisibilityChanged (t_gobj *, t_glist *, int);
int  text_behaviorMouse             (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    t_object *x = cast_object (z);
    t_box *text = box_fetch (glist, x);
    int width   = box_getWidth (text);
    int height  = box_getHeight (text);
    int a       = text_getPixelX (x, glist);
    int b       = text_getPixelY (x, glist);
    
    rectangle_set (r, a, b, a + width, b + height);
}

void text_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_object *x = cast_object (z);
    
    object_setX (x, object_getX (x) + deltaX);
    object_setY (x, object_getY (x) + deltaY);
    
    if (canvas_isMapped (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    box_displace (text, deltaX, deltaY);
    canvas_drawBox (glist, x, box_getTag (text), 0);
    canvas_updateLinesByObject (glist, x);
    //
    }
}

void text_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_object *x = cast_object (z);

    if (canvas_isMapped (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    
    box_select (text, isSelected);
    
    if (gobj_isVisible (z, glist)) {
    //
    sys_vGui (".x%lx.c itemconfigure %sBORDER -fill #%06x\n",
                    glist, 
                    box_getTag (text),
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
    //
    }
    //
    }
}

void text_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    box_activate (box_fetch (glist, cast_object (z)), isActivated);
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
    t_box *text = box_fetch (glist, x);
    
    if (isVisible) { box_draw (text); canvas_drawBox (glist, x, box_getTag (text), 1); } 
    else {
        canvas_eraseBox (glist, x, box_getTag (text)); box_erase (text);
    }
    //
    }
}

int text_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_object *x = cast_object (z);
    
    int k = object_isAtom (x) || object_isMessage (x);
    
    if (k || (object_isObject (x) && class_hasMethod (pd_class (x), sym_click))) { 
        if (m->m_clicked) {
            pd_message (cast_pd (x), sym_click, mouse_argc (m), mouse_argv (m)); 
        }
        
        return 1;
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_set (t_object *x, t_glist *glist, char *s, int size)
{
    if (!object_isObject (x)) { buffer_withStringUnzeroed (object_getBuffer (x), s, size); }
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
        buffer_free (object_getBuffer (x)); 
        object_setBuffer (x, t);
        
    } else {
        int w = object_getWidth (x);
        int a = object_getX (x);
        int b = object_getY (x);
        
        canvas_removeObject (glist, cast_gobj (x));
        canvas_makeTextObject (glist, a, b, w, 0, t);
        canvas_restoreCachedLines (canvas_getView (glist));
        
        if (instance_getNewestObject() && pd_class (instance_getNewestObject()) == canvas_class) {
            canvas_loadbang (cast_glist (instance_getNewestObject())); 
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
    if (canvas_canHaveWindow (glist)) { return object_getX (x); }
    else {
        int n = canvas_valueToPixelX (glist, bounds_getLeft (&glist->gl_bounds));
        return (n - rectangle_getTopLeftX (&glist->gl_geometryGraph) + object_getX (x));
    }
}

int text_getPixelY (t_object *x, t_glist *glist)
{
    if (canvas_canHaveWindow (glist)) { return object_getY (x); }
    else {
        int n = canvas_valueToPixelY (glist, bounds_getTop (&glist->gl_bounds));
        return (n - rectangle_getTopLeftY (&glist->gl_geometryGraph) + object_getY (x));
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
