/* Copyright (c) 1997-2013 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* The "scalar" object. */

#include "m_pd.h"
#include "m_macros.h"
#include "g_canvas.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

extern t_pd *pd_newest;
extern t_class *scalar_class;

t_class *scalar_define_class;

static void *scalar_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_atom a[9];
    t_glist *gl;
    t_canvas *x, *z = canvas_getcurrent();
    t_symbol *templatesym = &s_float, *asym = gensym("#A");
    t_template *template;
    t_scalar *sc;
    int keep = 0;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            keep = 1;
        else
        {
            post_error ("scalar define: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        templatesym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: scalar define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    
        /* make a canvas... */
    SET_FLOAT(a, 0);
    SET_FLOAT(a+1, 50);
    SET_FLOAT(a+2, 600);
    SET_FLOAT(a+3, 400);
    SET_SYMBOL(a+4, s);
    SET_FLOAT(a+5, 0);
    x = canvas_new(0, 0, 6, a);

    x->gl_owner = z;
    x->gl_private = 0;
        /* put a scalar in it */
    template = template_findbyname(canvas_makebindsym(templatesym));
    if (!template)
    {
        post_error ("scalar define: couldn't find template %s",
            templatesym->s_name);
        goto noscalar;
    }
    sc = scalar_new(x, canvas_makebindsym(templatesym));
    if (!sc)
    {
        post_error ("%s: couldn't create scalar", templatesym->s_name);
        goto noscalar;
    }
    sc->sc_g.g_next = 0;
    x->gl_list = &sc->sc_g;
    x->gl_private = keep;
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever text_define or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(&x->gl_obj.te_g.g_pd, asym); 
noscalar:
    pd_newest = &x->gl_obj.te_g.g_pd;     /* mimic action of canvas_pop() */
    pd_pop(&x->gl_obj.te_g.g_pd);
    x->gl_loading = 0;
    
        /* bash the class to "scalar define" -- see comment in x_array,c */
    x->gl_obj.te_g.g_pd = scalar_define_class;
    return (x);
}

    /* send a pointer to the scalar to whomever is bound to the symbol */
static void scalar_define_send(t_glist *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("scalar_define_send: %s: no such object", s->s_name);
    else if (x->gl_list && pd_class(&x->gl_list->g_pd) == scalar_class)
    {
        t_gpointer gp;
        gpointer_init(&gp);
        gpointer_setglist(&gp, x, (t_scalar *)&x->gl_list->g_pd);
        pd_pointer(s->s_thing, &gp);
        gpointer_unset(&gp);
    }
    else { PD_BUG; }
}

    /* set to a list, used to restore from scalar_define_save()s below */
static void scalar_define_set(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->gl_list && pd_class(&x->gl_list->g_pd) == scalar_class)
    {
        t_binbuf *b = binbuf_new();
        int nextmsg = 0, natoms;
        t_atom *vec;
        glist_clear(x);
        binbuf_restore(b, argc, argv);
        natoms = binbuf_getnatom(b);
        vec = binbuf_getvec(b);
        canvas_readscalar(x, natoms, vec, &nextmsg, 0);
        binbuf_free(b);
    }
    else { PD_BUG; }
}

    /* save to a binbuf (for file save or copy) */
static void scalar_define_save(t_gobj *z, t_binbuf *bb)
{
    t_glist *x = (t_glist *)z;
    binbuf_addv(bb, "ssff", &s__X, gensym("obj"),
        (float)x->gl_obj.te_xpix, (float)x->gl_obj.te_ypix);
    binbuf_addbinbuf(bb, x->gl_obj.te_binbuf);
    binbuf_addsemi(bb);
    if (x->gl_private && x->gl_list &&
        pd_class(&x->gl_list->g_pd) == scalar_class)
    {
        t_binbuf *b2 = binbuf_new();
        t_scalar *sc = (t_scalar *)(x->gl_list);
        binbuf_addv(bb, "ss", gensym("#A"), gensym("set"));
        canvas_writescalar(sc->sc_template, sc->sc_vec, b2, 0);
        binbuf_addbinbuf(bb, b2);
        binbuf_addsemi(bb);
        binbuf_free(b2);
    }
}

/* overall creator for "scalar" objects - dispatch to "scalar define" etc */
static void *scalarobj_new(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = scalar_define_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = scalar_define_new(s, argc-1, argv+1);
        else 
        {
            post_error ("scalar %s: unknown function", str);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}

void canvas_add_for_class(t_class *c);

/* ---------------- global setup function -------------------- */

void x_scalar_setup(void )
{
    scalar_define_class = class_new(gensym("scalar define"), 0,
        (t_method)canvas_free, sizeof(t_canvas), 0, 0);
    canvas_add_for_class(scalar_define_class);
    class_addMethod(scalar_define_class, (t_method)scalar_define_send,
        gensym("send"), A_SYMBOL, 0);
    class_addMethod(scalar_define_class, (t_method)scalar_define_set,
        gensym("set"), A_GIMME, 0);
    class_setHelpName(scalar_define_class, gensym("scalar"));
    class_setSaveFunction(scalar_define_class, scalar_define_save);

    class_addCreator((t_newmethod)scalarobj_new, gensym("scalar"), A_GIMME, 0);

}
