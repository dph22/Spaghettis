/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  miscellaneous: print~; more to come.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"
#include <stdio.h>
#include <string.h>

static t_class *bang_tilde_class;

typedef struct _bang_tilde
{
    t_object x_obj;
    t_clock *x_clock;
} t_bang_tilde;

static t_int *bang_tilde_perform(t_int *w)
{
    t_bang_tilde *x = (t_bang_tilde *)(w[1]);
    clock_delay(x->x_clock, 0);
    return (w+2);
}

static void bang_tilde_dsp(t_bang_tilde *x, t_signal **sp)
{
    dsp_add(bang_tilde_perform, 1, x);
}

static void bang_tilde_tick(t_bang_tilde *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

static void bang_tilde_free(t_bang_tilde *x)
{
    clock_free(x->x_clock);
}

static void *bang_tilde_new(t_symbol *s)
{
    t_bang_tilde *x = (t_bang_tilde *)pd_new(bang_tilde_class);
    x->x_clock = clock_new(x, (t_method)bang_tilde_tick);
    outlet_new(&x->x_obj, &s_bang);
    return (x);
}

void bang_tilde_setup(void)
{
    bang_tilde_class = class_new(sym_bang__tilde__, (t_newmethod)bang_tilde_new,
        (t_method)bang_tilde_free, sizeof(t_bang_tilde), 0, 0);
    class_addMethod(bang_tilde_class, (t_method)bang_tilde_dsp,
        sym_dsp, A_CANT, A_NULL);
}






