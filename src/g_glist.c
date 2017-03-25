
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist *glist_getRoot (t_glist *glist)
{
    if (glist_isRoot (glist)) { return glist; }
    else {
        return glist_getRoot (glist_getParent (glist));
    }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_environment) { glist = glist_getParent (glist); PD_ASSERT (glist); }
    
    PD_ASSERT (glist_isRoot (glist));
    
    return glist->gl_environment;
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist_hasParent (glist) && !glist_canHaveWindow (glist)) { 
        glist = glist_getParent (glist); 
    }
    
    return glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int glist_isMapped (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isRoot (t_glist *glist)
{
    int k = (!glist_hasParent (glist) || glist_isAbstraction (glist));
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist_hasParent (glist) && glist->gl_environment != NULL);
}

int glist_isSubpatch (t_glist *glist)
{
    return !glist_isRoot (glist);
}

int glist_isDirty (t_glist *glist)
{
    return (glist_getRoot (glist)->gl_isDirty != 0);
}

/* A graph-on-parent that contains an array of numbers. */

int glist_isArray (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

/* Either a top window a subpacth or a graph-on-parent forced. */

int glist_canHaveWindow (t_glist *glist)
{
    return (glist_hasWindow (glist) || !glist_isGraphOnParent (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_setName (t_glist *glist, t_symbol *name)
{
    canvas_unbind (glist);
    glist->gl_name = name;
    canvas_bind (glist);
    
    if (glist_hasWindow (glist)) { canvas_updateTitle (glist); }
}

void glist_setMapped (t_glist *glist, int n)
{
    glist->gl_isMapped = (n != 0);
}

void glist_setDirty (t_glist *glist, int n)
{
    int isDirty = (n != 0);
        
    t_glist *y = glist_getRoot (glist);
        
    if (y->gl_isDirty != isDirty) {
    //
    y->gl_isDirty = isDirty; 
    
    if (glist_hasWindow (y)) { canvas_updateTitle (y); }
    //
    }
}

void glist_setEditMode (t_glist *glist, int n)
{
    glist->gl_isEditing = (n != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
