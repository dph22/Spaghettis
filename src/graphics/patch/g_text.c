
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

/* That file implement behaviors for comments. */
/* Note that they are used also by every text object (that means rather ALL objects). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *text_class;                /* Shared. */

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
// MARK: -

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
    int a       = glist_getPixelX (glist, x);
    int b       = glist_getPixelY (glist, x);
    
    rectangle_set (r, a, b, a + width, b + height);
}

void text_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_object *x = cast_object (z);
    
    int m = object_setSnappedX (x, object_getX (x) + deltaX);
    int n = object_setSnappedY (x, object_getY (x) + deltaY);
    
    if (m || n) {
    //
    if (glist_isOnScreen (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    box_displace (text, m, n);
    glist_updateLinesForObject (glist, x);
    //
    }
    //
    }
}

void text_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_object *x = cast_object (z);

    if (glist_isOnScreen (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    
    box_select (text, isSelected);
    
    if (gobj_isViewable (z, glist)) {
    //
    gui_vAdd ("%s.c itemconfigure %sBORDER -fill #%06x\n",
                    glist_getTagAsString (glist), 
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
    glist_objectDeleteLines (glist, cast_object (z));
}

void text_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_object *x = cast_object (z);
    
    if (gobj_isViewable (z, glist)) {
    //
    t_box *text = box_fetch (glist, x);
    
    if (!isVisible) { box_erase (text); }
    else {
    //
    box_create (text);
    
    /* Required in various cases (width is changed in dialog and/or undo for instance). */
    
    box_update (text);
    //
    }
    //
    }
}

/* Mouse up message is never sent to window that lose the focus. */
/* Below a workaround. */
/* It uses another bind name because a bind list cannot be called recursively. */

static void text_behaviorMouseCancel (t_gobj *z, t_glist *glist)
{
    if (symbol_hasThingQuiet (sym__mousecancel)) {
    //
    t_atom t;
    SET_SYMBOL (&t, glist_getTag (glist));
    pd_message (symbol_getThing (sym__mousecancel), sym__mousecancel, 1, &t);
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
            if (gobj_isCanvas (z)) {
                text_behaviorMouseCancel (z, glist);
            }
        }
        
        return CURSOR_CLICK;
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void text_anything (t_object *x, t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_setHelpName (c, sym_comment);
    
    text_class = c;
}

void text_destroy (void)
{
    class_free (text_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
