
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GARRAY_FLAG_SAVE        (1)
#define GARRAY_FLAG_PLOT        (2 + 4)
#define GARRAY_FLAG_HIDE        (8)
#define GARRAY_FLAG_INHIBIT     (16)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_behaviorGetRectangle         (t_gobj *, t_glist *, t_rectangle *);
static void garray_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void garray_behaviorSelected             (t_gobj *, t_glist *, int);
static void garray_behaviorActivated            (t_gobj *, t_glist *, int);
static void garray_behaviorDeleted              (t_gobj *, t_glist *);
static void garray_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  garray_behaviorMouse                (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_dismiss (t_garray *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void array_copyGraphicArray (t_word *, t_word *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *garray_class;                                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _garray {
    t_gobj      x_gobj;
    t_scalar    *x_scalar;
    t_glist     *x_owner;
    t_symbol    *x_unexpandedName;
    t_symbol    *x_name;
    t_id        x_redrawn;
    int         x_isUsedInDSP;
    int         x_saveWithParent;
    int         x_hideName;
    int         x_inhibit;
    int         x_dismissed;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior garray_widgetBehavior =             /* Shared. */
    {
        garray_behaviorGetRectangle,
        garray_behaviorDisplaced,
        garray_behaviorSelected,
        garray_behaviorActivated,
        garray_behaviorDeleted,
        garray_behaviorVisibilityChanged,
        garray_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Assume that a pointer to a union object points to each of its members. */

/* < http://c0x.coding-guidelines.com/6.7.2.1.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define GARRAY_AT(n)    (array_getElements (array) + (int)n)

// array_getElementAtIndex (array, (int)n)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Create invisible, built-in canvases to supply templates for floats and float-arrays. */

void garray_initialize (void)
{
    static const char *floatTemplate =              /* Static. */
        "#N canvas 500 75 300 100 12;\n"
        "#X obj 24 24 struct float float y;\n"
        "#X coords 0 0 1 1 250 175 0 0 0;\n";

    static const char *floatArrayTemplate =         /* Static. */
        "#N canvas 500 200 600 100 12;\n"
        "#X obj 24 24 struct float-array array z float float style float linewidth float color;\n"
        "#X f 74;"
        "#X obj 24 60 plot z color linewidth 0 0 1 style;\n"
        "#X coords 0 0 1 1 250 175 0 0 0;\n";

    instance_loadBuiltIn (sym__floattemplate, floatTemplate);
    instance_loadBuiltIn (sym__floatarraytemplate, floatArrayTemplate);
}

#if PD_WITH_DEBUG
    
static void garray_check (t_garray *x)
{
    t_array *array = NULL; t_template *tmpl = NULL;
    
    // struct float-array array z float
        
    tmpl = template_findByIdentifier (scalar_getTemplateIdentifier (x->x_scalar));
    
    PD_ASSERT (tmpl);
    PD_ASSERT (template_fieldIsArrayAndValid (tmpl, sym_z));
    
    array = scalar_getArray (x->x_scalar, sym_z);
    
    // struct float float y
     
    tmpl = template_findByIdentifier (array_getTemplateIdentifier (array));
    
    PD_ASSERT (tmpl);
    PD_ASSERT (template_fieldIsFloat (tmpl, sym_y));
    PD_ASSERT (template_getSize (tmpl) == 1);                   /* Just one field. */
    PD_ASSERT (template_getIndexOfField (tmpl, sym_y) == 0);    /* Just one field. */
    PD_ASSERT (array_getElementSize (array) == 1);              /* Just one field. */
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_drawJob (t_gobj *z, t_glist *glist)
{
    t_garray *x = (t_garray *)z;
    
    PD_ASSERT (glist == x->x_owner);
    
    if (glist_isOnScreen (x->x_owner) && gobj_isViewable (z, x->x_owner)) {
    //
    garray_behaviorVisibilityChanged (z, x->x_owner, 0); 
    garray_behaviorVisibilityChanged (z, x->x_owner, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_updateGraphWindow (t_garray *x)
{
    glist_redraw (x->x_owner);
}

static void garray_updateGraphName (t_garray *x)
{
    glist_setName (x->x_owner, x->x_name);
}

static void garray_updateGraphRange (t_garray *x, t_float up, t_float down)
{
    if (!glist_isLoading (x->x_owner)) {    /* Would be overwrite by coords method. */
    //
    t_float a = bounds_getLeft (glist_getBounds (x->x_owner));
    t_float b = up;
    t_float c = bounds_getRight (glist_getBounds (x->x_owner));
    t_float d = down;
    
    t_bounds bounds; 
    
    t_error err = bounds_set (&bounds, a, b, c, d);
    
    PD_ASSERT (glist_isGraphicArray (x->x_owner));
    PD_ASSERT (!err);
    
    if (!err) { glist_setBounds (x->x_owner, &bounds); }
    //
    }
}

static void garray_updateGraphGeometry (t_garray *x, int width, int height)
{
    if (!glist_isLoading (x->x_owner)) {    /* Would be overwrite by coords method. */
    //
    int w = rectangle_getWidth (glist_getGraphGeometry (x->x_owner));
    int h = rectangle_getHeight (glist_getGraphGeometry (x->x_owner));
    
    PD_ASSERT (glist_isGraphicArray (x->x_owner));
    
    if (w != width || h != height) {
    //
    PD_ASSERT (width > 0);
    PD_ASSERT (height > 0);
    
    rectangle_setWidth (glist_getGraphGeometry (x->x_owner), width);
    rectangle_setHeight (glist_getGraphGeometry (x->x_owner), height);
    
    /* Required if it has its own window opened. */
    
    if (glist_isParentOnScreen (x->x_owner)) { glist_redrawRequired (glist_getParent (x->x_owner)); }
    //
    }
    //
    }
}

static void garray_updateGraphSize (t_garray *x, int size, int style)
{
    if (!glist_isLoading (x->x_owner)) {    /* Would be overwrite by coords method. */
    //
    t_float a = 0.0;
    t_float b = bounds_getTop (glist_getBounds (x->x_owner));
    t_float c = (t_float)((style == PLOT_POINTS || size == 1) ? size : size - 1);
    t_float d = bounds_getBottom (glist_getBounds (x->x_owner));
    
    t_bounds bounds; 
    
    t_error err = bounds_set (&bounds, a, b, c, d);
    
    PD_ASSERT (glist_isGraphicArray (x->x_owner));
    PD_ASSERT (!err);
    
    if (!err) { glist_setBounds (x->x_owner, &bounds); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_setWithSineWavesProceed (t_garray *x,
    int numberOfPoints,
    int numberOfSineWaves,
    t_atom *amplitudeOfSineWaves,
    int isSine)
{
    double phase, phaseIncrement;
    int i;
    t_array *array = garray_getArray (x);
    
    numberOfPoints = (numberOfPoints <= 0) ? 512 : numberOfPoints;
    numberOfPoints = PD_MIN (numberOfPoints, 1 << 30);
    
    if (!PD_IS_POWER_2 (numberOfPoints)) { numberOfPoints = (int)PD_NEXT_POWER_2 (numberOfPoints); }
    
    garray_resize (x, (t_float)(numberOfPoints + 3));
    
    phaseIncrement = PD_TWO_PI / numberOfPoints;
    
    for (i = 0, phase = -phaseIncrement; i < array_getSize (array); i++, phase += phaseIncrement) {
    //
    int j;
    double fj;
    double sum = 0.0;
    
    if (isSine) {
        for (j = 0, fj = phase; j < numberOfSineWaves; j++, fj += phase) { 
            sum += (double)atom_getFloatAtIndex (j, numberOfSineWaves, amplitudeOfSineWaves) * sin (fj);
        }
    } else {
        for (j = 0, fj = 0; j < numberOfSineWaves; j++, fj += phase) {
            sum += (double)atom_getFloatAtIndex (j, numberOfSineWaves, amplitudeOfSineWaves) * cos (fj);
        }
    }
    
    w_setFloat (GARRAY_AT (i), (t_float)sum);
    //
    }
    
    garray_redraw (x);
}

static void garray_setWithSineWaves (t_garray *x, t_symbol *s, int argc, t_atom *argv, int isSine)
{    
    if (argc > 1) {
    //
    int n = atom_getFloatAtIndex (0, argc, argv);
    
    argv++;
    argc--;
    
    garray_setWithSineWavesProceed (x, n, argc, argv, isSine);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *garray_getArray (t_garray *x)
{
    #if PD_WITH_DEBUG
    
    static int once = 0; if (!once) { garray_check (x); once = 1; }     /* Static. */
    
    #endif
    
    return scalar_getArray (x->x_scalar, sym_z);
}

int garray_getSize (t_garray *x)
{
    return array_getSize (garray_getArray (x));
}

int garray_getData (t_garray *x, int *size, t_word **w)
{
    t_array *array = garray_getArray (x);
    
    *size = array_getSize (array); *w = array_getElements (array);
    
    return 1; 
}

void garray_setDataAtIndex (t_garray *x, int i, t_float f)
{
    t_array *array = garray_getArray (x);
    
    int size = array_getSize (array);
    int n = PD_CLAMP (i, 0, size - 1);
    
    w_setFloat (GARRAY_AT (n), f);
}

t_float garray_getDataAtIndex (t_garray *x, int i)
{
    t_array *array = garray_getArray (x);
    
    int size = array_getSize (array);
    int n = PD_CLAMP (i, 0, size - 1);
    t_float f = w_getFloat (GARRAY_AT (n));
    
    return f;
}

void garray_setDataFromIndex (t_garray *x, int i, t_float f)
{
    t_array *array = garray_getArray (x);
    
    int n, size = array_getSize (array);
    
    for (n = PD_CLAMP (i, 0, size - 1); n < size; n++) { w_setFloat (GARRAY_AT (n), f); }
}

t_float garray_getAmplitude (t_garray *x)
{
    t_array *array = garray_getArray (x);
    
    int i, size = array_getSize (array);
    
    t_float f = 0.0;
    
    for (i = 0; i < size; i++) { t_float t = w_getFloat (GARRAY_AT (i)); f = PD_MAX (f, PD_ABS (t)); }

    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *garray_getName (t_garray *x)
{
    return x->x_name;
}

t_symbol *garray_getUnexpandedName (t_garray *x)
{
    return x ? x->x_unexpandedName : sym_Patch;     /* Could be NULL in legacy patches. */
}

t_glist *garray_getOwner (t_garray *x)
{
    return x->x_owner;
}

t_scalar *garray_getScalar (t_garray *x)
{
    return x->x_scalar;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void garray_setSaveWithParent (t_garray *x, int savedWithParent)
{
    x->x_saveWithParent = (savedWithParent != 0);
}

void garray_setHideName (t_garray *x, int hideName)
{
    x->x_hideName = (hideName != 0);
}

void garray_setInhibit (t_garray *x, int inhibit)
{
    x->x_inhibit = (inhibit != 0);
    
    if (x->x_inhibit) { scalar_disable (x->x_scalar); }
    else {
        scalar_enable (x->x_scalar);
    }
}

void garray_setAsUsedInDSP (t_garray *x, int usedInDSP)
{
    x->x_isUsedInDSP = (usedInDSP != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int garray_isUsedInDSP (t_garray *x)
{
    return (x->x_isUsedInDSP != 0);
}

int garray_isNameShown (t_garray *x)
{
    return (x->x_hideName == 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void garray_redraw (t_garray *x)
{
    garray_setChangeTag (x);
    
    gui_jobAdd ((void *)x, x->x_owner, garray_drawJob);
}

void garray_setChangeTag (t_garray *x)
{
    x->x_redrawn = utils_unique();
}

t_id garray_getChangeTag (t_garray *x)
{
    return x->x_redrawn;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_list (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { 
    //
    t_array *array = garray_getArray (x);
    
    int i, j = atom_getFloat (argv);
        
    argc--;
    argv++;

    if (j < 0) { argc += j; argv -= j; j = 0; }
    
    if (j + argc > array_getSize (array)) { argc = array_getSize (array) - j; }
    
    if (argc > 0) {
    //
    for (i = 0; i < argc; i++) { w_setFloat (GARRAY_AT (i + j), atom_getFloat (argv + i)); }
    
    garray_redraw (x);
    //
    }
    //
    }
}

static void garray_constant (t_garray *x, t_float f)
{
    garray_setDataFromIndex (x, 0, f);
    garray_redraw (x);
}

static void garray_normalize (t_garray *x, t_float f)
{
    int i;
    double maximum  = 0.0;
    t_array *array = garray_getArray (x);

    if (f <= 0.0) { f = (t_float)1.0; }

    for (i = 0; i < array_getSize (array); i++) {
        double t = w_getFloat (GARRAY_AT (i));
        if (PD_ABS (t) > maximum) { maximum = PD_ABS (t); }
    }
    
    if (maximum > 0.0) {
        double k = f / maximum;
        for (i = 0; i < array_getSize (array); i++) {
            w_setFloat (GARRAY_AT (i), w_getFloat (GARRAY_AT (i)) * (t_float)k);
        }
    }
    
    garray_redraw (x);
}

static void garray_sinesum (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    garray_setWithSineWaves (x, s, argc, argv, 1);
}

static void garray_cosinesum (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    garray_setWithSineWaves (x, s, argc, argv, 0);
}

static void garray_rename (t_garray *x, t_symbol *s)
{
    if (s != x->x_unexpandedName) {
    //
    t_symbol *expanded = dollar_expandSymbol (s, x->x_owner);
    
    if (symbol_getThingByClass (expanded, garray_class)) { error_alreadyExists (expanded); }
    else {
    //
    pd_unbind (cast_pd (x), x->x_name);
    x->x_unexpandedName = s;
    x->x_name = expanded;
    pd_bind (cast_pd (x), x->x_name);
    garray_redraw (x);
    garray_updateGraphName (x);
    garray_updateGraphWindow (x);
    dsp_update();
    //
    }
    //
    }
}

static void garray_read (t_garray *x, t_symbol *name)
{
    t_error err = PD_ERROR_NONE;
    
    t_fileproperties p; fileproperties_init (&p);
    
    int f = glist_fileOpen (glist_getView (x->x_owner), name->s_name, "", &p);
    
    if (!(err |= (f < 0))) {
    //
    FILE *file = fdopen (f, "r");
    
    if (!(err |= (file == NULL))) {

        int i;
        t_array *array = garray_getArray (x);

        for (i = 0; i < array_getSize (array); i++) {
        //
        double v = 0.0;
        if (!fscanf (file, "%lf", &v)) { break; }
        else {
            w_setFloat (GARRAY_AT (i), (t_float)v);
        }
        //
        }
        
        while (i < array_getSize (array)) { w_setFloat (GARRAY_AT (i), 0.0); i++; }
        
        fclose (file);      /* < http://stackoverflow.com/a/13691168 > */
        
        garray_redraw (x);
    }
    //
    }
    
    if (err) { error_canNotOpen (name); }
}

static void garray_write (t_garray *x, t_symbol *name)
{
    char t[PD_STRING] = { 0 };
    const char *s = environment_getDirectoryAsString (glist_getEnvironment (glist_getView (x->x_owner)));
    t_error err = path_withDirectoryAndName (t, PD_STRING, s, name->s_name);

    if (!err) {
    //
    int f = file_openWrite (t);
    
    if (!(err |= (f < 0))) {
    //
    FILE *file = fdopen (f, "w");
    
    if (!(err |= (file == NULL))) {
    
        int i;
        t_array *array = garray_getArray (x);
        
        for (i = 0; i < array_getSize (array); i++) {
            if (fprintf (file, "%.9g\n", w_getFloat (GARRAY_AT (i))) < 1) {
                PD_BUG; break;
            }
        }
        
        if (fclose (file) != 0) { PD_BUG; }
    }
    //
    }
    //
    }
    
    if (err) { error_canNotCreate (name); }
}

void garray_resize (t_garray *x, t_float f)
{
    t_array *array = garray_getArray (x);
    int n = PD_MAX (1, (int)f);
    int style = scalar_getFloat (x->x_scalar, sym_style);
    int dspState = 0;
    int dspSuspended = 0;
    
    PD_ASSERT (sys_isMainThread());
    
    if (garray_isUsedInDSP (x)) { dspState = dsp_suspend(); dspSuspended = 1; }
    
    garray_updateGraphSize (x, n, style);
    garray_updateGraphWindow (x);
    array_resizeAndRedraw (array, x->x_owner, n);
    
    if (dspSuspended) { dsp_resume (dspState); }
}

static void garray_range (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float down = atom_getFloatAtIndex (0, argc, argv);
    t_float up   = atom_getFloatAtIndex (1, argc, argv);
    
    garray_updateGraphRange (x, up, down);
    garray_updateGraphWindow (x);
    garray_redraw (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_garray *x = (t_garray *)z;
    
    gobj_getRectangle (cast_gobj (x->x_scalar), glist, r);
}

static void garray_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
}

static void garray_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
}

static void garray_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
}

static void garray_behaviorDeleted (t_gobj *z, t_glist *glist)
{
}

static void garray_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_garray *x = (t_garray *)z;
    
    gobj_visibilityChanged (cast_gobj (x->x_scalar), glist, isVisible);
    
    if (!isVisible) { gui_jobRemove ((void *)z); }
}

static int garray_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_garray *x = (t_garray *)z;

    if (m->m_clicked) { gobj_mouse (cast_gobj (x->x_scalar), glist, m); }

    return CURSOR_CLICK;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void garray_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_garray *x = (t_garray *)z;
    int style = scalar_getFloat (x->x_scalar, sym_style);    
    t_array *array = garray_getArray (x);
    int n = array_getSize (array);
    
    // GARRAY_FLAG_SAVE
    // GARRAY_FLAG_PLOT
    // GARRAY_FLAG_HIDE
    // GARRAY_FLAG_INHIBIT

    int t = x->x_saveWithParent + (2 * style) + (8 * x->x_hideName) + (16 * x->x_inhibit);

    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_array);
    buffer_appendSymbol (b, x->x_unexpandedName);
    buffer_appendFloat (b,  n);
    buffer_appendSymbol (b, &s_float);
    buffer_appendFloat (b,  t);
    buffer_appendSemicolon (b);
    
    gobj_saveUniques (z, b, flags);
}

static t_buffer *garray_functionData (t_gobj *z, int flags)
{
    t_garray *x = (t_garray *)z;

    if (SAVED_DEEP (flags) || x->x_saveWithParent) {
    //
    t_buffer *b = buffer_new();
    t_array *array = garray_getArray (x);
    int i, n = array_getSize (array);
    
    buffer_appendFloat (b, 0);
    for (i = 0; i < n; i++) { buffer_appendFloat (b, w_getFloat (GARRAY_AT (i))); }
    
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

/* Fake dialog message from interpreter. */

static void garray_functionUndo (t_gobj *z, t_buffer *b)
{
    t_garray *x = (t_garray *)z;
    int style = scalar_getFloat (x->x_scalar, sym_style);
    t_array *array = garray_getArray (x);
    t_bounds *bounds = glist_getBounds (x->x_owner);
    
    buffer_appendSymbol (b, sym__arraydialog);
    buffer_appendSymbol (b, symbol_dollarToHash (x->x_unexpandedName));
    buffer_appendFloat (b,  array_getSize (array));
    buffer_appendFloat (b,  rectangle_getWidth (glist_getGraphGeometry (x->x_owner)));
    buffer_appendFloat (b,  rectangle_getHeight (glist_getGraphGeometry (x->x_owner)));
    buffer_appendFloat (b,  bounds_getTop (bounds));
    buffer_appendFloat (b,  bounds_getBottom (bounds));
    buffer_appendFloat (b,  x->x_saveWithParent);
    buffer_appendFloat (b,  PD_CLAMP (style, PLOT_POLYGONS, PLOT_CURVES));
    buffer_appendFloat (b,  x->x_hideName);
    buffer_appendFloat (b,  x->x_inhibit);
}

static void garray_functionDismiss (t_gobj *z)
{
    garray_dismiss ((t_garray *)z);
}

void garray_functionProperties (t_garray *x)
{
    char t[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    int style = scalar_getFloat (x->x_scalar, sym_style);
    t_array *array = garray_getArray (x);
    t_bounds *bounds = glist_getBounds (x->x_owner);

    PD_ASSERT (glist_isGraphicArray (x->x_owner));
    
    err |= string_sprintf (t, PD_STRING,
                "::ui_array::show %%s %s %d %d %d %.9g %.9g %d %d %d %d 0\n",
                symbol_dollarToHash (x->x_unexpandedName)->s_name,
                array_getSize (array),
                rectangle_getWidth (glist_getGraphGeometry (x->x_owner)),
                rectangle_getHeight (glist_getGraphGeometry (x->x_owner)),
                bounds_getTop (bounds),
                bounds_getBottom (bounds),
                x->x_saveWithParent,
                PD_CLAMP (style, PLOT_POLYGONS, PLOT_CURVES),
                x->x_hideName,
                x->x_inhibit);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void garray_fromDialog (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    t_glist *parent = glist_getParent (x->x_owner);
    int undoable    = glist_undoIsOk (parent);
    
    PD_ASSERT (argc == 11);
    
    t_symbol *t1 = x->x_name;
    int t2       = x->x_saveWithParent;
    int t3       = x->x_hideName;
    int t4       = x->x_inhibit;
    int t5       = garray_getSize (x);
    t_float t6   = scalar_getFloat (x->x_scalar, sym_style);
    t_bounds t7;
    t_rectangle t8;

    t_undosnippet *s1 = NULL;
    t_undosnippet *s2 = NULL;
    
    if (undoable) { s1 = undosnippet_newProperties (cast_gobj (x), x->x_owner); }
    
    bounds_setCopy (&t7, glist_getBounds (x->x_owner));
    rectangle_setCopy (&t8, glist_getGraphGeometry (x->x_owner));
    
    {
    //
    t_symbol *name = symbol_hashToDollar (atom_getSymbolAtIndex (0, argc, argv));
    int size       = (int)atom_getFloatAtIndex (1, argc, argv);
    int width      = (int)atom_getFloatAtIndex (2, argc, argv);
    int height     = (int)atom_getFloatAtIndex (3, argc, argv);
    t_float up     = atom_getFloatAtIndex (4, argc, argv);
    t_float down   = atom_getFloatAtIndex (5, argc, argv);
    int save       = (int)atom_getFloatAtIndex (6, argc, argv);
    int style      = (int)atom_getFloatAtIndex (7, argc, argv);
    int hide       = (int)atom_getFloatAtIndex (8, argc, argv);
    int inhibit    = (int)atom_getFloatAtIndex (9, argc, argv);
    // int isMenu  = (int)atom_getFloatAtIndex (10, argc, argv);
    
    PD_ASSERT (size > 0);
    
    t_array *array = garray_getArray (x);
    
    garray_rename (x, name);

    if (size == 1) { style = PLOT_POINTS; }
    
    scalar_setFloat (x->x_scalar, sym_style, (t_float)style);
    
    if (size != array_getSize (array)) { garray_resize (x, (t_float)size); }
    
    garray_setHideName (x, hide);
    garray_setInhibit (x, inhibit);
    garray_updateGraphSize (x, size, style);
    garray_updateGraphRange (x, up, down);
    garray_updateGraphGeometry (x, width, height);
    garray_updateGraphWindow (x);
    garray_setSaveWithParent (x, save);
    garray_redraw (x);
    //
    }
    
    if (undoable) { s2 = undosnippet_newProperties (cast_gobj (x), x->x_owner); }
    
    isDirty |= (t1 != x->x_name);
    isDirty |= (t2 != x->x_saveWithParent);
    isDirty |= (t3 != x->x_hideName);
    isDirty |= (t4 != x->x_inhibit);
    isDirty |= (t5 != garray_getSize (x));
    isDirty |= (t6 != scalar_getFloat (x->x_scalar, sym_style));
    isDirty |= !bounds_areEquals (&t7, glist_getBounds (x->x_owner));
    isDirty |= !rectangle_areEquals (&t8, glist_getGraphGeometry (x->x_owner));
    
    if (isDirty) { glist_setDirty (parent, 1); }
    
    if (undoable) {
    //
    if (isDirty) {
        glist_undoAppend (parent, undoproperties_new (cast_gobj (x), s1, s2));
        glist_undoAppendSeparator (parent);
        
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

/* For now data are fetch during encapsulation only if properties of array have NOT changed. */

static void garray_restoreProceed (t_garray *x, t_garray *old)
{
    t_error err = PD_ERROR_NONE;

    err |= (garray_getSize (x)  != garray_getSize (old));
    err |= (x->x_saveWithParent != old->x_saveWithParent);
    err |= (x->x_hideName       != old->x_hideName);
    err |= (x->x_inhibit        != old->x_inhibit);
    err |= (x->x_unexpandedName != old->x_unexpandedName);
    
    err |= (scalar_getFloat (x->x_scalar, sym_style) != scalar_getFloat (old->x_scalar, sym_style));

    if (!err) {
    //
    t_word *dest = array_getElements (garray_getArray (x));
    t_word *src  = array_getElements (garray_getArray (old));
    
    array_copyGraphicArray (dest, src, garray_getSize (x));
    //
    }
}

static void garray_restore (t_garray *x)
{
    t_garray *old = (t_garray *)instance_pendingFetch (cast_gobj (x));
    
    if (!old) {
        old = (t_garray *)garbage_fetch (cast_gobj (x));
    }
    
    if (old) { garray_restoreProceed (x, old); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_garray *garray_makeObjectWithScalar (t_glist *glist,
    t_symbol *name,
    t_symbol *templateIdentifier,
    int save, 
    int hide,
    int inhibit)
{
    t_garray *x = (t_garray *)pd_new (garray_class);
    
    x->x_scalar         = scalar_new (glist, templateIdentifier);
    x->x_owner          = glist;
    x->x_unexpandedName = name;
    x->x_name           = dollar_expandSymbol (name, glist);
    x->x_saveWithParent = save;
    x->x_hideName       = hide;
    
    if (symbol_getThingByClass (x->x_name, garray_class)) {
        error_alreadyExists (x->x_name);
        x->x_name = utils_getUnusedBindName (garray_class, sym_array);
        x->x_unexpandedName = x->x_name;
    }
    
    pd_bind (cast_pd (x), x->x_name);
    
    glist_objectAdd (glist, cast_gobj (x));
    
    garray_setInhibit (x, inhibit);
    
    return x;
}

/* Note that an unexpanded name is expected. */

t_garray *garray_makeObject (t_glist *glist, t_symbol *name, t_float size, t_float flags)
{
    t_template *tmpl = template_findByIdentifier (sym__TEMPLATE_float__dash__array);
    
    t_garray *x = NULL;
    
    PD_ASSERT (tmpl);
    
    if (template_fieldIsArrayAndValid (tmpl, sym_z)) {
    //
    int save    = (((int)flags & GARRAY_FLAG_SAVE) != 0);
    int hide    = (((int)flags & GARRAY_FLAG_HIDE) != 0);
    int style   = PD_CLAMP ((((int)flags & GARRAY_FLAG_PLOT) >> 1), PLOT_POLYGONS, PLOT_CURVES);
    int inhibit = (((int)flags & GARRAY_FLAG_INHIBIT) != 0);
    int n       = (int)PD_MAX (1.0, size);
    
    x = garray_makeObjectWithScalar (glist, name, sym__TEMPLATE_float__dash__array, save, hide, inhibit);

    array_resize (scalar_getArray (x->x_scalar, sym_z), n);

    scalar_setFloat (x->x_scalar, sym_style, style);
    scalar_setFloat (x->x_scalar, sym_linewidth, 1);

    instance_setBoundA (cast_pd (x));
    
    garray_redraw (x);
    garray_updateGraphSize (x, n, style);
    garray_updateGraphName (x);
    garray_updateGraphWindow (x);
    dsp_update();
    //
    }
    
    PD_ASSERT (x != NULL);
    
    return x;
}

static void garray_dismiss (t_garray *x)
{
    if (!x->x_dismissed) {
    //
    x->x_dismissed = 1;
    
    gui_jobRemove ((void *)x);
    stub_destroyWithKey ((void *)x);
    pd_unbind (cast_pd (x), x->x_name);
    
    x->x_owner = NULL;
    //
    }
}

static void garray_free (t_garray *x)
{
    garray_dismiss (x); pd_free (cast_pd (x->x_scalar));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void garray_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_garray,
            NULL,
            (t_method)garray_free,
            sizeof (t_garray),
            CLASS_GRAPHIC,
            A_NULL);
        
    class_addList (c, (t_method)garray_list);
    
    class_addMethod (c, (t_method)garray_constant,      sym_constant,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_normalize,     sym_normalize,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_sinesum,       sym_sinesum,        A_GIMME,  A_NULL);
    class_addMethod (c, (t_method)garray_cosinesum,     sym_cosinesum,      A_GIMME,  A_NULL);
    class_addMethod (c, (t_method)garray_rename,        sym_rename,         A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_read,          sym_read,           A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_write,         sym_write,          A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_resize,        sym_resize,         A_FLOAT,  A_NULL);
    class_addMethod (c, (t_method)garray_range,         sym_range,          A_GIMME,  A_NULL);
    class_addMethod (c, (t_method)garray_fromDialog,    sym__arraydialog,   A_GIMME,  A_NULL);
    class_addMethod (c, (t_method)garray_restore,       sym__restore,       A_NULL);
    
    class_setWidgetBehavior (c, &garray_widgetBehavior);
    class_setSaveFunction (c, garray_functionSave);
    class_setDataFunction (c, garray_functionData);
    class_setUndoFunction (c, garray_functionUndo);
    class_setDismissFunction (c, garray_functionDismiss);
    class_requirePending (c);
    
    garray_class = c;
}

void garray_destroy (void)
{
    class_free (garray_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
