
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

#define DRAWPOLYGON_NONE            0
#define DRAWPOLYGON_CLOSED          1
#define DRAWPOLYGON_BEZIER          2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAWPOLYGON_HANDLE          8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          drawpolygon_field;                  /* Static. */
static t_float      drawpolygon_cumulativeX;            /* Static. */
static t_float      drawpolygon_cumulativeY;            /* Static. */
static t_float      drawpolygon_startX;                 /* Static. */
static t_float      drawpolygon_startY;                 /* Static. */
static t_float      drawpolygon_stepX;                  /* Static. */
static t_float      drawpolygon_stepY;                  /* Static. */
static t_gpointer   drawpolygon_gpointer;               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *drawpolygon_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _drawpolygon {
    t_object            x_obj;                          /* Must be the first. */
    int                 x_flags;
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_colorFill;
    t_fielddescriptor   x_colorOutline;
    t_fielddescriptor   x_width;
    t_fielddescriptor   x_isVisible;
    int                 x_numberOfPoints;
    int                 x_size;
    t_fielddescriptor   *x_coordinates;
    } t_drawpolygon;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawpolygon_behaviorGetRectangle        (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void drawpolygon_behaviorVisibilityChanged   (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  drawpolygon_behaviorMouse               (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterbehavior drawpolygon_painterBehavior =
    {
        drawpolygon_behaviorGetRectangle,
        drawpolygon_behaviorVisibilityChanged,
        drawpolygon_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_float drawpolygon_getRelativeX (t_drawpolygon *x, t_gpointer *gp, t_float baseX)
{
    return baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
}

static t_float drawpolygon_getRelativeY (t_drawpolygon *x, t_gpointer *gp, t_float baseY)
{
    return baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawpolygon_release (void)
{
    gpointer_unset (&drawpolygon_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawpolygon_float (t_drawpolygon *x, t_float f)
{
    if (field_isFloatConstant (&x->x_isVisible)) {
    //
    int k = (f != 0.0);
    
    if (k != (int)(field_getFloatConstant (&x->x_isVisible))) {
    //
    paint_erase();
    field_setAsFloatConstant (&x->x_isVisible, (t_float)k);
    paint_draw();
    //
    }
    //
    } else { error_unexpected (sym_drawpolygon, &s_float); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void drawpolygon_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawpolygon *x = (t_drawpolygon *)z;

    if (gpointer_isValid (&drawpolygon_gpointer)) {
    //
    t_fielddescriptor *fd = x->x_coordinates + drawpolygon_field;
    
    drawpolygon_cumulativeX += deltaX;
    drawpolygon_cumulativeY += deltaY;
    
    t_float positionX = drawpolygon_startX + (drawpolygon_cumulativeX * drawpolygon_stepX);
    t_float positionY = drawpolygon_startY + (drawpolygon_cumulativeY * drawpolygon_stepY);
    
    gpointer_erase (&drawpolygon_gpointer);
    
    if (field_isVariable (fd + 0)) {
        gpointer_setFloatByDescriptor (&drawpolygon_gpointer, fd + 0, positionX); 
    }
    
    if (field_isVariable (fd + 1)) {
        gpointer_setFloatByDescriptor (&drawpolygon_gpointer, fd + 1, positionY);
    }
    
    gpointer_draw (&drawpolygon_gpointer);
    
    gpointer_notify (&drawpolygon_gpointer, sym_change, 0, NULL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawpolygon_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    rectangle_setNothing (r);
        
    if (visible) {
    //
    int i;
    t_glist *glist = gpointer_getOwner (gp);
    t_fielddescriptor *fd = x->x_coordinates;
        
    for (i = 0; i < x->x_size; i += 2) {
    //
    t_float relativeX = drawpolygon_getRelativeX (x, gp, baseX);
    t_float relativeY = drawpolygon_getRelativeY (x, gp, baseY);
    t_float offsetX   = gpointer_getFloatByDescriptor (gp, fd + i);
    t_float offsetY   = gpointer_getFloatByDescriptor (gp, fd + i + 1);
    t_float valueX    = relativeX + offsetX;
    t_float valueY    = relativeY + offsetY;
    int a = glist_valueToPixelX (glist, valueX);
    int b = glist_valueToPixelY (glist, valueY);
    
    rectangle_add (r, a, b);
    //
    }
    //
    }
}

static void drawpolygon_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (!isVisible || visible) {
    //
    int n = x->x_numberOfPoints;
    
    if (n > 1) {
    //
    t_word *tag    = gpointer_getElement (gp);
    t_glist *glist = gpointer_getOwner (gp);
    t_glist *view  = glist_getView (glist);
    
    if (!isVisible) { gui_vAdd ("%s.c delete %lxCURVE\n", glist_getTagAsString (view), tag); }
    else {
    //
    int width            = gpointer_getFloatByDescriptor (gp, &x->x_width);
    t_float colorFill    = gpointer_getFloatByDescriptor (gp, &x->x_colorFill);
    t_float colorOutline = gpointer_getFloatByDescriptor (gp, &x->x_colorOutline);
    t_symbol *filled     = color_toEncoded (color_withDigits ((int)colorFill));
    t_symbol *outlined   = color_toEncoded (color_withDigits ((int)colorOutline));
    
    t_fielddescriptor *fd = x->x_coordinates;
    t_heapstring *t = heapstring_new (0);
    int i;
    
    if (x->x_flags & DRAWPOLYGON_CLOSED) { 
        heapstring_addSprintf (t, "%s.c create polygon", glist_getTagAsString (view)); 
    } else {
        heapstring_addSprintf (t, "%s.c create line", glist_getTagAsString (view));
    }
    
    for (i = 0; i < x->x_size; i += 2) {
    //
    t_float relativeX = drawpolygon_getRelativeX (x, gp, baseX);
    t_float relativeY = drawpolygon_getRelativeY (x, gp, baseY);
    t_float offsetX   = gpointer_getFloatByDescriptor (gp, fd + i);
    t_float offsetY   = gpointer_getFloatByDescriptor (gp, fd + i + 1);
    t_float valueX    = relativeX + offsetX;
    t_float valueY    = relativeY + offsetY;
    int a = glist_valueToPixelX (glist, valueX);
    int b = glist_valueToPixelY (glist, valueY);
        
    heapstring_addSprintf (t, " %d %d", a, b);
    //
    }
    
    if (x->x_flags & DRAWPOLYGON_BEZIER)  { heapstring_add (t, " -smooth 1"); }
    if (x->x_flags & DRAWPOLYGON_CLOSED)  {
    //
    heapstring_addSprintf (t, " -fill %s", filled->s_name);
    heapstring_addSprintf (t, " -outline %s", outlined->s_name);
    //
    } else {
    //
    heapstring_addSprintf (t, " -fill %s", outlined->s_name);
    //
    }

    width = PD_MAX (width, 1);
    
    heapstring_addSprintf (t, " -width %d", width);
    heapstring_addSprintf (t, " -tags %lxCURVE\n", tag);
    
    gui_add (heapstring_getRaw (t));
    
    heapstring_free (t);
    //
    }
    //
    }
    //
    }
}

static int drawpolygon_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (visible) {
    //
    t_glist *glist = gpointer_getOwner (gp);
    
    int i;
    int bestField = -1;
    int bestError = PD_INT_MAX;
    
    t_fielddescriptor *fd = x->x_coordinates;

    for (i = 0; i < x->x_size; i += 2) {
    //
    if (field_isVariable (fd + i) || field_isVariable (fd + i + 1)) {
    //
    t_float relativeX = drawpolygon_getRelativeX (x, gp, baseX);
    t_float relativeY = drawpolygon_getRelativeY (x, gp, baseY);
    t_float offsetX   = gpointer_getFloatByDescriptor (gp, fd + i);
    t_float offsetY   = gpointer_getFloatByDescriptor (gp, fd + i + 1);
    t_float valueX    = relativeX + offsetX;
    t_float valueY    = relativeY + offsetY;
    int pixelX = glist_valueToPixelX (glist, valueX);
    int pixelY = glist_valueToPixelY (glist, valueY);
    int error  = (int)math_euclideanDistance (pixelX, pixelY, m->m_x, m->m_y);
    
    if (error < bestError) {
        drawpolygon_startX = offsetX;
        drawpolygon_startY = offsetY;
        bestError = error;
        bestField = i;
    }
    //
    }
    //
    }
    
    if (bestError <= DRAWPOLYGON_HANDLE) {
    
        if (m->m_clicked) {
        
            drawpolygon_stepX       = glist_getValueForOnePixelX (glist);
            drawpolygon_stepY       = glist_getValueForOnePixelY (glist);
            drawpolygon_cumulativeX = 0.0;
            drawpolygon_cumulativeY = 0.0;
            drawpolygon_field       = bestField;
            
            gpointer_setByCopy (&drawpolygon_gpointer, gp);
            
            glist_setMotion (glist, z, (t_motionfn)drawpolygon_motion, m->m_x, m->m_y);
        }
    
        return (gpointer_isScalar (gp) ? CURSOR_OVER : CURSOR_ELEMENT_1);
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *drawpolygon_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    if (field_isFloatConstant (&x->x_isVisible)) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, &s_float);
    buffer_appendFloat (b, field_getFloatConstant (&x->x_isVisible));
    
    return b;
    //
    }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *drawpolygon_new (t_symbol *s, int argc, t_atom *argv)
{
    int i;
        
    t_drawpolygon *x = (t_drawpolygon *)pd_new (drawpolygon_class);

    x->x_flags = DRAWPOLYGON_NONE;
    
    if (s == sym_filledcurve || s == sym_filledpolygon) { x->x_flags |= DRAWPOLYGON_CLOSED; }
    if (s == sym_filledcurve || s == sym_drawcurve)     { x->x_flags |= DRAWPOLYGON_BEZIER; }
    
    field_setAsFloatConstant (&x->x_positionX,    0.0);
    field_setAsFloatConstant (&x->x_positionY,    0.0);
    field_setAsFloatConstant (&x->x_colorFill,    0.0);
    field_setAsFloatConstant (&x->x_colorOutline, 0.0);
    field_setAsFloatConstant (&x->x_width,        1.0);
    field_setAsFloatConstant (&x->x_isVisible,    1.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && t == sym___dash__visible)  {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__x) {
            field_setAsFloatExtended (&x->x_positionX, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__y) {
            field_setAsFloatExtended (&x->x_positionY, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else { break; }
    }
    
    error__options (s, argc, argv);
    
    if (argc && (x->x_flags & DRAWPOLYGON_CLOSED)) { field_setAsFloat (&x->x_colorFill, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_colorOutline, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_width, argc--, argv++); }

    argc = PD_MAX (0, argc);
    
    x->x_numberOfPoints = argc / 2;
    x->x_size           = x->x_numberOfPoints * 2;
    x->x_coordinates    = (t_fielddescriptor *)PD_MEMORY_GET (x->x_size * sizeof (t_fielddescriptor));
    
    for (i = 0; i < x->x_size; i++) { field_setAsFloatExtended (x->x_coordinates + i, 1, argv + i); }

    if (argc - x->x_size > 0) { warning_unusedArguments (s, argc - x->x_size, argv + x->x_size); }
    
    return x;
}

static void drawpolygon_free (t_drawpolygon *x)
{
    PD_MEMORY_FREE (x->x_coordinates);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawpolygon_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_drawpolygon,
            (t_newmethod)drawpolygon_new,
            (t_method)drawpolygon_free,
            sizeof (t_drawpolygon),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addCreator ((t_newmethod)drawpolygon_new, sym_drawcurve,      A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawpolygon_new, sym_filledpolygon,  A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawpolygon_new, sym_filledcurve,    A_GIMME, A_NULL);
    
    class_addFloat (c, (t_method)drawpolygon_float);
        
    class_setPainterBehavior (c, &drawpolygon_painterBehavior);

    class_setDataFunction (c, drawpolygon_functionData);

    drawpolygon_class = c;
}

void drawpolygon_destroy (void)
{
    class_free (drawpolygon_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
