
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabplay_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabplay_tilde {
    t_object    x_obj;                      /* Must be the first. */
    int         x_phase;
    int         x_end;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_clock     *x_clock;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_tabplay_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabplay_tilde_task (t_tabplay_tilde *x)
{
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabplay_tilde_list (t_tabplay_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    int start  = (int)atom_getFloatAtIndex (0, argc, argv);
    int length = (int)atom_getFloatAtIndex (1, argc, argv);
    
    x->x_phase = PD_MAX (0, start);
    x->x_end   = (length > 0) ? x->x_phase + length : PD_INT_MAX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabplay_tilde_set (t_tabplay_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabplay__tilde__);
}

static void tabplay_tilde_stop (t_tabplay_tilde *x)
{
    x->x_phase = PD_INT_MAX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *tabplay_tilde_perform (t_int *w)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    int phase = x->x_phase;
    int end   = PD_MIN (x->x_size, x->x_end);

    if (x->x_vector && phase < end) {
    //
    t_word *data = x->x_vector + phase;
    int size = PD_MIN (n, end - phase);
    int pad  = n - size;
    
    phase += size;
    
    while (size--) { *out++ = (t_sample)WORD_FLOAT (data); data++; }
    while (pad--)  { *out++ = 0.0; }
            
    if (phase >= end) { clock_delay (x->x_clock, 0.0); x->x_phase = PD_INT_MAX; }
    else { 
        x->x_phase = phase;
    }
    //
    } else { while (n--) { *out++ = 0.0; } }
    
    return (w + 4);
}

static void tabplay_tilde_dsp (t_tabplay_tilde *x, t_signal **sp)
{
    tabplay_tilde_set (x, x->x_name);
    
    dsp_add (tabplay_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabplay_tilde_new (t_symbol *s)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)pd_new (tabplay_tilde_class);
    
    x->x_phase       = PD_INT_MAX;
    x->x_name        = s;
    x->x_clock       = clock_new ((void *)x, (t_method)tabplay_tilde_task);
    x->x_outletLeft  = outlet_newSignal (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    return x;
}

static void tabplay_tilde_free (t_tabplay_tilde *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabplay_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabplay__tilde__,
            (t_newmethod)tabplay_tilde_new,
            (t_method)tabplay_tilde_free,
            sizeof (t_tabplay_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addDSP (c, (t_method)tabplay_tilde_dsp);
    
    class_addList (c, (t_method)tabplay_tilde_list);
        
    class_addMethod (c, (t_method)tabplay_tilde_set,    sym_set,    A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabplay_tilde_stop,   sym_stop,   A_NULL);
    
    tabplay_tilde_class = c;
}

void tabplay_tilde_destroy (void)
{
    class_free (tabplay_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------