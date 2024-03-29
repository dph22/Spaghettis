
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_class_h_
#define __m_class_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void (*t_freemethod)                (t_pd *x);
typedef void (*t_bangmethod)                (t_pd *x);
typedef void (*t_floatmethod)               (t_pd *x, t_float f);
typedef void (*t_symbolmethod)              (t_pd *x, t_symbol *s);
typedef void (*t_listmethod)                (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anythingmethod)            (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_pointermethod)             (t_pd *x, t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    SAVE_DEFAULT        = 0,
    SAVE_COPY           = 1,
    SAVE_UNDO           = 2,
    SAVE_ENCAPSULATE    = 4,
    SAVE_UPDATE         = 8
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SAVED_DEEP(flags)                   (flags & SAVE_UNDO || flags & SAVE_ENCAPSULATE)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void        (*t_savefn)             (t_gobj *x, t_buffer *b, int flags);
typedef t_buffer    *(*t_datafn)            (t_gobj *x, int flags);
typedef void        (*t_undofn)             (t_gobj *x, t_buffer *b);
typedef void        (*t_propertiesfn)       (t_gobj *x, t_glist *glist, t_mouse *m);
typedef void        (*t_valuefn)            (t_gobj *x, t_glist *glist, t_mouse *m);
typedef void        (*t_dismissfn)          (t_gobj *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void (*t_getrectanglefn)            (t_gobj *x, t_glist *glist, t_rectangle *r);
typedef void (*t_displacedfn)               (t_gobj *x, t_glist *glist, int deltaX, int deltaY);
typedef void (*t_selectedfn)                (t_gobj *x, t_glist *glist, int isSelected);
typedef void (*t_activatedfn)               (t_gobj *x, t_glist *glist, int isActivated);
typedef void (*t_deletedfn)                 (t_gobj *x, t_glist *glist);
typedef void (*t_visibilityfn)              (t_gobj *x, t_glist *glist, int isVisible);
typedef int  (*t_mousefn)                   (t_gobj *x, t_glist *glist, t_mouse *m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void (*t_paintergetrectanglefn)     (t_gobj *x, t_gpointer *gp, t_float a, t_float b, t_rectangle *r);
typedef void (*t_paintervisibilityfn)       (t_gobj *x, t_gpointer *gp, t_float a, t_float b, int isVisible);
typedef int  (*t_paintermousefn)            (t_gobj *x, t_gpointer *gp, t_float a, t_float b, t_mouse *m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _widgetbehavior {
    t_getrectanglefn            w_fnGetRectangle;
    t_displacedfn               w_fnDisplaced;
    t_selectedfn                w_fnSelected;
    t_activatedfn               w_fnActivated;
    t_deletedfn                 w_fnDeleted;
    t_visibilityfn              w_fnVisibilityChanged;
    t_mousefn                   w_fnMouse;
    } t_widgetbehavior;
    
typedef struct _painterbehavior {
    t_paintergetrectanglefn     w_fnPainterGetRectangle;
    t_paintervisibilityfn       w_fnPainterVisibilityChanged;
    t_paintermousefn            w_fnPainterMouse;
    } t_painterbehavior;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _entry {
    t_symbol                    *me_name;
    t_method                    me_method;
    t_atomtype                  me_arguments[PD_ARGUMENTS + 1];
    } t_entry;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _class {
    t_symbol                    *c_name;
    t_symbol                    *c_helpName;
    t_symbol                    *c_helpDirectory;
    t_entry                     *c_methods;
    int                         c_methodsSize;
    t_freemethod                c_methodFree;
    t_bangmethod                c_methodBang;
    t_floatmethod               c_methodFloat;
    t_symbolmethod              c_methodSymbol;
    t_listmethod                c_methodList;
    t_anythingmethod            c_methodAnything;
    t_pointermethod             c_methodPointer;
    t_widgetbehavior            *c_behaviorWidget;
    t_painterbehavior           *c_behaviorPainter;
    t_savefn                    c_fnSave;
    t_datafn                    c_fnData;
    t_undofn                    c_fnUndo;
    t_propertiesfn              c_fnProperties;
    t_valuefn                   c_fnValue;
    t_dismissfn                 c_fnDismiss;
    int                         c_requirePending;
    int                         c_hasSignal;
    int                         c_hasFirstInlet;
    int                         c_hasDSP;
    int                         c_type;
    size_t                      c_size;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_method    class_getMethod                     (t_class *c, t_symbol *s);
int         class_hasMethod                     (t_class *c, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         class_hasOverrideBangMethod         (t_class *c);
int         class_hasOverridePointerMethod      (t_class *c);
int         class_hasOverrideFloatMethod        (t_class *c);
int         class_hasOverrideSymbolMethod       (t_class *c);
int         class_hasOverrideListMethod         (t_class *c);
int         class_hasOverrideAnythingMethod     (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int class_isBox (t_class *c)
{
    return (c->c_type == CLASS_BOX);
}

static inline int class_isAbstract (t_class *c)
{
    return (c->c_type == CLASS_ABSTRACT);
}

static inline int class_isInvisible (t_class *c)
{
    return (c->c_type == CLASS_INVISIBLE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline size_t class_getInstanceSize (t_class *c)
{
    return c->c_size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int class_hasDSP (t_class *c)
{
    return (c->c_hasDSP != 0);
}

static inline int class_hasFirstInlet (t_class *c)
{
    return (c->c_hasFirstInlet != 0);
}

static inline int class_hasFirstInletAsSignal (t_class *c)
{
    return (class_hasFirstInlet (c) && c->c_hasSignal);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void class_requirePending (t_class *c)
{
    PD_ASSERT (!class_hasDSP (c)); c->c_requirePending = 1;
}

static inline int class_hasPendingRequired (t_class *c)
{
    return (c->c_requirePending != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int class_hasFreeMethod (t_class *c)
{
    return (c->c_methodFree != NULL);
}

static inline t_freemethod class_getFreeMethod (t_class *c)
{
    return c->c_methodFree;
}

static inline t_bangmethod class_getBangMethod (t_class *c)
{
    return c->c_methodBang;
}

static inline t_floatmethod class_getFloatMethod (t_class *c)
{
    return c->c_methodFloat;
}

static inline t_symbolmethod class_getSymbolMethod (t_class *c)
{
    return c->c_methodSymbol;
}

static inline t_listmethod class_getListMethod (t_class *c)
{
    return c->c_methodList;
}

static inline t_anythingmethod class_getAnythingMethod (t_class *c)
{
    return c->c_methodAnything;
}

static inline t_pointermethod class_getPointerMethod (t_class *c)
{
    return c->c_methodPointer;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_symbol *class_getName (t_class *c)
{
    return c->c_name;
}

static inline const char *class_getNameAsString (t_class *c)
{
    return c->c_name->s_name;
}

static inline const char *class_getHelpNameAsString (t_class *c)
{
    return c->c_helpName->s_name;
}

static inline const char *class_getHelpDirectoryAsString (t_class *c)
{
    return c->c_helpDirectory->s_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int class_hasWidgetBehavior (t_class *c)
{
    return (c->c_behaviorWidget != NULL);
}

static inline int class_hasPainterBehavior (t_class *c)
{
    return (c->c_behaviorPainter != NULL);
}

static inline int class_hasSaveFunction (t_class *c)
{
    return (c->c_fnSave != NULL);
}

static inline int class_hasDataFunction (t_class *c)
{
    return (c->c_fnData != NULL);
}

static inline int class_hasUndoFunction (t_class *c)
{
    return (c->c_fnUndo != NULL);
}

static inline int class_hasPropertiesFunction (t_class *c)
{
    return (c->c_fnProperties != NULL);
}

static inline int class_hasValueFunction (t_class *c)
{
    return (c->c_fnValue != NULL);
}

static inline int class_hasDismissFunction (t_class *c)
{
    return (c->c_fnDismiss != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_widgetbehavior *class_getWidgetBehavior (t_class *c)
{
    return c->c_behaviorWidget;
}

static inline t_painterbehavior *class_getPainterBehavior (t_class *c)
{
    return c->c_behaviorPainter;
}

static inline t_savefn class_getSaveFunction (t_class *c)
{
    return c->c_fnSave;
}

static inline t_datafn class_getDataFunction (t_class *c)
{
    return c->c_fnData;
}

static inline t_undofn class_getUndoFunction (t_class *c)
{
    return c->c_fnUndo;
}

static inline t_propertiesfn class_getPropertiesFunction (t_class *c)
{
    return c->c_fnProperties;
}

static inline t_valuefn class_getValueFunction (t_class *c)
{
    return c->c_fnValue;
}

static inline t_dismissfn class_getDismissFunction (t_class *c)
{
    return c->c_fnDismiss;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void class_setWidgetBehavior (t_class *c, t_widgetbehavior *w)
{
    c->c_behaviorWidget = w;
}

static inline void class_setPainterBehavior (t_class *c, t_painterbehavior *pw)
{
    c->c_behaviorPainter = pw;
}

static inline void class_setSaveFunction (t_class *c, t_savefn f)
{
    c->c_fnSave = f;
}

static inline void class_setDataFunction (t_class *c, t_datafn f)
{
    c->c_fnData = f;
}

static inline void class_setUndoFunction (t_class *c, t_undofn f)
{
    c->c_fnUndo = f;
}

static inline void class_setPropertiesFunction (t_class *c, t_propertiesfn f)
{
    c->c_fnProperties = f;
}

static inline void class_setValueFunction (t_class *c, t_valuefn f)
{
    c->c_fnValue = f;
}

static inline void class_setDismissFunction (t_class *c, t_dismissfn f)
{
    c->c_fnDismiss = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_class_h_
