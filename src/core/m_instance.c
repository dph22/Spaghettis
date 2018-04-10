
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"
#include "../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pdinstance *pd_this;  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_new (void *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_setLogicalTime (t_systime);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *instance_contextGetCurrent (void)
{
    return cast_glist (instance_getBoundX());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_rootsAdd (t_glist *glist)
{
    glist_setNext (glist, instance_get()->pd_roots); instance_get()->pd_roots = glist;
}

void instance_rootsRemove (t_glist *glist)
{
    if (glist == instance_get()->pd_roots) { instance_get()->pd_roots = glist_getNext (glist); }
    else {
        t_glist *z = NULL;
        for (z = instance_get()->pd_roots; glist_getNext (z) != glist; z = glist_getNext (z)) { }
        glist_setNext (z, glist_getNext (glist));
    }
}

/* At release, close remaining (i.e. NOT dirty) and invisible patches. */

void instance_rootsFreeAll (void)
{    
    while (1) {

        t_glist *glist = instance_get()->pd_roots;
        
        if (glist == NULL) { break; }
        else {
            glist_closebang (glist); pd_free (cast_pd (glist)); 
            if (glist == instance_get()->pd_roots) { 
                PD_BUG; break;                                      /* Not removed? */
            }   
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int instance_dspDone (t_int *dummy)
{
    return 0;
}

static void instance_dspRelease (void)
{
    if (instance_get()->pd_dspChain != NULL) {
    //
    PD_MEMORY_FREE (instance_get()->pd_dspChain);
    
    instance_get()->pd_dspChain = NULL;
    //
    }
    
    instance_signalFreeAll();
}

static void instance_dspInitialize (void)
{
    instance_dspRelease();
    
    PD_ASSERT (instance_ugenGetContext() == NULL);
    PD_ASSERT (instance_get()->pd_dspChain == NULL);
    
    instance_get()->pd_dspChainSize = 1;
    instance_get()->pd_dspChain     = (t_int *)PD_MEMORY_GET (sizeof (t_int));
    instance_get()->pd_dspChain[0]  = (t_int)instance_dspDone;
    
    instance_incrementDspChainIdentifier();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void instance_dspTick (void)
{
    t_int *t = instance_getDspChain();
    
    if (t) {
    //
    while (t) { t = (*(t_perform)(*t))(t); } instance_incrementDspPhase();
        
    // PD_LOG ("#");
    // PD_LOG_NUMBER (instance_getDspPhase());
    // PD_LOG ("#");
    //
    }
}

void instance_dspStart (void)
{
    t_glist *glist;

    // PD_LOG ("START");
    
    instance_dspInitialize();
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) { 
        canvas_dspProceed (glist, 1, NULL); 
    }
}

void instance_dspStop (void)
{
    // PD_LOG ("STOP");
    
    instance_dspRelease();
}

void instance_dspChainAppend (t_perform f, int n, ...)
{
    int size = instance_get()->pd_dspChainSize + n + 1;
    
    PD_ASSERT (instance_get()->pd_dspChain != NULL);
    
    size_t newSize = sizeof (t_int) * size;
    size_t oldSize = sizeof (t_int) * instance_get()->pd_dspChainSize;
    
    // -- TODO: Is that resizing with big chunks worth the cost?
    
    instance_get()->pd_dspChain = (t_int *)PD_MEMORY_RESIZE (instance_get()->pd_dspChain, oldSize, newSize);
    
    {
    //
    int i;
    va_list ap;

    instance_get()->pd_dspChain[instance_get()->pd_dspChainSize - 1] = (t_int)f;

    va_start (ap, n);
    
    for (i = 0; i < n; i++) { 
        instance_get()->pd_dspChain[instance_get()->pd_dspChainSize + i] = va_arg (ap, t_int);
    }
    
    va_end (ap);
    //
    }
    
    instance_get()->pd_dspChain[size - 1] = (t_int)instance_dspDone;
    instance_get()->pd_dspChainSize = size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_signalAdd (t_signal *s)
{
    s->s_next = instance_get()->pd_signals;
    
    instance_get()->pd_signals = s;
}

void instance_signalFreeAll (void)
{
    t_signal *s = NULL;
    
    while ((s = instance_get()->pd_signals)) {
    //
    instance_get()->pd_signals = s->s_next;
    
    if (!s->s_hasBorrowed) { PD_MEMORY_FREE (s->s_vector); }
    else {
    //
    PD_ASSERT (s->s_unused); PD_MEMORY_FREE (s->s_unused);
    //
    }
    
    PD_MEMORY_FREE (s);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Enqueue the clock sorted up by logical time. */

void instance_clockAdd (t_clock *c)
{
    t_systime systime = clock_getLogicalTime (c);
    
    if (instance_get()->pd_clocks && clock_getLogicalTime (instance_get()->pd_clocks) <= systime) {
    
        t_clock *m = instance_get()->pd_clocks;
        t_clock *n = instance_get()->pd_clocks->c_next;
        
        while (m) {
            if (!n || clock_getLogicalTime (n) > systime) {
                m->c_next = c; c->c_next = n; return;
            }
            m = n;
            n = m->c_next;
        }
        
    } else {
        c->c_next = instance_get()->pd_clocks; instance_get()->pd_clocks = c;
    }
}

void instance_clockUnset (t_clock *c)
{
    if (c == instance_get()->pd_clocks) { instance_get()->pd_clocks = c->c_next; }
    else {
        t_clock *t = instance_get()->pd_clocks;
        while (t->c_next != c) { t = t->c_next; } t->c_next = c->c_next;
    }
}

void instance_clockTick (t_systime t)
{
    while (instance_get()->pd_clocks && clock_getLogicalTime (instance_get()->pd_clocks) < t) {
    //
    t_clock *c = instance_get()->pd_clocks;
    scheduler_setLogicalTime (clock_getLogicalTime (c));
    clock_unset (c);
    clock_execute (c);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_destroyAllScalarsByTemplate (t_template *tmpl)
{
    t_glist *glist = instance_get()->pd_roots;
    
    PD_ASSERT (tmpl);
    
    while (glist) {

        if (!template_isPrivate (template_getTemplateIdentifier (tmpl))) {
            glist_objectRemoveAllByTemplate (glist, tmpl); 
        }
    
        glist = glist_getNext (glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_setDefaultCoordinates (t_glist *glist, int a, int b)
{
    instance_get()->pd_locate.p_glist = glist;
    instance_get()->pd_locate.p_x     = a;
    instance_get()->pd_locate.p_y     = b;
}

int instance_getDefaultX (t_glist *glist)
{
    t_glist *t = instance_get()->pd_locate.p_glist;
    
    return ((t == glist) ? instance_get()->pd_locate.p_x : 40);
}

int instance_getDefaultY (t_glist *glist)
{
    t_glist *t = instance_get()->pd_locate.p_glist;
    
    return ((t == glist) ? instance_get()->pd_locate.p_y : 40);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Called if no method of the maker object match. */

static void instance_factory (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    instance_get()->pd_newest = NULL;
    
    /* Note that it can be a recursive call. */
    
    if (!instance_get()->pd_loadingExternal) {

        /* First search an external. */
        
        if (loader_load (instance_contextGetCurrent(), s)) {
            instance_get()->pd_loadingExternal = 1;
            pd_message (x, s, argc, argv);              /* Try again. */
            instance_get()->pd_loadingExternal = 0;
        
        /* Otherwise look for an abstraction. */
        
        } else {
            instance_loadAbstraction (s, argc, argv);
        }
        
    } else {
        error_canNotFind (sym_loader, sym_class);       /* External MUST provide a properly named class. */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_pdinstance *instance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    x->pd_stack.s_stack = (t_stackelement *)PD_MEMORY_GET (INSTANCE_STACK * sizeof (t_stackelement));
    
    x->pd_environment.env_directory = &s_;
    x->pd_environment.env_fileName  = &s_;
    
    x->pd_objectMaker = class_new (sym_objectmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    x->pd_canvasMaker = class_new (sym_canvasmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    
    class_addAnything (x->pd_objectMaker, (t_method)instance_factory);
        
    class_addMethod (x->pd_canvasMaker, (t_method)canvas_new, sym_canvas, A_GIMME, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (x->pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
    
    #endif
    
    return x;
}

static void instance_free (t_pdinstance *x)
{
    PD_ASSERT (x->pd_dspChain       == NULL);
    PD_ASSERT (x->pd_clocks         == NULL);
    PD_ASSERT (x->pd_signals        == NULL);
    PD_ASSERT (x->pd_roots          == NULL);
    PD_ASSERT (x->pd_polling        == NULL);
    PD_ASSERT (x->pd_autorelease    == NULL);
    
    class_free (x->pd_canvasMaker);
    class_free (x->pd_objectMaker);
    
    PD_ASSERT (x->pd_stack.s_stackIndex == 0);
    
    if (x->pd_environment.env_argv) { PD_MEMORY_FREE (x->pd_environment.env_argv); }
    
    PD_MEMORY_FREE (x->pd_stack.s_stack);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_initialize (void)
{
    pd_this = instance_new();
    
    instance_setBoundN (&(instance_get()->pd_canvasMaker));
}

void instance_release (void)
{
    instance_setBoundA (NULL);
    instance_setBoundN (NULL);
    instance_setBoundX (NULL);
    
    instance_free (pd_this);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------