
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_stackPush (t_pd *x)
{
    t_stack *p = (t_stack *)PD_MEMORY_GET (sizeof (t_stack));
    p->g_what = instance_getBoundX();
    p->g_next = instance_get()->pd_stackHead;
    p->g_abstraction = instance_get()->pd_loadingAbstraction;
    instance_get()->pd_loadingAbstraction = NULL;
    instance_get()->pd_stackHead = p;
    instance_setBoundX (x);
}

void instance_stackPop (t_pd *x)
{
    if (!instance_get()->pd_stackHead || instance_getBoundX() != x) { PD_BUG; }
    else {
        t_stack *p = instance_get()->pd_stackHead;
        instance_setBoundX (p->g_what);
        instance_get()->pd_stackHead = p->g_next;
        PD_MEMORY_FREE (p);
        instance_get()->pd_stackPopped = x;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void stack_proceedLoadbang (void)
{
    if (instance_get()->pd_stackPopped) { 
        pd_message (instance_get()->pd_stackPopped, sym_loadbang, 0, NULL); 
    }
    
    instance_get()->pd_stackPopped = NULL;
}

int stack_setLoadingAbstraction (t_symbol *s)
{
    t_stack *p = instance_get()->pd_stackHead;
    
    for (p = instance_get()->pd_stackHead; p; p = p->g_next) {
        if (p->g_abstraction == s) { return 1; }
    }
    
    instance_get()->pd_loadingAbstraction = s;
    
    return 0;
}

static void instance_popAbstraction (t_glist *glist)
{
    instance_get()->pd_newest = cast_pd (glist);
    instance_stackPop (cast_pd (glist));
    
    glist->gl_isLoading = 0;
    
    canvas_resortInlets (glist);
    canvas_resortOutlets (glist);
}

void instance_loadAbstraction (t_symbol *s, int argc, t_atom *argv)
{
    char directory[PD_STRING] = { 0 }; char *name = NULL;
    
    if (canvas_fileFind (canvas_getCurrent(), s->s_name, PD_PATCH, directory, &name, PD_STRING)) {
    //
    t_symbol *f = gensym (name);
    
    if (stack_setLoadingAbstraction (f)) { error_recursiveInstantiation (f); }
    else {
        t_pd *t = instance_getBoundX();
        environment_setActiveArguments (argc, argv);
        buffer_fileEval (f, gensym (directory));
        if (instance_getBoundX() && t != instance_getBoundX()) { 
            instance_popAbstraction (cast_glist (instance_getBoundX())); 
        } else { 
            instance_setBoundX (t); 
        }
        environment_resetActiveArguments();
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
