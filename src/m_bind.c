
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *bindlist_class;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _bindelement {
    t_pd                    *e_what;        /* MUST be the first. */
    struct _bindelement     *e_next;
    } t_bindelement;

typedef struct _bindlist {
    t_pd                    b_pd;           /* MUST be the first. */
    t_bindelement           *b_cached;
    t_bindelement           *b_list;
    } t_bindlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that binding or/and unbinding might be possible while broadcasting. */
/* It means therefore that a receiver can unbound itself also during the sender call. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int bindlist_isEmpty (t_bindlist *x)
{
    return (x->b_list == NULL);
}

static t_bindelement *bindlist_traverseStart (t_bindlist *x)
{
    x->b_cached = NULL;
    
    if (x->b_list) { x->b_cached = x->b_list->e_next; }
    
    return x->b_list;
}

static t_bindelement *bindlist_traverseNext (t_bindlist *x)
{
    t_bindelement *e = x->b_cached;
    
    if (e) { x->b_cached = e->e_next; }
    
    return e;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bindlist_bang (t_bindlist *x)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_bang (e->e_what); e = bindlist_traverseNext (x); }
}

static void bindlist_float (t_bindlist *x, t_float f)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_float (e->e_what, f); e = bindlist_traverseNext (x); }
}

static void bindlist_symbol (t_bindlist *x, t_symbol *s)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_symbol (e->e_what, s); e = bindlist_traverseNext (x); }
}

static void bindlist_pointer (t_bindlist *x, t_gpointer *gp)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_pointer (e->e_what, gp); e = bindlist_traverseNext (x); }
}

static void bindlist_list (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_list (e->e_what, argc, argv); e = bindlist_traverseNext (x); }
}

static void bindlist_anything (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = bindlist_traverseStart (x);
    while (e) { pd_message (e->e_what, s, argc, argv); e = bindlist_traverseNext (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bindlist_free (t_bindlist *x)
{
    PD_ASSERT (bindlist_isEmpty (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bindlist_initialize (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bindlist,
            NULL,
            (t_method)bindlist_free,
            sizeof (t_bindlist),
            CLASS_NOBOX,
            A_NULL);
    
    class_addBang (c, (t_method)bindlist_bang);
    class_addFloat (c, (t_method)bindlist_float);
    class_addSymbol (c, (t_method)bindlist_symbol);
    class_addPointer (c, (t_method)bindlist_pointer);
    class_addList (c, (t_method)bindlist_list);
    class_addAnything (c, (t_method)bindlist_anything);
    
    bindlist_class = c;
}

void bindlist_release (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bind (t_pd *x, t_symbol *s)
{
    PD_ASSERT (s != &s__A);
    PD_ASSERT (s != &s__N);
    PD_ASSERT (s != &s__X);
    
    if (s->s_thing) {
    
        if (pd_class (s->s_thing) == bindlist_class) {
            t_bindlist *b = (t_bindlist *)s->s_thing;
            t_bindelement *e = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            e->e_next = b->b_list;
            e->e_what = x;
            b->b_list = e;
            
        } else {
            t_bindlist *b = (t_bindlist *)pd_new (bindlist_class);
            t_bindelement *e1 = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            t_bindelement *e2 = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            b->b_list  = e1;
            e1->e_what = x;
            e1->e_next = e2;
            e2->e_what = s->s_thing;
            e2->e_next = NULL;
            s->s_thing = &b->b_pd;
        }
        
    } else {
        s->s_thing = x;
    }
}

void pd_unbind (t_pd *x, t_symbol *s)
{
    if (s->s_thing == x) { 
        s->s_thing = NULL; 
        
    } else if (s->s_thing && pd_class (s->s_thing) == bindlist_class) {
        
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelement *e1 = NULL;
        t_bindelement *e2 = NULL;
        
        if ((e1 = b->b_list)->e_what == x) {
            b->b_list = e1->e_next;
            PD_MEMORY_FREE (e1);
        } else {
            for (e1 = b->b_list; e2 = e1->e_next; e1 = e2) {
                if (e2->e_what == x) {
                    e1->e_next = e2->e_next;
                    if (e2 == b->b_cached) { b->b_cached = e2->e_next; }
                    PD_MEMORY_FREE (e2);
                    break;
                }
            }
        }
        
    } else { PD_BUG; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pd_isThingError (t_symbol *s)
{  
    if (s) {
    if (!string_startWith (s->s_name, PD_GUISTUB)) {
    if (!string_startWith (s->s_name, ".x")) {
        error_noSuch (s, sym_object);
    }
    }
    }
}

static int pd_isThingPerform (t_symbol *s, int withError)
{
    int k = 0;
    
    if (s && s->s_thing) {
    //
    if (pd_class (s->s_thing) == bindlist_class) { k = !bindlist_isEmpty ((t_bindlist *)s->s_thing); }
    else {
        k = 1;
    }
    //
    }
    
    if (withError && !k) { pd_isThingError (s); }
    
    return k;
}

int pd_isThingQuiet (t_symbol *s)
{
    return pd_isThingPerform (s, 0);
}

int pd_isThing (t_symbol *s)
{
    return pd_isThingPerform (s, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *pd_getThingByClass (t_symbol *s, t_class *c)
{
    t_pd *x = NULL;
    
    if (!s->s_thing) { return NULL; }
    if (pd_class (s->s_thing) == c) { return s->s_thing; }
    
    if (pd_class (s->s_thing) == bindlist_class) {
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelement *e = NULL;
        
        for (e = b->b_list; e; e = e->e_next) {
            if (*e->e_what == c) { PD_ASSERT (x == NULL); x = e->e_what; }
        }
    }
    
    return x;
}

t_pd *pd_getThing (t_symbol *s)
{
    if (pd_isThingQuiet (s)) { return s->s_thing; }
    else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
