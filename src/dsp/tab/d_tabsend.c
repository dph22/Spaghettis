
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

#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabsend_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabsend_tilde {
    t_object            x_obj;              /* Must be the first. */
    pthread_mutex_t     x_mutex;
    t_int32Atomic       x_redraw;
    int                 x_dismissed;
    int                 x_set;
    int                 x_size;
    t_word              *x_vector;
    t_symbol            *x_name;
    } t_tabsend_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabsend_tilde_dismiss (t_tabsend_tilde *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabsend_tilde_polling (t_tabsend_tilde *x)
{
    int n = PD_ATOMIC_INT32_READ (&x->x_redraw);
    
    if (n > 0) {
    //
    t_garray *a = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
    if (a) { garray_redraw (a); }

    while (n--) { PD_ATOMIC_INT32_DECREMENT (&x->x_redraw); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabsend_tilde_setProceed (t_tabsend_tilde *x, t_symbol *s, int verbose)
{
    tabsend_tilde_polling (x);
    
    pthread_mutex_lock (&x->x_mutex);
    
        t_error err = tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector);
    
        x->x_set = 1;
    
    pthread_mutex_unlock (&x->x_mutex);
    
    if (verbose && err) { tab_error (sym_tabsend__tilde__, s); }
}

static void tabsend_tilde_set (t_tabsend_tilde *x, t_symbol *s)
{
    tabsend_tilde_setProceed (x, s, 1);
}

static void tabsend_tilde_restore (t_tabsend_tilde *x, t_symbol *s)
{
    tabsend_tilde_setProceed (x, s, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *tabsend_tilde_perform (t_int *w)
{
    t_tabsend_tilde *x = (t_tabsend_tilde *)(w[1]);
    PD_RESTRICTED in   = (t_sample *)(w[2]);
    t_space *t         = (t_space *)(w[3]);
    int n = (int)w[4];
    
    if (pthread_mutex_trylock (&x->x_mutex) == 0) {
    //
    if (x->x_set) {
        t->s_int0     = x->x_size;
        t->s_pointer0 = (void *)x->x_vector;
        x->x_set      = 0;
    }
    
    pthread_mutex_unlock (&x->x_mutex);
    //
    }
    
    if (t->s_pointer0) {
    //
    t_word *data = (t_word *)t->s_pointer0;
    
    n = PD_MIN (t->s_int0, n);

    while (n--) {
    //  
    t_sample f = *in++;
    if (PD_FLOAT32_IS_BIG_OR_SMALL (f)) { f = 0.0; }
    w_setFloat (data, (t_float)f);
    data++;
    //
    }
    
    PD_ATOMIC_INT32_INCREMENT (&x->x_redraw);
    //
    }

    return (w + 5);
}

static void tabsend_tilde_dsp (t_tabsend_tilde *x, t_signal **sp)
{
    if (dsp_objectNeedInitializer (cast_gobj (x))) {
    //
    t_tabsend_tilde *old = (t_tabsend_tilde *)garbage_fetch (cast_gobj (x));
    
    if (old) {
    //
    if (x->x_name != old->x_name) { tabsend_tilde_setProceed (x, old->x_name, 1); }
    
    object_copySignalValues (cast_object (x), cast_object (old));
    //
    }
    //
    }
    
    {
    //
    t_space *t  = space_new (cast_gobj (x));
    int size    = 0;
    t_word *w   = NULL;
    t_error err = tab_fetchArray (x->x_name, &size, &w);

    if (err) { tab_error (sym_tabsend__tilde__, x->x_name); }
    else {
        t->s_int0 = size; t->s_pointer0 = (void *)w;
    }
    
    dsp_add4 (tabsend_tilde_perform, x, sp[0]->s_vector, t, sp[0]->s_vectorSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *tabsend_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_tabsend_tilde *x = (t_tabsend_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, x->x_name);
    buffer_appendComma (b);
    object_getSignalValues (cast_object (x), b);
    
    return b;
    //
    }
    
    return NULL;
}

static void tabsend_tilde_functionDismiss (t_gobj *z)
{
    tabsend_tilde_dismiss ((t_tabsend_tilde *)z);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabsend_tilde_new (t_symbol *s)
{
    t_tabsend_tilde *x = (t_tabsend_tilde *)pd_new (tabsend_tilde_class);
    
    pthread_mutex_init (&x->x_mutex, NULL);
    
    x->x_name = s;
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void tabsend_tilde_dismiss (t_tabsend_tilde *x)
{
    if (!x->x_dismissed) {
    //
    x->x_dismissed = 1;
    
    instance_pollingUnregister (cast_pd (x));
    //
    }
}

static void tabsend_tilde_free (t_tabsend_tilde *x)
{
    tabsend_tilde_dismiss (x);
    
    pthread_mutex_destroy (&x->x_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabsend_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabsend__tilde__,
            (t_newmethod)tabsend_tilde_new,
            (t_method)tabsend_tilde_free,
            sizeof (t_tabsend_tilde),
            CLASS_DEFAULT | CLASS_SIGNAL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addDSP (c, (t_method)tabsend_tilde_dsp);
    class_addPolling (c, (t_method)tabsend_tilde_polling);
    
    class_addMethod (c, (t_method)tabsend_tilde_set,        sym_set,        A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabsend_tilde_restore,    sym__restore,   A_SYMBOL, A_NULL);

    class_setDataFunction (c, tabsend_tilde_functionData);
    class_setDismissFunction (c, tabsend_tilde_functionDismiss);

    tabsend_tilde_class = c;
}

void tabsend_tilde_destroy (void)
{
    class_free (tabsend_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
