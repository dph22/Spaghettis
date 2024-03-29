
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

#define GLIST_DELAY_REDRAW              281.0
#define GLIST_DELAY_UNDO                107.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_updateTitle (t_glist *glist)
{
    if (glist_hasWindow (glist)) {
    //
    gui_vAdd ("::ui_patch::setTitle %s {%s} {%s} %d\n",  // --
                    glist_getTagAsString (glist),
                    environment_getDirectoryAsString (glist_getEnvironment (glist)),
                    glist_getName (glist)->s_name,
                    glist_getDirty (glist));
    //
    }
}

void glist_updateUndo (t_glist *glist)
{
    if (glist_hasWindow (glist)) {
    //
    if (glist_undoHasSeparatorAtLast (glist)) {
    
        t_undomanager *undo = glist_getUndoManager (glist);
        t_symbol *undoLabel = undomanager_getUndoLabel (undo);
        t_symbol *redoLabel = undomanager_getRedoLabel (undo);
        
        gui_vAdd ("::ui_patch::setUndoAndRedo %s {%s} {%s}\n",      // --
                    glist_getTagAsString (glist),
                    undoLabel->s_name,
                    redoLabel->s_name);
        
    } else {
        clock_delay (glist->gl_clockUndo, GLIST_DELAY_UNDO);        /* Do it later. */
    }
    //
    }
}

void glist_updateEncapsulate (t_glist *glist)
{
    int hasEncapsulate   = 0;
    int hasDeencapsulate = 0;
    
    int n = glist_objectGetNumberOfSelected (glist);
    
    hasEncapsulate = (n != 0);
    
    if (n == 1) {
    //
    t_gobj *y = selection_getObject (editor_getSelection (glist_getEditor (glist)));
    
    if (gobj_isCanvas (y) && glist_isSubpatch (cast_glist (y))) {
        hasDeencapsulate = 1;
    }
    //
    }
    
    gui_vAdd ("::ui_patch::setEncapsulateAndDeencapsulate %s %d %d\n",      // --
                    glist_getTagAsString (glist),
                    hasEncapsulate,
                    hasDeencapsulate);
}

void glist_updateWindow (t_glist *glist)
{
    if (glist_isWindowable (glist) && glist_isOnScreen (glist)) {
        glist_windowMapped (glist, 0);
        glist_windowMapped (glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* The rectangle drawn onto the parent. */

void glist_updateRectangleOnParent (t_glist *glist)
{
    if (glist_isParentOnScreen (glist)) {
    //
    int isSelected = glist_isSelected (glist);
    int hasWindow  = glist_hasWindow (glist);
    int color      = hasWindow ? COLOR_OPENED : (isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    
    t_glist *view  = glist_getView (glist_getParent (glist));
    
    gui_vAdd ("%s.c itemconfigure %lxGRAPH -fill #%06x\n",
                    glist_getTagAsString (view),
                    glist,
                    color);
    //
    }
}

/* The dashed rectangle drawn to show the area. */

void glist_updateRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isGraphicArray (glist)) {
    //
    gui_vAdd ("%s.c delete RECTANGLE\n", glist_getTagAsString (glist));
    
    glist_drawRectangle (glist);
    //
    }
    //
    }
}

void glist_updateLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c coords LASSO %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    drag_getStartX (editor_getDrag (glist_getEditor (glist))),
                    drag_getStartY (editor_getDrag (glist_getEditor (glist))),
                    a,
                    b);
    //
    }
    //
    }
}

void glist_updateTemporary (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c coords TEMPORARY %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    drag_getStartX (editor_getDrag (glist_getEditor (glist))),
                    drag_getStartY (editor_getDrag (glist_getEditor (glist))),
                    a,
                    b);
    //
    }
    //
    }
}

void glist_updateLineSelected (t_glist *glist, int isSelected)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    int color = (isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    
    gui_vAdd ("%s.c itemconfigure %lxLINE -fill #%06x\n",
                    glist_getTagAsString (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)), 
                    color);
    //
    }
}

void glist_updateLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c coords %lxLINE %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c));

    gui_vAdd ("%s.c itemconfigure %lxLINE -width %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_isSignal (c) ? 2 : 1);
    //
    }
    //
    }
}

void glist_updateLinesForObject (t_glist *glist, t_object *o)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
        glist_updateLine (glist, traverser_getCord (&t));
    }
    //
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* For compatibility several names can be rendered. */

