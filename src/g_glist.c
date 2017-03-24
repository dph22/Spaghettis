
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
        return glist_getRoot (glist->gl_parent);
    }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_environment) { glist = glist->gl_parent; PD_ASSERT (glist); }
    
    PD_ASSERT (glist_isRoot (glist));
    
    return glist->gl_environment;
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist->gl_parent && !glist_canHaveWindow (glist)) { glist = glist->gl_parent; }
    
    return glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int glist_isMapped (t_glist *glist)
{
    return (!glist->gl_isLoading && glist_getView (glist)->gl_isMapped);
}

int glist_isRoot (t_glist *glist)
{
    int k = (!glist->gl_parent || glist_isAbstraction (glist));
    
    if (k) { PD_ASSERT (glist->gl_environment != NULL); }
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist->gl_parent && glist->gl_environment != NULL);
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

int glist_isGraph (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

/* Either a top window a subpacth or a graph-on-parent forced. */

int glist_canHaveWindow (t_glist *glist)
{
    return (glist->gl_hasWindow || !glist->gl_isGraphOnParent);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
