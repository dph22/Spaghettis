
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Weak pointer machinery. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gmaster *gpointer_masterCreateWithGlist (t_glist *glist)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (glist);
    
    master->gm_type         = POINTER_GLIST;
    master->gm_un.gm_glist  = glist;
    master->gm_count        = 0;
    
    return master;
}

t_gmaster *gpointer_masterCreateWithArray (t_array *array)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (array);
    
    master->gm_type         = POINTER_ARRAY;
    master->gm_un.gm_array  = array;
    master->gm_count        = 0;
    
    return master;
}

void gpointer_masterRelease (t_gmaster *master)
{
    PD_ASSERT (master->gm_count >= 0);
    
    master->gm_type = POINTER_NONE; if (master->gm_count == 0) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gpointer_incrementMaster (t_gmaster *master)
{
    master->gm_count++;
}

static void gpointer_decrementMaster (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == POINTER_NONE) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_initialize (t_gpointer *gp)
{
    gp->gp_un.gp_scalar = NULL;
    gp->gp_master       = NULL;
    gp->gp_identifier   = 0;
}

void gpointer_setScalar (t_gpointer *gp, t_glist *glist, t_scalar *scalar)
{
    if (gp->gp_master) { gpointer_decrementMaster (gp->gp_master); }
    
    gp->gp_un.gp_scalar = scalar;
    gp->gp_master       = glist->gl_master;
    gp->gp_identifier   = glist->gl_identifier;

    gpointer_incrementMaster (gp->gp_master);
}

void gpointer_setWord (t_gpointer *gp, t_array *array, t_word *w)
{
    if (gp->gp_master) { gpointer_decrementMaster (gp->gp_master); }
    
    gp->gp_un.gp_w      = w;
    gp->gp_master       = array->a_master;
    gp->gp_identifier   = array->a_identifier;

    gpointer_incrementMaster (gp->gp_master);
}

void gpointer_unset (t_gpointer *gp)
{
    if (gp->gp_master) { gpointer_decrementMaster (gp->gp_master); }
    
    gpointer_initialize (gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_copy (const t_gpointer *src, t_gpointer *dest)
{
    gpointer_unset (dest);
    
    *dest = *src;
    
    if (dest->gp_master) { gpointer_incrementMaster (dest->gp_master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_isValid (const t_gpointer *gp, int headPointerIsValid)
{
    t_gmaster *master = gp->gp_master;
    
    if (master) {
    //
    if (master->gm_type == POINTER_ARRAY) {
        if (master->gm_un.gm_array->a_identifier == gp->gp_identifier)  { return 1; }
        
    } else if (master->gm_type == POINTER_GLIST) {
        if (!headPointerIsValid && !gp->gp_un.gp_scalar)                { return 0; }
        if (master->gm_un.gm_glist->gl_identifier == gp->gp_identifier) { return 1; }
        
    } else {
        PD_ASSERT (master->gm_type == POINTER_NONE);
    }
    //
    }
    
    return 0;
}

/* get the template for the object pointer to.  Assumes we've already checked
freshness. */

t_symbol *gpointer_gettemplatesym (const t_gpointer *gp)
{
    t_gmaster *gs = gp->gp_master;
    if (gs->gm_type == POINTER_GLIST)
    {
        t_scalar *sc = gp->gp_un.gp_scalar;
        if (sc)
            return (sc->sc_template);
        else return (0);
    }
    else
    {
        t_array *a = gs->gm_un.gm_array;
        return (a->a_template);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