static void glist_drawArrayName (t_glist *glist)
{
    if (glist_isGraphicArray (glist)) {
    //
    int isSelected = glist_isSelected (glist);
    int color      = isSelected ? COLOR_SELECTED : COLOR_NORMAL;
    
    t_glist *view  = glist_getView (glist_getParent (glist));
    
    t_rectangle r;

    glist_getRectangleOnParent (glist, &r);

    {
        int a = rectangle_getTopLeftX (&r);
        int b = rectangle_getTopLeftY (&r);
        int k = glist_getFontSize (view);
        int h = (int)font_getHeight (glist_getFontSize (view));
        int i = 0;
        
        /* Legacy patches can contains multiple arrays. */
        
        t_gobj *y = NULL;
         
        for (y = glist->gl_graphics; y; y = y->g_next) {
        //
        if (gobj_isGraphicArray (y)) {
        //
        if (garray_isNameShown ((t_garray *)y)) {
        //
        gui_vAdd ("%s.c create text %d %d -text {%s}"   // --
                        " -anchor nw"
                        " -font [::getFont %d]"         // --
                        " -fill #%06x"
                        " -tags %lxGRAPH\n",
                        glist_getTagAsString (view),
                        a,
                        b - (++i) * h,
                        garray_getName ((t_garray *)y)->s_name,
                        k,
                        color,
                        glist);
        //
        }
        //
        }
        //
        }
    }
    //
    }
}

void glist_drawRectangleOnParent (t_glist *glist)
{
    int isSelected   = glist_isSelected (glist);
    int hasWindow    = glist_hasWindow (glist);
    int color        = hasWindow ? COLOR_OPENED : (isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    const char *type = hasWindow ? "polygon" : "line";
    
    t_glist *view = glist_getView (glist_getParent (glist));
    
    t_rectangle r;
    
    glist_getRectangleOnParent (glist, &r);
    
    {
        int a = rectangle_getTopLeftX (&r);
        int b = rectangle_getTopLeftY (&r);
        int c = rectangle_getBottomRightX (&r);
        int d = rectangle_getBottomRightY (&r);
        
        gui_vAdd ("%s.c create %s %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %lxGRAPH\n",
                        glist_getTagAsString (view),
                        type,
                        a,
                        b,
                        a,
                        d,
                        c,
                        d,
                        c,
                        b,
                        a,
                        b,
                        color,
                        glist);
    }
    
    if (!hasWindow) { glist_drawArrayName (glist); }
}

void glist_drawRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isGraphicArray (glist)) {
    //
    int a = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int b = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int c = rectangle_getBottomRightX (glist_getGraphGeometry (glist));
    int d = rectangle_getBottomRightY (glist_getGraphGeometry (glist));
    
    int selected = editor_hasSelectedGraph (glist_getEditor (glist));
    
    gui_vAdd ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill #%06x"
                    " -tags RECTANGLE\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    c,
                    b,
                    c,
                    d,
                    a,
                    d,
                    a,
                    b, 
                    selected ? COLOR_SELECTED : COLOR_GRAPH);
    }
    //
    }
}

void glist_drawLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c create rectangle %d %d %d %d -tags LASSO\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b);
    //
    }
    //
    }
}

void glist_drawTemporary (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    int isSignal = (editor_getAction (glist_getEditor (glist)) == ACTION_SIGNAL);
    
    gui_vAdd ("%s.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b,
                    isSignal ? 2 : 1);
    //
    }
    //
    }
}

void glist_drawLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getTagAsString (glist),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c),
                    cord_isSignal (c) ? 2 : 1,
                    cord_getConnection (c));
    //
    }
    //
    }
}

void glist_drawAllLines (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) { glist_drawLine (glist, traverser_getCord (&t)); }
    //
    }
    //
    }
}

static void glist_drawAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y)) && object_isComment (o)) {
            box_draw (box_fetch (glist, o));
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_eraseRectangleOnParent (t_glist *glist)
{
    t_glist *view = glist_getView (glist_getParent (glist));
    
    gui_vAdd ("%s.c delete %lxGRAPH\n", glist_getTagAsString (view), glist);
}

void glist_eraseLasso (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c delete LASSO\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseTemporary (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c delete TEMPORARY\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c delete %lxLINE\n", glist_getTagAsString (glist), cord_getConnection (c));
    //
    }
    //
    }
}

