
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *undomotion_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undomotion {
    t_undoaction    x_undo;             /* Must be the first. */
    int             x_deltaX;
    int             x_deltaY;
    } t_undomotion;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undomotion_collapse (t_undoaction *kept, t_undoaction *deleted)
{
    t_undomotion *a = (t_undomotion *)kept;
    t_undomotion *b = (t_undomotion *)deleted;
    
    PD_ASSERT (pd_class (kept)    == undomotion_class);
    PD_ASSERT (pd_class (deleted) == undomotion_class);
    
    a->x_deltaX += b->x_deltaX;
    a->x_deltaY += b->x_deltaY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undomotion_undo (t_undomotion *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    int m = -(z->x_deltaX);
    int n = -(z->x_deltaY);
    
    glist_objectDisplaceByUnique (undoaction_getUnique (x), m, n);
}

void undomotion_redo (t_undomotion *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    int m = z->x_deltaX;
    int n = z->x_deltaY;
    
    glist_objectDisplaceByUnique (undoaction_getUnique (x), m, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undomotion_new (t_gobj *o, int deltaX, int deltaY)
{
    t_undoaction *x = (t_undoaction *)pd_new (undomotion_class);
    t_undomotion *z = (t_undomotion *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_MOTION;
    x->ua_safe  = 1;
    x->ua_label = sym_motion;
    
    z->x_deltaX = deltaX;
    z->x_deltaY = deltaY;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undomotion_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undomotion,
            NULL,
            NULL,
            sizeof (t_undomotion),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undomotion_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undomotion_redo, sym_redo, A_GIMME, A_NULL);

    undomotion_class = c;
}

void undomotion_destroy (void)
{
    class_free (undomotion_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
