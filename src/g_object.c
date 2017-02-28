
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;
extern t_class *scalar_class;
extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, t_rectangle *r)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnGetRectangle) {
        (*(pd_class (x)->c_behavior->w_fnGetRectangle)) (x, owner, r);
    }
}

void gobj_displaced (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDisplaced) {
        (*(pd_class (x)->c_behavior->w_fnDisplaced)) (x, owner, deltaX, deltaY);
    }
}

void gobj_selected (t_gobj *x, t_glist *owner, int isSelected)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnSelected) {
        (*(pd_class (x)->c_behavior->w_fnSelected)) (x, owner, isSelected);
    }
}

void gobj_activated (t_gobj *x, t_glist *owner, int isActivated)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnActivated) {
        (*(pd_class (x)->c_behavior->w_fnActivated)) (x, owner, isActivated);
    }
}

void gobj_deleted (t_gobj *x, t_glist *owner)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDeleted) {
        (*(pd_class (x)->c_behavior->w_fnDeleted)) (x, owner);
    }
}

int gobj_mouse (t_gobj *x, t_glist *owner, t_mouse *m)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnMouse) {
        return (*(pd_class (x)->c_behavior->w_fnMouse)) (x, owner, m);
    } else {
        return 0;
    }
}

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (pd_class (x)->c_fnSave) {
        (*(pd_class (x)->c_fnSave)) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gobj_hit (t_gobj *x, t_glist *owner, int positionX, int positionY, t_rectangle *r)
{
    if (gobj_isVisible (x, owner)) {
    //
    t_rectangle t;
    
    gobj_getRectangle (x, owner, &t);
    
    if (!rectangle_isNothing (&t) && rectangle_containsPoint (&t, positionX, positionY)) {
        rectangle_setCopy (r, &t);
        return 1;
    }
    //
    }
    
    return 0;
}

/* Always true if NOT a GOP. */

int gobj_isVisible (t_gobj *x, t_glist *owner)
{
    if (owner->gl_parent && !canvas_canHaveWindow (owner)) {
    //
    t_object *object = NULL;
            
    /* Is parent visible? */
    
    if (!gobj_isVisible (cast_gobj (owner), owner->gl_parent)) { return 0; }
    
    if (pd_class (x) == scalar_class)      { return 1; }    /* Always true. */
    else if (pd_class (x) == garray_class) { return 1; }    /* Ditto. */
    else {
    //
    
    /* Falling outside the graph rectangle? */
    
    t_rectangle r1, r2;
    gobj_getRectangle (cast_gobj (owner), owner->gl_parent, &r1);
    gobj_getRectangle (x, owner, &r2);
    if (!rectangle_containsRectangle (&r1, &r2)) {
        return 0; 
    }
    //
    }
    
    /* In GOP the only regular box type shown is comment. */
    
    if ((object = cast_objectIfPatchable (x))) {
        if (object_isBox (object)) {
            if (!object_isComment (object)) {
                return 0; 
            }
        }
    }
    //
    }

    return 1;
}

void gobj_visibilityChanged (t_gobj *x, t_glist *owner, int isVisible)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnVisibilityChanged) {
        if (gobj_isVisible (x, owner)) {
            (*(pd_class (x)->c_behavior->w_fnVisibilityChanged)) (x, owner, isVisible);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_isBox (t_object *x)
{
    return (pd_class (x)->c_behavior == &text_widgetBehavior)
        || (cast_glistChecked (cast_pd (x)) && !(cast_glist (x)->gl_isGraphOnParent));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