static void glist_eraseAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("%s.c delete COMMENTBAR\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_redrawTemporary (t_glist *glist)
{
    t_drag *drag = editor_getDrag (glist_getEditor (glist));
    
    glist_drawTemporary (glist, drag_getStartX (drag), drag_getStartY (drag));
    glist_updateTemporary (glist, drag_getEndX (drag), drag_getEndY (drag));
}

static void glist_redrawLasso (t_glist *glist)
{
    t_drag *drag = editor_getDrag (glist_getEditor (glist));
    
    glist_drawLasso (glist, drag_getStartX (drag), drag_getStartY (drag));
    glist_updateLasso (glist, drag_getEndX (drag), drag_getEndY (drag));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_windowEdit (t_glist *glist, int isEditMode)
{
    int hasEditMode = 0;
    
    if (glist_isEditable (glist)) {
    //
    if (isEditMode != glist_hasEditMode (glist)) {
    //
    glist_setEditMode (glist, isEditMode);
    
    if (isEditMode) { glist_drawAllCommentBars (glist); }
    else {
        glist_deselectAll (glist); glist_eraseAllCommentBars (glist);
    }
    //
    }
    
    hasEditMode = glist_hasEditMode (glist);
    //
    }
    
    if (glist_isOnScreen (glist)) {
    //
    gui_vAdd ("::ui_patch::setEditMode %s %d\n", glist_getTagAsString (glist), hasEditMode);
    
    if (hasEditMode) { gui_vAdd ("::ui_patch::askForUpdateMenu %s\n", glist_getTagAsString (glist)); }
    //
    }
    
    instance_openedWindowInEditModeReset();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_windowMappedEraseContent (t_glist *glist)
{
    gui_vAdd ("%s.c delete all\n", glist_getTagAsString (glist)); 
    
    glist_setMapped (glist, 0);
}

static void glist_windowMappedDrawContent (t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    int action = editor_getAction (glist_getEditor (glist));
    
    glist_setMapped (glist, 1);
    
    for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
        gobj_selected (selection_getObject (s), glist, 1);
    }

    glist_drawAllLines (glist);
    glist_drawRectangle (glist);
    
    if (action == ACTION_REGION) { glist_redrawLasso (glist);     }
    if (action == ACTION_LINE)   { glist_redrawTemporary (glist); }
    if (action == ACTION_SIGNAL) { glist_redrawTemporary (glist); }
}

/* When a window is put or removed from screen. */
/* It can result from normal, withdrawn or iconic states. */

void glist_windowMapped (t_glist *glist, int isMapped)
{
    if (isMapped != glist_isOnScreen (glist)) {
    //
    PD_ASSERT (glist_hasWindow (glist));
    
    glist_cancelEditingBox (glist);
    
    if (isMapped) { glist_windowMappedDrawContent (glist); }
    else {
        glist_windowMappedEraseContent (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_windowOpen (t_glist *glist)
{
    /* If a GOP is opened in its own window content needs to be redrawn on parent. */
    /* Temporary force the window state in order to properly drawn it. */
    
    if (glist_isOnScreen (glist) && !glist_isWindowable (glist)) {      

        t_glist *t = glist_getParent (glist);
        
        if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
        glist_setWindow (glist, 1); 
        if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
        glist_setWindow (glist, 0);
    }
    
    /* Create or deiconify the window. */
    
    if (glist_hasWindow (glist)) { gui_vAdd ("::bringToFront %s\n", glist_getTagAsString (glist)); }
    else {
    //
    t_rectangle *r = glist_getWindowGeometry (glist);
    int isEditable = glist_isEditable (glist);
    int isEditMode = isEditable && glist_hasEditMode (glist);
    
    gui_vAdd ("::ui_patch::create %s %d %d %d %d %d %d\n",    // --
                    glist_getTagAsString (glist),
                    rectangle_getWidth (r),
                    rectangle_getHeight (r),
                    rectangle_getTopLeftX (r),
                    rectangle_getTopLeftY (r),
                    isEditMode,
                    isEditable);
                    
    glist_setWindow (glist, 1); glist_updateTitle (glist);
    //
    }
    
    instance_openedWindowInEditModeReset();
}

void glist_windowClose (t_glist *glist)
{
    /* Erase everything first and destroy the window. */
    
    glist_deselectAll (glist);
    
    if (glist_isOnScreen (glist)) { glist_windowMapped (glist, 0); }

    gui_vAdd ("destroy %s\n", glist_getTagAsString (glist));
    
    /* If it is a GOP opened in its own window it needs to be redrawn on parent. */
    /* Note that as above the window state influence the way it is rendered. */
    
    if (glist_isGraphOnParent (glist) && glist_hasParent (glist)) {

        t_glist *t = glist_getParent (glist);
        
        if (!glist_isDeleting (t)) {
            if (glist_isOnScreen (t))  { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
            glist_setWindow (glist, 0);
            if (glist_isOnScreen (t))  { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
        }
    }
    
    glist_setWindow (glist, 0);
    
    instance_openedWindowInEditModeReset();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Used to track modifications of scalars. */

t_id glist_getChangeTag (t_glist *glist)
{
    return glist->gl_redrawn;
}

void glist_setChangeTag (t_glist *glist)
{
    glist->gl_redrawn = utils_unique();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* To maintain z order consistency, redraw everything is needed. */

void glist_redrawRequired (t_glist *glist)
{
    clock_delay (glist->gl_clockRedraw, GLIST_DELAY_REDRAW);
}

void glist_redraw (t_glist *glist)
{
    glist_setChangeTag (glist);
    
    clock_unset (glist->gl_clockRedraw);
    
    if (glist_isOnScreen (glist)) {
    //
    if (glist_hasWindow (glist))  { glist_updateWindow (glist); }
    else {
        PD_ASSERT (glist_isParentOnScreen (glist));
        glist_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
        glist_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
