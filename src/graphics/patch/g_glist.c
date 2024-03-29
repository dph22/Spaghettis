
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

t_outlet *outlet_newUndefined (t_object *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_changeSource (t_gobj *, t_id);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_unbindAll (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_taskRedraw (t_glist *glist)
{
    glist_redraw (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that an expanded name is expected (with or without the file extension). */
/* At load it can be temporarly set with the unexpanded form. */

static t_glist *glist_new (t_glist *owner, 
    t_symbol    *name, 
    t_bounds    *bounds, 
    t_rectangle *graph, 
    t_rectangle *window)
{
    t_glist *x = (t_glist *)pd_new (canvas_class);

    x->gl_holder            = gmaster_createWithGlist (x);
    x->gl_parent            = owner;
    x->gl_environment       = instance_environmentFetchIfAny();
    x->gl_abstractions      = NULL;
    x->gl_undomanager       = undomanager_new (x);
    x->gl_name              = (name != &s_ ? name : environment_getFileName (x->gl_environment));
    x->gl_editor            = editor_new (x);
    x->gl_sorterObjects     = buffer_new();
    x->gl_sorterIndexes     = buffer_new();
    x->gl_clockRedraw       = clock_new ((void *)x, (t_method)glist_taskRedraw);
    x->gl_clockUndo         = clock_new ((void *)x, (t_method)glist_updateUndo);
    x->gl_uniqueIdentifier  = utils_unique();
    
    glist_setFontSize (x, (owner ? glist_getFontSize (owner) : font_getDefaultSize()));

    if (bounds) { bounds_setCopy (&x->gl_bounds, bounds);            }
    if (graph)  { rectangle_setCopy (&x->gl_geometryGraph, graph);   }
    if (window) { rectangle_setCopy (&x->gl_geometryWindow, window); }
    
    rectangle_setNothing (&x->gl_geometryPatch);
    
    if (glist_isRoot (x)) { x->gl_abstractions = abstractions_new(); }
    
    glist_bind (x);
    
    if (glist_isRoot (x)) { instance_rootsAdd (x); }
        
    return x;
}

void glist_free (t_glist *glist)
{
    PD_ASSERT (!glist_objectGetNumberOf (glist));
    
    if (glist_isRoot (glist)) { instance_rootsRemove (glist); }
    
    glist_unbind (glist);
    
    clock_free (glist->gl_clockRedraw);
    clock_free (glist->gl_clockUndo);
    buffer_free (glist->gl_sorterIndexes);
    buffer_free (glist->gl_sorterObjects);
    editor_free (glist_getEditor (glist));
    environment_free (glist->gl_environment);
    gmaster_reset (glist_getMaster (glist));
    
    undomanager_free (glist->gl_undomanager);
    abstractions_free (glist->gl_abstractions);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *glist_newPatchPop (t_symbol *name,
    t_bounds    *bounds, 
    t_rectangle *graph,
    t_rectangle *window, 
    int isOpened,
    int isEditMode,
    int isGOP,
    int fontSize)
{
    if (!utils_isNameAllowedForWindow (name)) { warning_badName (sym_pd, name); }
    
    {
    //
    t_glist *x = glist_newPatch (name, bounds, graph, window, isOpened, isEditMode, isGOP, fontSize);
        
    PD_ASSERT (instance_contextGetCurrent() == x);
    
    instance_stackPopPatch (x, glist_isOpenedAtLoad (x));
    
    return x;
    //
    }
}

t_glist *glist_newPatch (t_symbol *name,
    t_bounds    *bounds, 
    t_rectangle *graph,
    t_rectangle *window, 
    int isOpened,
    int isEditMode,
    int isGOP,
    int fontSize)
{
    t_glist *owner = instance_contextGetCurrent();
    
    t_bounds t1; t_rectangle t2, t3;
    
    bounds_set (&t1, 0, 0, 1, 1);
    rectangle_set (&t2, 0, 0, GLIST_WIDTH / 2, GLIST_HEIGHT / 2);
    rectangle_set (&t3, GLIST_X, GLIST_Y, GLIST_X + GLIST_WIDTH, GLIST_Y + GLIST_HEIGHT);
    
    if (bounds) { bounds_setCopy (&t1, bounds);    }
    if (graph)  { rectangle_setCopy (&t2, graph);  }
    if (window) { rectangle_setCopy (&t3, window); }
    
    {
    //
    t_glist *x = glist_new (owner, name, &t1, &t2, &t3);
    
    object_setBuffer (cast_object (x), buffer_new());
    object_setX (cast_object (x), 0);
    object_setY (cast_object (x), 0);
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_OBJECT);
    
    glist_setFontSize (x, fontSize);
    glist_setGraphOnParent (x, (isGOP != 0));
    glist_setEditMode (x, isEditMode);
    glist_setOpenedAtLoad (x, isOpened);
    
    glist_loadBegin (x); instance_stackPush (x);
    
    return x;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* A root has no parent and an environment. */
/* An abstraction has a parent and an environment. */
/* A subpatch has a parent also but no environment. */
/* A top patch is either a root or an abstraction. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isRoot (t_glist *glist)
{
    return (!glist_hasParent (glist));
}

int glist_isTop (t_glist *glist)
{
    int k = (glist_isRoot (glist) || glist_isAbstraction (glist));
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist_hasParent (glist) && (glist->gl_environment != NULL));
}

int glist_isAbstractionOrInside (t_glist *glist)
{
    return (glist_isAbstraction (glist_getTop (glist)));
}

int glist_isInvisibleOrInside (t_glist *glist)
{
    return (glist_getRoot (glist)->gl_isInvisible);
}

int glist_isSubpatchOrGraphicArray (t_glist *glist)
{
    return (!glist_isTop (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Array is a GOP patch that contains only a scalar. */
/* This scalar has an array of numbers as unique field. */
/* Dirty bit is always owned by the top patch. */
/* For GOP the view to draw is owned by a parent. */
/* Note that a GOP can be opened in its own window on demand. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isSubpatch (t_glist *glist)
{
    return (glist_isSubpatchOrGraphicArray (glist) && !glist_isGraphicArray (glist));
}

int glist_isGraphicArray (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

int glist_isDirty (t_glist *glist)
{
    return (glist_getTop (glist)->gl_isDirty != 0);
}

int glist_isFrozen (t_glist *glist)
{
    return (glist_getTop (glist)->gl_isFrozen != 0);
}

int glist_isOnScreen (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isParentOnScreen (t_glist *g)
{
    return (!glist_isLoading (g) && glist_hasParent (g) && glist_isOnScreen (glist_getParent (g)));
}

int glist_isWindowable (t_glist *glist)
{
    return (!glist_isGraphOnParent (glist) || glist_hasWindow (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *glist_getRoot (t_glist *glist)
{
    if (glist_isRoot (glist)) { return glist; } else { return glist_getRoot (glist_getParent (glist)); }
}

t_glist *glist_getTop (t_glist *glist)
{
    if (glist_isTop (glist)) { return glist; } else { return glist_getTop (glist_getParent (glist)); }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    return (glist_getTop (glist)->gl_environment);
}

t_undomanager *glist_getUndoManager (t_glist *glist)
{
    return glist->gl_undomanager;
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist_hasParent (glist) && !glist_isWindowable (glist)) { glist = glist_getParent (glist); }
    
    return glist;
}

t_garray *glist_getGraphicArray (t_glist *glist)
{
    t_gobj *y = glist->gl_graphics;
    
    if (y && gobj_isGraphicArray (y)) { return (t_garray *)y; }
    else {
        return NULL;    /* Could be NULL in legacy patches or at release time. */
    }
}

/* Code below is required to fetch the unexpanded form. */

t_symbol *glist_getUnexpandedName (t_glist *glist)
{
    t_buffer *z = buffer_new();
    buffer_serialize (z, object_getBuffer (cast_object (glist)));
    t_symbol *s = atom_getSymbolAtIndex (1, buffer_getSize (z), buffer_getAtoms (z));
    buffer_free (z);
    
    return s;
}

t_abstractions *glist_getAbstractions (t_glist *glist)
{
    return glist_getRoot (glist)->gl_abstractions;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_point glist_getPositionForNewObject (t_glist *glist)
{
    t_rectangle *r = glist_getPatchGeometry (glist);
    int a = rectangle_isNothing (r) ? 0 : rectangle_getTopLeftX (r) + (rectangle_getWidth (r)  / 4.0);
    int b = rectangle_isNothing (r) ? 0 : rectangle_getTopLeftY (r) + (rectangle_getHeight (r) / 4.0);
    
    return point_make (a, b);
}

t_point glist_getPositionMiddle (t_glist *glist)
{
    t_rectangle *r = glist_getPatchGeometry (glist);
    int a = rectangle_isNothing (r) ? 0 : rectangle_getMiddleX (r);
    int b = rectangle_isNothing (r) ? 0 : rectangle_getMiddleY (r);
    
    return point_make (a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_updatePatchGeometry (t_glist *glist)
{
    int w = rectangle_getWidth (glist_getWindowGeometry (glist));
    int h = rectangle_getHeight (glist_getWindowGeometry (glist));
    int a = glist_getScrollX (glist);
    int b = glist_getScrollY (glist);
    
    rectangle_setByWidthAndHeight (&glist->gl_geometryPatch, a, b, w, h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_setName (t_glist *glist, t_symbol *name)
{
    if (!utils_isNameAllowedForWindow (name)) { warning_badName (sym_pd, name); }
    
    if (name == &s_) { name = glist_isGraphicArray (glist) ? sym_Array : sym_Patch; }
    
    if (name != glist->gl_name) {
    //
    glist_unbind (glist);
    
    glist->gl_name = name;
    
    glist_bind (glist);
    
    glist_updateTitle (glist);
    
    if (glist_isTop (glist)) { environment_setFileName (glist_getEnvironment (glist), name); }
    //
    }
}

void glist_setDirty (t_glist *glist, int n)
{
    int isDirty       = (n != 0);
    t_glist *y        = glist_getTop (glist);
    int isAbstraction = glist_isAbstraction (y);
    
    if (!isAbstraction) {
    //
    if (y->gl_isDirty != isDirty) {
    //
    y->gl_isDirty = isDirty; 
    
    glist_updateTitle (y);
    //
    }
    //
    }
}

void glist_setFrozen (t_glist *glist, int n)
{
    glist_getTop (glist)->gl_isFrozen = (n != 0);
}

void glist_setInvisible (t_glist *glist)
{
    glist->gl_isInvisible = 1; glist_unbindAll (glist);
}

void glist_setFontSize (t_glist *g, int n)
{
    if (n > 0) { g->gl_fontSize = font_getValidSize (n); }
}

void glist_setMotion (t_glist *glist, t_gobj *y, t_motionfn callback, int a, int b)
{
    editor_motionSet (glist_getEditor (glist_getView (glist)), y, glist, callback, a, b);
}

void glist_setBounds (t_glist *glist, t_bounds *bounds)
{
    bounds_setCopy (glist_getBounds (glist), bounds);
}

void glist_setGraphGeometry (t_glist *glist, t_rectangle *r, t_bounds *bounds, int isGOP)
{   
    int update = glist_isParentOnScreen (glist);
    
    if (update) {
    //
    gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
    //
    }
    
    rectangle_setCopy (glist_getGraphGeometry (glist), r);
    bounds_setCopy (glist_getBounds (glist), bounds);
    glist_setGraphOnParent (glist, isGOP);
        
    if (update) {
    //
    gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    //
    }
}

void glist_setWindowGeometry (t_glist *glist, t_rectangle *r)
{
    rectangle_setCopy (glist_getWindowGeometry (glist), r); glist_updatePatchGeometry (glist);
}

void glist_setScroll (t_glist *glist, int a, int b)
{
    glist->gl_scrollX = a; glist->gl_scrollY = b; glist_updatePatchGeometry (glist);
}

void glist_setIdentifiers (t_glist *glist, int argc, t_atom *argv)
{
    t_id u; t_error err = utils_uniqueWithAtoms (argc, argv, &u);
    
    PD_ASSERT (!err); PD_UNUSED (err);
    
    gobj_changeIdentifiers (cast_gobj (glist), u);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_setDollarZero (t_glist *glist, int n)
{
    environment_setDollarZero (glist_getEnvironment (glist), n);
}

int glist_getDollarZero (t_glist *glist)
{
    return environment_getDollarZero (glist_getEnvironment (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_bindProceed (t_glist *glist)
{
    t_symbol *s = symbol_removeExtension (glist_getName (glist));
    
    if (utils_isNameAllowedForWindow (s)) {
        pd_bind (cast_pd (glist), symbol_makeBind (s));
    }
}

void glist_bind (t_glist *glist)
{
    if (!glist_isInvisibleOrInside (glist)) { glist_bindProceed (glist); }
}

static void glist_unbindProceed (t_glist *glist)
{
    t_symbol *s = symbol_removeExtension (glist_getName (glist));
    
    if (utils_isNameAllowedForWindow (s)) {
        pd_unbind (cast_pd (glist), symbol_makeBind (s));
    }
}

void glist_unbind (t_glist *glist)
{
    if (!glist_isInvisibleOrInside (glist)) { glist_unbindProceed (glist); }
}

static void glist_unbindAllRecursive (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) { glist_unbindAllRecursive (cast_glist (y)); }
        else if (pd_class (y) == namecanvas_class) {
            pd_message (cast_pd (y), sym_set, 0, NULL);
        }
    }
    
    glist_unbindProceed (cast_glist (glist));
}

static void glist_unbindAll (t_glist *glist)
{
    glist_unbindAllRecursive (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_rename (t_glist *glist, int argc, t_atom *argv)
{
    t_symbol *s = atom_getSymbolOrDollarSymbolAtIndex (0, argc, argv);
    
    if (argc > 1) { warning_unusedArguments (sym_pd, argc - 1, argv + 1); }
    
    glist_setName (glist, dollar_expandSymbol (s, glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_loadbangAbstractions (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) {
            if (glist_isAbstraction (cast_glist (y))) { glist_loadbang (cast_glist (y)); }
            else {
                glist_loadbangAbstractions (cast_glist (y));
            }
        }
    }
}

static void glist_loadbangSubpatches (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) {
            if (!glist_isAbstraction (cast_glist (y))) { glist_loadbangSubpatches (cast_glist (y)); }
        }
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (!gobj_isCanvas (y) && class_hasMethod (pd_class (y), sym_loadbang)) {
            pd_message (cast_pd (y), sym_loadbang, 0, NULL);
        }
    }
}

void glist_loadbang (t_glist *glist)
{
    glist_loadbangAbstractions (glist);
    glist_loadbangSubpatches (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_closebangAbstractions (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) {
            if (glist_isAbstraction (cast_glist (y))) { glist_closebang (cast_glist (y)); }
            else {
                glist_closebangAbstractions (cast_glist (y));
            }
        }
    }
}

static void glist_closebangSubpatches (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) {
            if (!glist_isAbstraction (cast_glist (y))) { glist_closebangSubpatches (cast_glist (y)); }
        }
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (!gobj_isCanvas (y) && class_hasMethod (pd_class (y), sym_closebang)) {
            pd_message (cast_pd (y), sym_closebang, 0, NULL);
        }
    }
    
    glist->gl_hasBeenCloseBanged = 1;
}

void glist_closebang (t_glist *glist)
{
    if (!glist->gl_hasBeenCloseBanged) {
    //
    glist_closebangAbstractions (glist);
    glist_closebangSubpatches (glist);
    
    glist->gl_hasBeenCloseBanged = 1;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Files are searching in the directory of the patch first. */
/* Without success it tries to find it using the search path. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int glist_fileExist (t_glist *glist, const char *name, const char *extension, t_fileproperties *p)
{
    int f = glist_fileOpen (glist, name, extension, p);
    
    if (f >= 0) { close (f); return 1; }
    
    return 0;
}

/* Caller is responsible to close the file. */

int glist_fileOpen (t_glist *glist, const char *name, const char *extension, t_fileproperties *p)
{
    const char *directory = glist ? environment_getDirectoryAsString (glist_getEnvironment (glist)) : ".";
    
    int f = file_openReadConsideringSearchPath (directory, name, extension, p);
        
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int glist_undoIsOk (t_glist *glist)
{
    if (glist_isAbstractionOrInside (glist) || glist_isGraphicArray (glist)) { return 0; }
    else if (editor_hasSelectedBox (glist_getEditor (glist))) { return 0; }
    
    return (glist_hasUndo (glist) && !instance_undoIsRecursive());
}

int glist_undoHasSeparatorAtLast (t_glist *glist)
{
    return (undomanager_hasSeparatorAtLast (glist_getUndoManager (glist)));
}

/* Raw function to use with care. */

t_undomanager *glist_undoReplaceManager (t_glist *glist, t_undomanager *undo)
{
    t_undomanager *t = glist->gl_undomanager; glist->gl_undomanager = undo; return t;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_undoAppendSeparator (t_glist *glist)
{
    if (!glist_undoHasSeparatorAtLast (glist)) {
    //
    undomanager_appendSeparator (glist_getUndoManager (glist));
    
    glist_updateUndo (glist);
    //
    }
}

static void glist_undoAppendProceed (t_glist *glist, t_undoaction *a, int scheduled)
{
    t_undomanager *undo = glist_getUndoManager (glist);
    
    undomanager_append (undo, a);
    
    if (scheduled) { undomanager_appendSeparatorLater (undo); }
    
    glist_updateUndo (glist);
}

void glist_undoAppendUnscheduled (t_glist *glist, t_undoaction *a)
{
    glist_undoAppendProceed (glist, a, 0);
}

void glist_undoAppend (t_glist *glist, t_undoaction *a)
{
    glist_undoAppendProceed (glist, a, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_objectMake (t_glist *glist, int a, int b, int w, int isSelected, t_buffer *t)
{
    instance_setNewestObject (NULL);
    
    instance_stackPush (glist);
    
    {
    //
    t_environment *e = glist_getEnvironment (instance_contextGetCurrent());
    
    t_object *x = NULL;
    
    eval_buffer (t, 
        instance_getMakerObject(), 
        environment_getNumberOfArguments (e), 
        environment_getArguments (e));

    if (instance_getNewestObject()) { x = cast_objectIfConnectable (instance_getNewestObject()); }

    if (!x) {
        x = (t_object *)pd_new (text_class);    /* If failed create a dummy box. */
        if (buffer_getSize (t)) {
            error_canNotMake (buffer_getSize (t), buffer_getAtoms (t));
        }
    }

    /* Replace original name of an abstraction created from snippet (e.g. encapsulation). */
    
    if (buffer_getSize (t) && IS_SYMBOL (buffer_getAtomAtIndex (t, 0))) {
    //
    t_symbol *key  = buffer_getSymbolAt (t, 0);
    
    if (pool_check (key)) {
    //
    t_symbol *name = abstractions_getName (glist_getAbstractions (instance_contextGetCurrent()), key);
    
    if (name) { buffer_setSymbolAtIndex (t, 0, name); }
    //
    }
    //
    }
    
    object_setBuffer (x, t);
    
    if (isSelected) {
        object_setSnappedX (x, a);              /* Interactive creation. */
        object_setSnappedY (x, b);
    } else {
        object_setX (x, a);                     /* File creation. */
        object_setY (x, b);
    }
    
    object_setWidth (x, w);
    object_setType (x, TYPE_OBJECT);
    
    glist_objectAdd (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    glist_objectSelect (glist, cast_gobj (x));
    gobj_activated (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { glist_inletSort (glist_getView (glist));  }
    if (pd_class (x) == voutlet_class) { glist_outletSort (glist_getView (glist)); }
    if (pd_class (x) == block_class)   { dsp_update(); }
    
    if (class_hasDataFunction (pd_class (x))) { instance_setBoundA (cast_pd (x)); }
    //
    }
    
    instance_stackPop (glist);
}

void glist_objectMakeScalar (t_glist *glist, int argc, t_atom *argv)
{
    if (argc > 0) {
    //
    t_atom *expanded = NULL;
    
    PD_ATOMS_ALLOCA (expanded, argc);
    
    atom_copyAtomsExpanded (argv, argc, expanded, argc, glist);
    
    if (IS_SYMBOL (expanded)) {
    //
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (GET_SYMBOL (expanded));
        
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    scalar_deserialize (scalar, argc, expanded);
    glist_objectAdd (glist, cast_gobj (scalar));
    //
    } else {
        error_noSuch (GET_SYMBOL (expanded), sym_template);
    }
    //
    }
    
    PD_ATOMS_FREEA (expanded, argc);
    //
    }
}

void glist_objectSetIdentifiersOfLast (t_glist *glist, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_id u; t_error err = utils_uniqueWithAtoms (argc, argv, &u);
    
    PD_ASSERT (!err); PD_UNUSED (err);
    
    for ((g1 = glist->gl_graphics); (g2 = g1->g_next); (g1 = g2)) { }
    
    gobj_changeIdentifiers (g1, u);
    //
    }
}

void glist_objectSetSourceOfLast (t_glist *glist, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_id u; t_error err = utils_uniqueWithAtoms (argc, argv, &u);
    
    PD_ASSERT (!err); PD_UNUSED (err);
    
    for ((g1 = glist->gl_graphics); (g2 = g1->g_next); (g1 = g2)) { }
    
    gobj_changeSource (g1, u);
    //
    }
}

void glist_objectSetWidthOfLast (t_glist *glist, int w)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    for ((g1 = glist->gl_graphics); (g2 = g1->g_next); (g1 = g2)) { }
    
    if (cast_objectIfConnectable (g1)) {
    
        object_setWidth (cast_object (g1), PD_MAX (1, w));
        
        if (glist_isOnScreen (glist)) {
            gobj_visibilityChanged (g1, glist, 0);
            gobj_visibilityChanged (g1, glist, 1);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_objectAddProceed (t_glist *glist, t_gobj *y, t_gobj *first, int prepend)
{
    y->g_next = NULL;
    
    if (first != NULL) { y->g_next = first->g_next; first->g_next = y; }
    else {
    //    
    if (prepend || !glist->gl_graphics) {
        y->g_next = glist->gl_graphics; glist->gl_graphics = y;
    } else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { }
        t->g_next = y;
    }
    //
    }
    
    instance_registerAdd (y, glist);
}

void glist_objectAddNextProceed (t_glist *glist, t_gobj *y, t_gobj *first)
{
    int needToRepaint = class_hasPainterBehavior (pd_class (y));
    
    if (needToRepaint) { paint_erase(); }
    
    glist_objectAddProceed (glist, y, first, 0);
    
    if (cast_objectIfConnectable (y)) { editor_boxAdd (glist_getEditor (glist), cast_object (y)); }
    if (glist_isOnScreen (glist_getView (glist))) { gobj_visibilityChanged (y, glist, 1); }
    
    if (needToRepaint) { paint_draw(); }
}

void glist_objectAddUndoProceed (t_glist *glist, t_gobj *y)
{
    if (glist_undoIsOk (glist)) {
        t_undosnippet *snippet = undosnippet_newSave (y, glist);
        if (glist_undoHasSeparatorAtLast (glist)) { glist_undoAppend (glist, undoadd_new()); }
        if (obj_isDummy (y)) { glist_undoAppendUnscheduled (glist, undocreate_new (y, snippet)); }
        else {
            glist_undoAppend (glist, undocreate_new (y, snippet));
        }
    }
}

void glist_objectAddNext (t_glist *glist, t_gobj *y, t_gobj *first)
{
    glist_objectAddNextProceed (glist, y, first);
    glist_objectAddUndoProceed (glist, y);
}

void glist_objectAdd (t_glist *glist, t_gobj *y)
{
    glist_objectAddNextProceed (glist, y, NULL);
    glist_objectAddUndoProceed (glist, y);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_objectRemoveProceed (t_glist *glist, t_gobj *y)
{
    instance_registerRemove (y);
    
    if (gobj_isScalar (y)) { glist_setChangeTag (glist); }
    
    if (glist->gl_graphics == y) { glist->gl_graphics = y->g_next; }
    else {
        t_gobj *t = NULL;
        for (t = glist->gl_graphics; t; t = t->g_next) {
            if (t->g_next == y) { t->g_next = y->g_next; break; }
        }
    }
}

static void glist_objectRemoveFree (t_glist *glist, t_gobj *y)
{
    if (gobj_hasDSP (y) && !gobj_isCanvas (y)) {
        if (garbage_newObject (y)) { return; }
    }
    
    if (instance_pendingRequired (y)) { instance_pendingAdd (y); return; }

    pd_free (cast_pd (y));
}

void glist_objectRemove (t_glist *glist, t_gobj *y)
{
    int needToInvalidate = gobj_isScalar (y);
    int needToRebuild    = gobj_hasDSP (y);
    int needToRepaint    = class_hasPainterBehavior (pd_class (y)) || (pd_class (y) == struct_class);
    int undoable         = glist_undoIsOk (glist);
    int state            = 0;
    
    glist_deleteBegin (glist);
    
    editor_motionUnset (glist_getEditor (glist), y);
    
    if (glist_objectIsSelected (glist, y)) { glist_objectDeselect (glist, y, 0); }
    if (needToRepaint) { paint_erase(); }
    if (glist_isOnScreen (glist)) { gobj_visibilityChanged (y, glist, 0); }
    if (gobj_isCanvas (y)) { glist_closebang (cast_glist (y)); }
    
    if (needToRebuild) { state = dsp_suspend(); }
    
    {
        t_box *box = NULL;
    
        if (cast_objectIfConnectable (y)) { box = box_fetch (glist, cast_object (y)); }
    
        if (undoable && glist_undoHasSeparatorAtLast (glist)) {
            glist_undoAppend (glist, undoremove_new());
        }
    
        {
            t_undosnippet *snippet = NULL;
        
            if (undoable) { snippet = undosnippet_newSave (y, glist); }     /* MUST be before call below. */
        
            gobj_deleted (y, glist);
    
            if (undoable) { glist_undoAppend (glist, undodelete_new (y, snippet)); }
        }
    
        glist_objectRemoveProceed (glist, y);
        glist_objectRemoveFree (glist, y);

        if (box) {
            editor_boxRemove (glist_getEditor (glist), box); 
        }
    }
    
    if (needToRebuild)    { dsp_resume (state); }
    if (needToRepaint)    { paint_draw(); }
    if (needToInvalidate) { glist->gl_uniqueIdentifier = utils_unique(); }  /* Invalidate all pointers. */

    glist_deleteEnd (glist);
}

/* Inlets and outlets must be deleted from right to left to handle Undo/Redo properly. */

static void glist_objectRemoveCacheInletsProceed (t_glist *glist, t_gobj *y, int isVinlet)
{
    int i, n = isVinlet ? vinlet_getIndex ((t_vinlet *)y) : voutlet_getIndex ((t_voutlet *)y);
    
    t_atom a;
    
    for (i = 0; i < buffer_getSize (glist->gl_sorterIndexes); i++) {
        if (buffer_getFloatAtIndex (glist->gl_sorterIndexes, i) < n) { break; }
    }
    
    SET_FLOAT (&a, n);  buffer_insertAtIndex (glist->gl_sorterIndexes, i, &a);
    SET_OBJECT (&a, y); buffer_insertAtIndex (glist->gl_sorterObjects, i, &a);
}

void glist_objectRemoveCacheInlets (t_glist *glist, t_gobj *y)
{
    if (pd_class (y) == vinlet_class)       { glist_objectRemoveCacheInletsProceed (glist, y, 1); }
    else if (pd_class (y) == voutlet_class) { glist_objectRemoveCacheInletsProceed (glist, y, 0); }
    else {
        glist_objectRemove (glist, y);
    }
}

void glist_objectRemovePurgeInlets (t_glist *glist)
{
    int i;
    
    for (i = 0; i < buffer_getSize (glist->gl_sorterObjects); i++) {
        glist_objectRemove (glist, buffer_getObjectAt (glist->gl_sorterObjects, i));
    }
    
    buffer_clear (glist->gl_sorterIndexes);
    buffer_clear (glist->gl_sorterObjects);
}

t_error glist_objectRemoveByUnique (t_id u)
{
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    glist_objectRemove (glist, object); return PD_ERROR_NONE;
    //
    }

    return PD_ERROR;
}

/* If needed the DSP is suspended to avoid multiple rebuilds of DSP graph. */

void glist_objectRemoveAll (t_glist *glist)
{
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;
    
    int dspState = 0;
    int dspSuspended = 0;

    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (!dspSuspended) {
        if (gobj_hasDSP (t1)) { dspState = dsp_suspend(); dspSuspended = 1; }
    }

    glist_objectRemoveCacheInlets (glist, t1);
    //
    }
    
    glist_objectRemovePurgeInlets (glist);
    
    if (dspSuspended) { dsp_resume (dspState); }
}

void glist_objectRemoveAllByTemplate (t_glist *glist, t_template *tmpl)
{
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;

    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (gobj_isScalar (t1)) {
        if (scalar_containsTemplate (cast_scalar (t1), template_getTemplateIdentifier (tmpl))) {
            glist_objectRemove (glist, t1);
        }
    }
    
    if (gobj_isCanvas (t1)) {
        glist_objectRemoveAllByTemplate (cast_glist (t1), tmpl);
    }
    //
    }
}

void glist_objectRemoveAllScalars (t_glist *glist)
{
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;

    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (gobj_isScalar (t1)) { glist_objectRemove (glist, t1); }
    //
    }
}

void glist_objectRemoveInletsAndOutlets (t_glist *glist)
{
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;

    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t_class *c = pd_class (t1);
    
    t2 = t1->g_next;
    
    if (c == vinlet_class || c == voutlet_class) { glist_objectRemoveCacheInlets (glist, t1); }
    //
    }
    
    glist_objectRemovePurgeInlets (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_gobj *glist_objectGetAt (t_glist *glist, int n)
{
    t_gobj *t = NULL;
    int i = 0;
    
    for (t = glist->gl_graphics; t; t = t->g_next) {
        if (i == n) { return t; }
        i++;
    }
    
    return NULL;
}

int glist_objectGetIndexOf (t_glist *glist, t_gobj *y)
{
    t_gobj *t = NULL;
    int n = 0;
    
    for (t = glist->gl_graphics; t && t != y; t = t->g_next) { 
        n++; 
    }
    
    return n;
}

int glist_objectGetNumberOf (t_glist *glist)
{
    return glist_objectGetIndexOf (glist, NULL);
}

int glist_objectMoveGetPosition (t_glist *glist, t_gobj *y)
{
    return glist_objectGetIndexOf (glist, y);
}

void glist_objectMoveAtFirst (t_glist *glist, t_gobj *y)
{
    glist_objectRemoveProceed (glist, y);
    glist_objectAddProceed (glist, y, NULL, 1);
}

void glist_objectMoveAtLast (t_glist *glist, t_gobj *y)
{
    glist_objectRemoveProceed (glist, y);
    glist_objectAddProceed (glist, y, NULL, 0);
}

void glist_objectMoveAt (t_glist *glist, t_gobj *y, int n)
{
    if (n < 1) { glist_objectMoveAtFirst (glist, y); }
    else {
    //
    glist_objectRemoveProceed (glist, y);
    glist_objectAddProceed (glist, y, glist_objectGetAt (glist, (n - 1)), 0);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error glist_objectMoveAtFirstByUnique (t_id u)
{
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    glist_objectMoveAtFirst (glist, object); glist_redrawRequired (glist); return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

t_error glist_objectMoveAtLastByUnique (t_id u)
{
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    glist_objectMoveAtLast (glist, object); glist_redrawRequired (glist); return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

t_error glist_objectMoveAtByUnique (t_id u, int n)
{
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    glist_objectMoveAt (glist, object, n); glist_redrawRequired (glist); return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

t_error glist_objectGetIndexOfByUnique (t_id u, int *n)
{
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    (*n) = glist_objectGetIndexOf (glist, object); return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_objectDeleteLines (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int m = (traverser_getSource (&t) == o);
    int n = (traverser_getDestination (&t) == o);
    
    if (m || n) {
    //
    glist_eraseLine (glist, traverser_getCord (&t)); traverser_disconnect (&t, glist);
    //
    }
    //
    }
}

static void glist_objectDeleteLinesByInlets (t_glist *glist, t_object *o, t_inlet *inlet, t_outlet *outlet)
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
    glist_eraseLine (glist, traverser_getCord (&t)); traverser_disconnect (&t, glist);
    //
    }
    //
    }
}

void glist_objectDeleteLinesByInlet (t_glist *glist, t_object *o, t_inlet *inlet)
{
    glist_objectDeleteLinesByInlets (glist, o, inlet, NULL);
}

void glist_objectDeleteLinesByOutlet (t_glist *glist, t_object *o, t_outlet *outlet)
{
    glist_objectDeleteLinesByInlets (glist, o, NULL, outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error glist_objectConnect (t_glist *glist, t_object *src, int m, t_object *dest, int n)
{
    t_outconnect *connection = NULL;

    if ((connection = object_connect (src, m, dest, n))) {
    //
    if (glist_isOnScreen (glist)) {
        t_cord t; cord_make (&t, connection, src, m, dest, n, glist);
        glist_drawLine (glist, &t);
    }
    
    if (glist_undoIsOk (glist)) { glist_undoAppend (glist, undoconnect_new (src, m, dest, n)); }

    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_inlet *glist_inletAdd (t_glist *glist, t_pd *receiver, int isSignal)
{
    t_inlet *inlet = inlet_new (cast_object (glist), receiver, (isSignal ? &s_signal : NULL), NULL);
    
    if (!glist_isLoading (glist)) {
    //
    if (glist_isParentOnScreen (glist)) {
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    
    glist_inletSort (glist);
    //
    }
    
    return inlet;
}

void glist_inletRemove (t_glist *glist, t_inlet *inlet)
{
    t_glist *owner = glist_getParent (glist);
    
    int redraw = (owner && !glist_isDeleting (owner) && glist_isOnScreen (owner));
    
    if (owner)  { glist_objectDeleteLinesByInlet (owner, cast_object (glist), inlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }
        
    inlet_free (inlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { glist_updateLinesForObject (owner, cast_object (glist)); }
}

int glist_inletNumberOf (t_glist *glist)
{
    int n = 0;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == vinlet_class) { n++; } }
    
    return n;
}

void glist_inletSort (t_glist *glist)
{
    int numberOfInlets = glist_inletNumberOf (glist);

    if (numberOfInlets > 1) {
    //
    int i;
    t_gobj *y = NULL;
    
    /* Fetch all inlets into a list. */
    
    t_gobj **inlets    = (t_gobj **)PD_MEMORY_GET (numberOfInlets * sizeof (t_gobj *));
    t_rectangle *boxes = (t_rectangle *)PD_MEMORY_GET (numberOfInlets * sizeof (t_rectangle));
    t_gobj **t         = inlets;
    t_rectangle *b     = boxes;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (pd_class (y) == vinlet_class) { *t = y; gobj_getRectangle (y, glist, b); t++; b++; }
    //
    }
    
    /* Take the most right inlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfInlets; i > 0; i--) {
    //
    int j = numberOfInlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightInlet = NULL;
    
    for (t = inlets, b = boxes; j--; t++, b++) {
        if (*t && rectangle_getTopLeftX (b) > maximumX) {
            maximumX = rectangle_getTopLeftX (b); mostRightInlet = t;
        }
    }
    
    if (mostRightInlet) {
        inlet_moveFirst (vinlet_getInlet (cast_pd (*mostRightInlet))); *mostRightInlet = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (boxes);
    PD_MEMORY_FREE (inlets);
    
    if (glist_isParentOnScreen (glist)) {
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_outlet *glist_outletAddProceed (t_glist *glist, t_outlet *outlet)
{
    if (!glist_isLoading (glist)) {
    //
    if (glist_isParentOnScreen (glist)) {
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    
    glist_outletSort (glist);
    //
    }
    
    return outlet;
}

t_outlet *glist_outletAddSignal (t_glist *glist)
{
    return glist_outletAddProceed (glist, outlet_newSignal (cast_object (glist)));
}

t_outlet *glist_outletAdd (t_glist *glist)
{
    return glist_outletAddProceed (glist, outlet_newMixed (cast_object (glist)));
}

void glist_outletRemove (t_glist *glist, t_outlet *outlet)
{
    t_glist *owner = glist_getParent (glist);
    
    int redraw = (owner && !glist_isDeleting (owner) && glist_isOnScreen (owner));
    
    if (owner)  { glist_objectDeleteLinesByOutlet (owner, cast_object (glist), outlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }

    outlet_free (outlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { glist_updateLinesForObject (owner, cast_object (glist)); }
}

int glist_outletNumberOf (t_glist *glist)
{
    int n = 0;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == voutlet_class) { n++; } }
    
    return n;
}

void glist_outletSort (t_glist *glist)
{
    int numberOfOutlets = glist_outletNumberOf (glist);
    
    if (numberOfOutlets > 1) {
    //
    int i;
    t_gobj *y = NULL;
    
    /* Fetch all outlets into a list. */
    
    t_gobj **outlets   = (t_gobj **)PD_MEMORY_GET (numberOfOutlets * sizeof (t_gobj *));
    t_rectangle *boxes = (t_rectangle *)PD_MEMORY_GET (numberOfOutlets * sizeof (t_rectangle));
    t_gobj **t         = outlets;
    t_rectangle *b     = boxes;
        
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (pd_class (y) == voutlet_class) { *t = y; gobj_getRectangle (y, glist, b); t++; b++; }
    //
    }
    
    /* Take the most right outlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfOutlets; i > 0; i--) {
    //
    int j = numberOfOutlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightOutlet = NULL;
    
    for (t = outlets, b = boxes; j--; t++, b++) {
        if (*t && rectangle_getTopLeftX (b) > maximumX) {
            maximumX = rectangle_getTopLeftX (b); mostRightOutlet = t;
        }
    }
    
    if (mostRightOutlet) {
        outlet_moveFirst (voutlet_getOutlet (cast_pd (*mostRightOutlet))); *mostRightOutlet = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (boxes);
    PD_MEMORY_FREE (outlets);
    
    if (glist_isParentOnScreen (glist)) {
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_lineSelect (t_glist *glist, t_traverser *t)
{
    glist_deselectAll (glist);
                
    editor_selectedLineSet (glist_getEditor (glist),
        cord_getConnection (traverser_getCord (t)),
        glist_objectGetIndexOf (glist, cast_gobj (traverser_getSource (t))), 
        traverser_getIndexOfOutlet (t),
        glist_objectGetIndexOf (glist, cast_gobj (traverser_getDestination (t))), 
        traverser_getIndexOfInlet (t));
        
    glist_updateLineSelected (glist, 1);
}

void glist_lineDeselect (t_glist *glist)
{
    glist_updateLineSelected (glist, 0);
    
    editor_selectedLineReset (glist_getEditor (glist));
}

int glist_lineExist (t_glist *glist, t_object *o, int m, t_object *i, int n)
{
    t_outconnect *connection = NULL;
    
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) { 
        if (traverser_isLineBetween (&t, o, m, i, n)) {
            return 1; 
        } 
    }
    
    return 0;
}

t_error glist_lineConnect (t_glist *glist, 
    int indexOfObjectOut, 
    int indexOfOutlet, 
    int indexOfObjectIn,
    int indexOfInlet)
{
    t_gobj *src  = glist_objectGetAt (glist, indexOfObjectOut);
    t_gobj *dest = glist_objectGetAt (glist, indexOfObjectIn);
    
    PD_ASSERT (src);
    PD_ASSERT (dest);
    
    if (src && dest) {
    //
    t_object *srcObject  = cast_objectIfConnectable (src);
    t_object *destObject = cast_objectIfConnectable (dest);
    
    if (srcObject && destObject) {
    //
    int m = indexOfOutlet;
    int n = indexOfInlet;
    
    /* Creates dummy outlets and inlets. */
    /* It is required in case of failure at object creation. */
    
    if (object_isDummy (srcObject)) {
        while (m >= object_getNumberOfOutlets (srcObject)) {
            outlet_newUndefined (srcObject);
        }
    }
    
    if (object_isDummy (destObject)) {
        while (n >= object_getNumberOfInlets (destObject)) {
            inlet_new (destObject, cast_pd (destObject), NULL, NULL);
        }
    }

    return glist_objectConnect (glist, srcObject, m, destObject, n);
    //
    }
    //
    }
    
    return PD_ERROR;
}

t_error glist_lineConnectByUnique (t_id u, int indexOfOutlet, t_id v, int indexOfInlet)
{
    t_gobj *src  = instance_registerGetObject (u);
    t_gobj *dest = instance_registerGetObject (v);

    if (src && dest) {
    //
    t_object *srcObject  = cast_objectIfConnectable (src);
    t_object *destObject = cast_objectIfConnectable (dest);
    
    if (srcObject && destObject) {
    //
    t_glist *srcOwner    = instance_registerGetOwner (u);
    t_glist *destOwner   = instance_registerGetOwner (v);
    
    if (srcOwner && (srcOwner == destOwner)) {
        return glist_objectConnect (srcOwner, srcObject, indexOfOutlet, destObject, indexOfInlet);
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

t_error glist_lineDisconnect (t_glist *glist, 
    int indexOfObjectOut, 
    int indexOfOutlet, 
    int indexOfObjectIn,
    int indexOfInlet)
{
    t_outconnect *connection = NULL;
    t_traverser t;
        
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if ((traverser_getIndexOfOutlet (&t) == indexOfOutlet)) {
        if ((traverser_getIndexOfInlet (&t) == indexOfInlet)) {

            int m = glist_objectGetIndexOf (glist, cast_gobj (traverser_getSource (&t)));
            int n = glist_objectGetIndexOf (glist, cast_gobj (traverser_getDestination (&t)));

            if (m == indexOfObjectOut && n == indexOfObjectIn) {
                glist_eraseLine (glist, traverser_getCord (&t));
                traverser_disconnect (&t, glist);
                return PD_ERROR_NONE;
            }
        }
    }
    //
    }
    
    return PD_ERROR;
}

t_error glist_lineDisconnectByUnique (t_id u, int indexOfOutlet, t_id v, int indexOfInlet)
{
    t_glist *srcOwner  = instance_registerGetOwner (u);
    t_glist *destOwner = instance_registerGetOwner (v);
    
    if (srcOwner && (srcOwner == destOwner)) {
    //
    t_gobj *src  = instance_registerGetObject (u);
    t_gobj *dest = instance_registerGetObject (v);
    
    if (src && dest) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;
    
    traverser_start (&t, srcOwner);
    
    while ((connection = traverser_next (&t))) {
    //
    if (indexOfOutlet == traverser_getIndexOfOutlet (&t)) {
    if (indexOfInlet  == traverser_getIndexOfInlet (&t)) {
    if (src           == cast_gobj (traverser_getSource (&t))) {
    if (dest          == cast_gobj (traverser_getDestination (&t))) {
    //
    glist_eraseLine (srcOwner, traverser_getCord (&t)); return traverser_disconnect (&t, srcOwner);
    //
    }
    }
    }
    }
    //
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

void glist_lineDeleteSelected (t_glist *glist)
{
    if (editor_hasSelectedLine (glist_getEditor (glist))) {
        editor_selectedLineDisconnect (glist_getEditor (glist));
        glist_setDirty (glist, 1);
        if (glist_undoIsOk (glist)) { glist_undoAppendSeparator (glist); }
    }
}

void glist_lineCheck (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    t_object *o1 = traverser_getSource (&t);
    t_object *o2 = traverser_getDestination (&t);
    
    if (!object_isDummy (o1) && !object_isDummy (o2)) {
    if (o1 == o || o2 == o) {
        
        int m = traverser_getIndexOfOutlet (&t);
        int n = traverser_getIndexOfInlet (&t);
        
        if (object_isSignalOutlet (o1, m) && !object_isSignalInlet (o2, n)) {
            traverser_disconnect (&t, NULL); error_failed (sym_connect);
        }
    }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
