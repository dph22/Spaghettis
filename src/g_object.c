
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryHelp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, t_rectangle *r)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnGetRectangle) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnGetRectangle)) (x, owner, r); return;
        }
    }
    
    PD_BUG; rectangle_setNothing (r);
}

void gobj_displaced (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnDisplaced) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnDisplaced)) (x, owner, deltaX, deltaY);
        }
    }
}

void gobj_selected (t_gobj *x, t_glist *owner, int isSelected)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnSelected) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnSelected)) (x, owner, isSelected);
        }
    }
}

void gobj_activated (t_gobj *x, t_glist *owner, int isActivated)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnActivated) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnActivated)) (x, owner, isActivated);
        }
    }
}

void gobj_deleted (t_gobj *x, t_glist *owner)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnDeleted) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnDeleted)) (x, owner);
        }
    }
}

void gobj_visibilityChanged (t_gobj *x, t_glist *owner, int isVisible)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnVisibilityChanged) {
            if (gobj_isVisible (x, owner)) {
                (*(class_getWidgetBehavior (pd_class (x))->w_fnVisibilityChanged)) (x, owner, isVisible);
            }
        }
    }
}

int gobj_mouse (t_gobj *x, t_glist *owner, t_mouse *m)
{
    if (class_hasWidgetBehavior (pd_class (x))) { 
        if (class_getWidgetBehavior (pd_class (x))->w_fnMouse) {
            return (*(class_getWidgetBehavior (pd_class (x))->w_fnMouse)) (x, owner, m);
        }
    } 

    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (class_hasSaveFunction (pd_class (x))) {
        (*(class_getSaveFunction (pd_class (x)))) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int gobj_hit (t_gobj *x, t_glist *owner, int a, int b, int n, t_rectangle *r)
{
    if (gobj_isVisible (x, owner)) {
    //
    t_rectangle t1, t2;
    
    gobj_getRectangle (x, owner, &t1);
    
    rectangle_setCopy (&t2, &t1); rectangle_enlargeHeight (&t2, n);
    
    if (!rectangle_isNothing (&t2) && rectangle_containsPoint (&t2, a, b)) {
        rectangle_setCopy (r, &t1);
        return 1;
    }
    //
    }
    
    return 0;
}

/* Always true if NOT a GOP. */

int gobj_isVisible (t_gobj *x, t_glist *owner)
{
    if (glist_hasParent (owner) && !glist_isWindowable (owner)) {
    //
    t_object *object = NULL;
            
    /* Is parent visible? */
    
    if (!gobj_isVisible (cast_gobj (owner), glist_getParent (owner))) { return 0; }
    
    if (pd_class (x) == garray_class) { return 1; }                 /* Always true. */
    if (gobj_isScalar (x) && glist_isArray (owner)) { return 1; }   /* Ditto. */
    else {
    //
    {
        /* In GOP the only regular box type shown is comment. */
        
        if ((object = cast_objectIfConnectable (x))) {
            if (object_isViewedAsBox (object)) {
                if (!object_isComment (object)) {
                    return 0; 
                }
            }
        }
    }
    {
        /* Falling outside the graph rectangle? */
        
        t_rectangle r1, r2;
        gobj_getRectangle (cast_gobj (owner), glist_getParent (owner), &r1);
        gobj_getRectangle (x, owner, &r2);
        
        if (!rectangle_containsRectangle (&r1, &r2)) {
            return 0; 
        }
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

static void gobj_openHelp (const char *directory, const char *name)
{
    t_fileproperties p;
    int f = -1;
    
    if (*directory != 0) { f = file_openReadWithDirectoryAndName (directory, name, PD_HELP, &p); }
    
    if (f < 0) { 
        f = file_openReadConsideringSearchPath (main_directoryHelp->s_name, name, PD_HELP, &p); 
    }
    
    if (f < 0) { error_canNotFind (gensym (name), sym_help); }
    else {
        t_symbol *s1 = gensym (fileproperties_getName (&p));
        t_symbol *s2 = gensym (fileproperties_getDirectory (&p));
        close (f); 
        instance_patchOpen (s1, s2);
    }
}

/* First consider the sibling files of an abstraction. */
/* For an external search in its help directory if provided. */
/* Then look for in the application "help" folder. */
/* And last in the user search path. */

void gobj_help (t_gobj *y)
{
    const char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (gobj_isCanvas (y) && glist_isAbstraction (cast_glist (y))) {
        if (!(err = (buffer_getSize (object_getBuffer (cast_object (y))) < 1))) {
            atom_toString (buffer_getAtoms (object_getBuffer (cast_object (y))), name, PD_STRING);
            directory = environment_getDirectoryAsString (glist_getEnvironment (cast_glist (y)));
        }
    
    } else if (gobj_isCanvas (y) && glist_isArray (cast_glist (y))) {
        err = string_copy (name, PD_STRING, sym_garray->s_name);
        directory = "";
        
    } else {
        err = string_copy (name, PD_STRING, class_getHelpNameAsString (pd_class (y)));
        directory = class_getHelpDirectoryAsString (pd_class (y));
    }
    
    if (!err) { gobj_openHelp (directory, name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if object is really drawn as a box in the patch. */

int object_isViewedAsBox (t_object *x)
{
    return ((class_getWidgetBehavior (pd_class (x)) == &text_widgetBehavior)
        || ((gobj_isCanvas (cast_gobj (x))) && !glist_isGraphOnParent (cast_glist (x))));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
