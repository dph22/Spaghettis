
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_behaviorGetRectangle  (t_gobj *, t_glist *, t_rectangle *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_drawUpdateWindow (t_glist *glist)
{
    if (glist_isWindowable (glist) && glist_isOnScreen (glist)) { 
        canvas_map (glist, 0); 
        canvas_map (glist, 1); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawLines (t_glist *glist)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getView (glist),
                    traverser_getStartX (&t),
                    traverser_getStartY (&t),
                    traverser_getEndX (&t),
                    traverser_getEndY (&t), 
                    (outlet_isSignal (traverser_getOutlet (&t)) ? 2 : 1),
                    connection);
    //
    }
}

void canvas_updateLinesByObject (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui (".x%lx.c coords %lxLINE %d %d %d %d\n",
                    glist_getView (glist),
                    connection,
                    traverser_getStartX (&t),
                    traverser_getStartY (&t),
                    traverser_getEndX (&t),
                    traverser_getEndY (&t));
    //
    }
    //
    }
    //
    }
}

void canvas_deleteLinesByObject (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    glist_getView (glist),
                    connection);
    //
    }

    traverser_disconnect (&t);
    //
    }
    //
    }
}

void canvas_deleteLinesByInlets (t_glist *glist, t_object *o, t_inlet *inlet, t_outlet *outlet)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int m = (traverser_getSource (&t) == o && traverser_getOutlet (&t) == outlet);
    int n = (traverser_getDestination (&t) == o && traverser_getInlet (&t) == inlet);
    
    if (m || n) {
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    glist_getView (glist),
                    connection);
    //
    }
                
    traverser_disconnect (&t);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_drawBoxObject (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    char *pattern = (pd_class (o) == text_class) ? "{6 4}" : "{}";  // --
    
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -dash %s"
                        " -tags %sBORDER\n",
                        glist_getView (glist),
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
                        pattern,        /* Dashes for badly created boxes? */
                        tag);
                        
    } else {
    
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        a,
                        b,
                        c,
                        b,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b);
                        
        sys_vGui (".x%lx.c itemconfigure %sBORDER -dash %s\n",
                        glist_getView (glist),
                        tag,
                        pattern);
    }
}

static void canvas_drawBoxMessage (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        glist_getView (glist),
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
                    
    } else {
    
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void canvas_drawBoxAtom (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        glist_getView (glist),
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
                        
    } else {
    
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void canvas_drawBoxComment (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    if (glist_hasEditMode (glist)) {
    //
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d"
                        " -tags [list %sBORDER COMMENTBAR]\n",      // --
                        glist_getView (glist),
                        c,
                        b,
                        c,
                        d,
                        tag);
                        
    } else {
    
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        c,
                        b,
                        c,
                        d);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_drawInletsAndOutlets (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int i;
    int m = object_getNumberOfInlets (o);
    int n = object_getNumberOfOutlets (o);
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    for (i = 0; i < m; i++) {
    //
    int offset = a + inlet_offset ((c - a), i, m);
    
    if (create) {
    
        sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags %sINLET%d\n",
                        glist_getView (glist),
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT,
                        tag,
                        i);
                        
    } else {
    
        sys_vGui (".x%lx.c coords %sINLET%d %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        i,
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT);
    }
    //
    }
    
    for (i = 0; i < n; i++) {
    //
    int offset = a + inlet_offset ((c - a), i, n);
    
    if (create) {
    
        sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags %sOUTLET%d\n",
                        glist_getView (glist),
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d,
                        tag,
                        i);
                        
    } else {
    
        sys_vGui (".x%lx.c coords %sOUTLET%d %d %d %d %d\n",
                        glist_getView (glist),
                        tag,
                        i,
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d);
    }
    //
    }
}

void canvas_drawBox (t_glist *glist, t_object *o, char *tag, int create)
{
    t_rectangle r;
    
    text_behaviorGetRectangle (cast_gobj (o), glist, &r);

    if (object_isObject (o))            { canvas_drawBoxObject (glist, o, tag, create, &r);        }
    else if (object_isMessage (o))      { canvas_drawBoxMessage (glist, o, tag, create, &r);       }
    else if (object_isAtom (o))         { canvas_drawBoxAtom (glist, o, tag, create, &r);          }
    else if (object_isComment (o))      { canvas_drawBoxComment (glist, o, tag, create, &r);       }
    if (cast_objectIfConnectable (o))   { canvas_drawInletsAndOutlets (glist, o, tag, create, &r); }
}

static void canvas_eraseInletsAndOutlets (t_glist *glist, t_object *o, char *tag)
{
    int i;
    int m = object_getNumberOfInlets (o);
    int n = object_getNumberOfOutlets (o);
    
    for (i = 0; i < m; i++) { sys_vGui (".x%lx.c delete %sINLET%d\n",  glist_getView (glist), tag, i); }
    for (i = 0; i < n; i++) { sys_vGui (".x%lx.c delete %sOUTLET%d\n", glist_getView (glist), tag, i); }
}

void canvas_eraseBox (t_glist *glist, t_object *o, char *tag)
{
    if (!object_isComment (o) || glist_hasEditMode (glist)) {   /* Comments have borders only in edit mode. */
    //
    sys_vGui (".x%lx.c delete %sBORDER\n", glist_getView (glist), tag); 
    canvas_eraseInletsAndOutlets (glist, o, tag);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawGraphOnParentRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    int a = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int b = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int c = rectangle_getBottomRightX (glist_getGraphGeometry (glist));
    int d = rectangle_getBottomRightY (glist_getGraphGeometry (glist));
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill #%06x"
                    " -tags RECTANGLE\n",
                    glist_getView (glist),
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
                    COLOR_GOP);
    }
    //
    }
}

void canvas_updateGraphOnParentRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
        sys_vGui (".x%lx.c delete RECTANGLE\n", glist_getView (glist));
        canvas_drawGraphOnParentRectangle (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
