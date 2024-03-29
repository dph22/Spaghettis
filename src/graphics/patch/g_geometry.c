
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_float glist_pixelToValueX (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeX (glist_getBounds (glist));
    t_float v = 0.0;
        
    if (glist_isWindowable (glist)) { 
        v = f; 
        if (glist_isGraphicArray (glist))  { v /= rectangle_getWidth (glist_getWindowGeometry (glist)); }
        
    } else {
        t_rectangle r;
        glist_getRectangleOnParent (glist, &r);
        v = (f - rectangle_getTopLeftX (&r));
        if (glist_isGraphicArray (glist))  { v /= rectangle_getWidth (&r); }
    }

    return (bounds_getLeft (glist_getBounds (glist)) + (range * v));
}

t_float glist_pixelToValueY (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeY (glist_getBounds (glist));
    t_float v = 0.0;
        
    if (glist_isWindowable (glist)) {
        v = f;
        if (glist_isGraphicArray (glist))  { v /= rectangle_getHeight (glist_getWindowGeometry (glist)); }
        
    } else {
        t_rectangle r;
        glist_getRectangleOnParent (glist, &r);
        v = (f - rectangle_getTopLeftY (&r));
        if (glist_isGraphicArray (glist))  { v /= rectangle_getHeight (&r); }
    }
    
    return (bounds_getTop (glist_getBounds (glist)) + (range * v));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float glist_valueToPixelX (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeX (glist_getBounds (glist));
    t_float v = (t_float)1.0;
    t_float x = 0.0;
    
    if (glist_isWindowable (glist)) {
        if (glist_isGraphicArray (glist))  { v = rectangle_getWidth (glist_getWindowGeometry (glist)); }
        
    } else {
        t_rectangle r;
        glist_getRectangleOnParent (glist, &r);
        x = rectangle_getTopLeftX (&r);
        if (glist_isGraphicArray (glist))  { v = rectangle_getWidth (&r); }
    }
    
    return (x + (v * ((f - bounds_getLeft (glist_getBounds (glist))) / range)));
}

t_float glist_valueToPixelY (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeY (glist_getBounds (glist));
    t_float v = (t_float)1.0;
    t_float x = 0.0;
    
    if (glist_isWindowable (glist)) {
        if (glist_isGraphicArray (glist))  { v = rectangle_getHeight (glist_getWindowGeometry (glist)); }
    
    } else {
        t_rectangle r;
        glist_getRectangleOnParent (glist, &r);
        x = rectangle_getTopLeftY (&r);
        if (glist_isGraphicArray (glist))  { v = rectangle_getHeight (&r); }
    }
    
    return (x + (v * ((f - bounds_getTop (glist_getBounds (glist))) / range)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float glist_getValueForOnePixelX (t_glist *glist)
{
    return (glist_pixelToValueX (glist, (t_float)1.0) - glist_pixelToValueX (glist, 0.0));
}

t_float glist_getValueForOnePixelY (t_glist *glist)
{
    return (glist_pixelToValueY (glist, (t_float)1.0) - glist_pixelToValueY (glist, 0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_getRectangleOnParent (t_glist *glist, t_rectangle *r)
{
    PD_ASSERT (glist_hasParent (glist));
    PD_ASSERT (glist_isGraphOnParent (glist));
    
    int a = glist_getPixelX (glist_getParent (glist), cast_object (glist));
    int b = glist_getPixelY (glist_getParent (glist), cast_object (glist));
    int c = a + rectangle_getWidth (glist_getGraphGeometry (glist));
    int d = b + rectangle_getHeight (glist_getGraphGeometry (glist));
    
    rectangle_set (r, a, b, c, d);
}

t_error glist_convertPixelFromParent (t_glist *glist, t_point *pt)
{
    if (!glist_isWindowable (glist)) {
    //
    t_rectangle r; glist_getRectangleOnParent (glist, &r);
    
    if (rectangle_containsPoint (&r, pt)) {
    //
    int a = rectangle_getTopLeftX (&r);
    int b = rectangle_getTopLeftY (&r);
    int c = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int d = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int m = point_getX (pt) - a + c;
    int n = point_getY (pt) - b + d;
    
    point_set (pt, m, n);
    
    return PD_ERROR_NONE;
    //
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int glist_getPixelX (t_glist *glist, t_object *x)
{
    PD_ASSERT (!glist_isGraphicArray (glist));
    
    if (glist_isWindowable (glist)) { return object_getX (x); }
    else {
    //
    int n = glist_valueToPixelX (glist, 0.0);
    n -= rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    n += object_getX (x);
    
    return n;
    //
    }
}

int glist_getPixelY (t_glist *glist, t_object *x)
{
    PD_ASSERT (!glist_isGraphicArray (glist));
    
    if (glist_isWindowable (glist)) { return object_getY (x); }
    else {
    //
    int n = glist_valueToPixelY (glist, 0.0);
    n -= rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    n += object_getY (x);
    
    return n;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
