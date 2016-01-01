/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>      /* for read/write to files */
#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"
#include "g_canvas.h"
#include <math.h>

extern t_pd pd_canvasMaker;
extern int glist_valid;

/* jsarlo { */
#define ARRAYPAGESIZE 1000  /* this should match the page size in u_main.tk */
/* } jsarlo */

/* --------- "pure" arrays with scalars for elements. --------------- */

/* Pure arrays have no a priori graphical capabilities.
They are instantiated by "garrays" below or can be elements of other
scalars (g_scalar.c); their graphical behavior is defined accordingly. */

t_array *array_new(t_symbol *templatesym, t_gpointer *parent)
{
    t_array *x = (t_array *)PD_MEMORY_GET(sizeof (*x));
    t_template *template;
    t_gpointer *gp;
    template = template_findbyname(templatesym);
    x->a_templatesym = templatesym;
    x->a_n = 1;
    x->a_elemsize = sizeof(t_word) * template->t_n;
    x->a_vec = (char *)PD_MEMORY_GET(x->a_elemsize);
        /* note here we blithely copy a gpointer instead of "setting" a
        new one; this gpointer isn't accounted for and needn't be since
        we'll be deleted before the thing pointed to gets deleted anyway;
        see array_free. */
    x->a_gp = *parent;
    x->a_stub = gstub_new(0, x);
    word_init((t_word *)(x->a_vec), template, parent);
    return (x);
}

/* jsarlo { */
void garray_arrayviewlist_close(t_garray *x);
/* } jsarlo */

void array_resize(t_array *x, int n)
{
    int elemsize, oldn;
    t_gpointer *gp;
    t_template *template = template_findbyname(x->a_templatesym);
    if (n < 1)
        n = 1;
    oldn = x->a_n;
    elemsize = sizeof(t_word) * template->t_n;

    x->a_vec = (char *)PD_MEMORY_RESIZE(x->a_vec, oldn * elemsize, n * elemsize);
    x->a_n = n;
    if (n > oldn)
    {
        char *cp = x->a_vec + elemsize * oldn;
        int i = n - oldn;
        for (; i--; cp += elemsize)
        {
            t_word *wp = (t_word *)cp;
            word_init(wp, template, &x->a_gp);
        }
    }
    x->a_valid = ++glist_valid;
}

void array_resize_and_redraw(t_array *array, t_glist *glist, int n)
{
    t_array *a2 = array;
    int vis = glist_isvisible(glist);
    while (a2->a_gp.gp_stub->gs_type == POINTER_ARRAY)
        a2 = a2->a_gp.gp_stub->gs_un.gs_array;
    if (vis)
        gobj_vis(&a2->a_gp.gp_un.gp_scalar->sc_g, glist, 0);
    array_resize(array, n);
    if (vis)
        gobj_vis(&a2->a_gp.gp_un.gp_scalar->sc_g, glist, 1);
}

void word_free(t_word *wp, t_template *template);

void array_free(t_array *x)
{
    int i;
    t_template *scalartemplate = template_findbyname(x->a_templatesym);
    gstub_cutoff(x->a_stub);
    for (i = 0; i < x->a_n; i++)
    {
        t_word *wp = (t_word *)(x->a_vec + x->a_elemsize * i);
        word_free(wp, scalartemplate);
    }
    PD_MEMORY_FREE(x->a_vec, x->a_elemsize * x->a_n);
    PD_MEMORY_FREE(x, sizeof *x);
}

/* --------------------- graphical arrays (garrays) ------------------- */

t_class *garray_class;      /* Shared. */

struct _garray
{
    t_gobj x_gobj;
    t_scalar *x_scalar;     /* scalar "containing" the array */
    t_glist *x_glist;       /* containing glist */
    t_symbol *x_name;       /* unexpanded name (possibly with leading '$') */
    t_symbol *x_realname;   /* expanded name (symbol we're bound to) */
    char x_usedindsp;       /* true if some DSP routine is using this */
    char x_saveit;          /* true if we should save this with parent */
    char x_listviewing;     /* true if list view window is open */
    char x_hidename;        /* don't print name above graph */
};

static t_pd *garray_arraytemplatecanvas;
static char garray_arraytemplatefile[] = "\
canvas 0 0 458 153 10;\n\
#X obj 43 31 struct float-array array z float float style\n\
float linewidth float color;\n\
#X obj 43 70 plot z color linewidth 0 0 1 style;\n\
";
static char garray_floattemplatefile[] = "\
canvas 0 0 458 153 10;\n\
#X obj 39 26 struct float float y;\n\
";

/* create invisible, built-in canvases to supply templates for floats
and float-arrays. */

void garray_init( void)
{
    t_binbuf *b;
    if (garray_arraytemplatecanvas)
        return;
    b = binbuf_new();
    
    glob_setfilename(0, gensym("_float_template"), gensym("."));
    binbuf_text(b, garray_floattemplatefile, strlen(garray_floattemplatefile));
    binbuf_eval(b, &pd_canvasMaker, 0, 0);
    pd_variadicMessage(s__X.s_thing, gensym("pop"), "i", 0);
    
    glob_setfilename(0, gensym("_float_array_template"), gensym("."));
    binbuf_text(b, garray_arraytemplatefile, strlen(garray_arraytemplatefile));
    binbuf_eval(b, &pd_canvasMaker, 0, 0);
    garray_arraytemplatecanvas = s__X.s_thing;
    pd_variadicMessage(s__X.s_thing, gensym("pop"), "i", 0);

    glob_setfilename(0, &s_, &s_);
    binbuf_free(b);  
}

/* create a new scalar attached to a symbol.  Used to make floating-point
arrays (the scalar will be of type "float-array").  Currently this is
always called by graph_array() below; but when we make a more general way
to save and create arrays this might get called more directly. */

static t_garray *graph_scalar(t_glist *gl, t_symbol *s, t_symbol *templatesym,
    int saveit)
{
    int i, zz;
    t_garray *x;
    t_pd *x2;
    t_template *template;
    char *str;
    t_gpointer gp;
    if (!template_findbyname(templatesym))
        return (0);  
    x = (t_garray *)pd_new(garray_class);
    x->x_scalar = scalar_new(gl, templatesym);
    x->x_name = s;
    x->x_realname = canvas_realizedollar(gl, s);
    pd_bind(&x->x_gobj.g_pd, x->x_realname);
    x->x_usedindsp = 0;
    x->x_saveit = saveit;
    x->x_listviewing = 0;
    glist_add(gl, &x->x_gobj);
    x->x_glist = gl;
    return (x);
}

    /* get a garray's "array" structure. */
t_array *garray_getarray(t_garray *x)
{
    int nwords, zonset, ztype;
    t_symbol *zarraytype;
    t_scalar *sc = x->x_scalar;
    t_symbol *templatesym = sc->sc_template;
    t_template *template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("array: couldn't find template %s", templatesym->s_name);
        return (0);
    }
    if (!template_find_field(template, gensym("z"), 
        &zonset, &ztype, &zarraytype))
    {
        post_error ("array: template %s has no 'z' field", templatesym->s_name);
        return (0);
    }
    if (ztype != DATA_ARRAY)
    {
        post_error ("array: template %s, 'z' field is not an array",
            templatesym->s_name);
        return (0);
    }
    return (sc->sc_vector[zonset].w_array);
}

    /* get the "array" structure and furthermore check it's float */
static t_array *garray_getarray_floatonly(t_garray *x,
    int *yonsetp, int *elemsizep)
{
    t_array *a = garray_getarray(x);
    int yonset, type;
    t_symbol *arraytype;
    t_template *template = template_findbyname(a->a_templatesym);
    if (!template_find_field(template, gensym("y"), &yonset,
        &type, &arraytype) || type != DATA_FLOAT)
            return (0);
    *yonsetp = yonset;
    *elemsizep = a->a_elemsize;
    return (a);
}

    /* get the array's name.  Return nonzero if it should be hidden */
int garray_getname(t_garray *x, t_symbol **namep)
{
    *namep = x->x_name;
    return (x->x_hidename);
}

    /* get a garray's containing glist */
t_glist *garray_getglist(t_garray *x)
{
    return (x->x_glist);
}

    /* get a garray's associated scalar */
t_scalar *garray_getscalar(t_garray *x)
{
    return (x->x_scalar);
}

        /* if there is one garray in a graph, reset the graph's coordinates
            to fit a new size and style for the garray */
static void garray_fittograph(t_garray *x, int n, int style)
{
    t_array *array = garray_getarray(x);
    t_glist *gl = x->x_glist;
    if (gl->gl_list == &x->x_gobj && !x->x_gobj.g_next)
    {
        pd_variadicMessage(&gl->gl_obj.te_g.g_pd, gensym("bounds"), "ffff",
            0., gl->gl_y1, (double)
                (style == PLOT_POINTS || n == 1 ? n : n-1),
                    gl->gl_y2);
        
            /* hack - if the xlabels seem to want to be from 0 to table size-1,
            update the second label */
        if (gl->gl_nxlabels == 2 && !strcmp(gl->gl_xlabel[0]->s_name, "0"))
        {
            t_atom a;
            SET_FLOAT(&a, n-1);
            gl->gl_xlabel[1] = atom_gensym(&a);
            glist_redraw(gl);
        }
            /* close any dialogs that might have the wrong info now... */
        gfxstub_deleteforkey(gl);
    }
}

/* handle "array" message to glists; call graph_scalar above with
an appropriate template; then set size and flags.  This is called
from the menu and in the file format for patches.  LATER replace this
by a more coherent (and general) invocation. */

t_garray *graph_array(t_glist *gl, t_symbol *s, t_symbol *templateargsym,
    t_floatarg fsize, t_floatarg fflags)
{
    int n = fsize, i, zz, nwords, zonset, ztype, saveit;
    t_symbol *zarraytype, *asym = gensym("#A");
    t_garray *x;
    t_pd *x2;
    t_template *template, *ztemplate;
    t_symbol *templatesym;
    char *str;
    int flags = fflags;
    t_gpointer gp;
    int filestyle = ((flags & 6) >> 1);
    int style = (filestyle == 0 ? PLOT_POLYGONS :
        (filestyle == 1 ? PLOT_POINTS : filestyle));
    if (templateargsym != &s_float)
    {
        post_error ("array %s: only 'float' type understood", templateargsym->s_name);
        return (0);
    }
    templatesym = gensym("pd-float-array");
    template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("array: couldn't find template %s", templatesym->s_name);
        return (0);
    }
    if (!template_find_field(template, gensym("z"), 
        &zonset, &ztype, &zarraytype))
    {
        post_error ("array: template %s has no 'z' field", templatesym->s_name);
        return (0);
    }
    if (ztype != DATA_ARRAY)
    {
        post_error ("array: template %s, 'z' field is not an array",
            templatesym->s_name);
        return (0);
    }
    if (!(ztemplate = template_findbyname(zarraytype)))
    {
        post_error ("array: no template of type %s", zarraytype->s_name);
        return (0);
    }
    saveit = ((flags & 1) != 0);
    x = graph_scalar(gl, s, templatesym, saveit);
    x->x_hidename = ((flags & 8) >> 3);

    if (n <= 0)
        n = 100;
    array_resize(x->x_scalar->sc_vector[zonset].w_array, n);

    template_setfloat(template, gensym("style"), x->x_scalar->sc_vector,
        style, 1);
    template_setfloat(template, gensym("linewidth"), x->x_scalar->sc_vector, 
        ((style == PLOT_POINTS) ? 2 : 1), 1);

           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever textobj or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(&x->x_gobj.g_pd, asym); 

    garray_redraw(x);
    canvas_update_dsp();
    return (x);
}

    /* called from array menu item to create a new one */
void canvas_menuarray(t_glist *canvas)
{
    t_glist *x = (t_glist *)canvas;
    int gcount;
    char cmdbuf[200], arraybuf[80];
    for (gcount = 1; gcount < 1000; gcount++)
    {
        sprintf(arraybuf, "array%d", gcount);
        if (!pd_findByClass(gensym(arraybuf), garray_class))
            break;
    }
    sprintf(cmdbuf, "::ui_array::show %%s array%d 100 3\n", gcount);
    gfxstub_new(&x->gl_obj.te_g.g_pd, x, cmdbuf);
}

    /* called from graph_dialog to set properties */
void garray_properties(t_garray *x)
{
    char cmdbuf[200];
    t_array *a = garray_getarray(x);
    t_scalar *sc = x->x_scalar;
    int style = template_getfloat(template_findbyname(sc->sc_template),
        gensym("style"), x->x_scalar->sc_vector, 1);
    int filestyle = (style == 0 ? PLOT_POLYGONS :
        (style == 1 ? PLOT_POINTS : style));

    if (!a)
        return;
    gfxstub_deleteforkey(x);
        /* create dialog window.  LATER fix this to escape '$'
        properly; right now we just detect a leading '$' and escape
        it.  There should be a systematic way of doing this. */
    sprintf(cmdbuf, "::ui_array::show %%s %s %d %d\n",
            iem_dollar2raute(x->x_name)->s_name, a->a_n, x->x_saveit + 
            2 * filestyle);
    gfxstub_new(&x->x_gobj.g_pd, x, cmdbuf);
}

    /* this is called back from the dialog window to create a garray. 
    The otherflag requests that we find an existing graph to put it in. */
void glist_arraydialog(t_glist *parent, t_symbol *name, t_floatarg size,
    t_floatarg fflags)
{
    t_glist *gl;
    t_garray *a;
    int flags = fflags;
    if (size < 1)
        size = 1;
    if (0 /* otherflag */ == 0 || (!(gl = glist_findgraph(parent))))
        gl = glist_addglist(parent, &s_, 0, 1,
            size, -1, 0, 0, 0, 0);
    a = graph_array(gl, iem_raute2dollar(name), &s_float, size, flags);
    canvas_dirty(parent, 1);
}

    /* this is called from the properties dialog window for an existing array */
void garray_arraydialog(t_garray *x, t_symbol *name, t_floatarg fsize,
    t_floatarg fflags)
{
    int flags = fflags;
    int saveit = ((flags & 1) != 0);
    int filestyle = ((flags & 6) >> 1);
    int style = (filestyle == 0 ? PLOT_POLYGONS :
        (filestyle == 1 ? PLOT_POINTS : filestyle));
    t_float stylewas = template_getfloat(
        template_findbyname(x->x_scalar->sc_template),
            gensym("style"), x->x_scalar->sc_vector, 1);
    if (0) // deleteit
    {
        int wasused = x->x_usedindsp;
        glist_delete(x->x_glist, &x->x_gobj);
        if (wasused)
            canvas_update_dsp();
    }
    else
    {
        long size;
        int styleonset, styletype;
        t_symbol *stylearraytype;
        t_symbol *argname = iem_raute2dollar(name);
        t_array *a = garray_getarray(x);
        t_template *scalartemplate;
        if (!a)
        {
            post_error ("can't find array\n");
            return;
        }
        if (!(scalartemplate = template_findbyname(x->x_scalar->sc_template)))
        {
            post_error ("array: no template of type %s",
                x->x_scalar->sc_template->s_name);
            return;
        }
        if (argname != x->x_name)
        {
            /* jsarlo { */
            if (x->x_listviewing)
            {
              // garray_arrayviewlist_close(x);
            }
            /* } jsarlo */
            x->x_name = argname;
            pd_unbind(&x->x_gobj.g_pd, x->x_realname);
            x->x_realname = canvas_realizedollar(x->x_glist, argname);
            pd_bind(&x->x_gobj.g_pd, x->x_realname);
                /* redraw the whole glist, just so the name change shows up */
            if (x->x_glist->gl_havewindow)
                canvas_redraw(x->x_glist);
            else if (glist_isvisible(x->x_glist->gl_owner))
            {
                gobj_vis(&x->x_glist->gl_obj.te_g, x->x_glist->gl_owner, 0);
                gobj_vis(&x->x_glist->gl_obj.te_g, x->x_glist->gl_owner, 1);
            }
            canvas_update_dsp();
        }
        size = fsize;
        if (size < 1)
            size = 1;
        if (size != a->a_n)
            garray_resize_long(x, size);
        else if (style != stylewas)
            garray_fittograph(x, size, style);
        template_setfloat(scalartemplate, gensym("style"),
            x->x_scalar->sc_vector, (t_float)style, 0);

        garray_setsaveit(x, (saveit != 0));
        garray_redraw(x);
        canvas_dirty(x->x_glist, 1);
    }
}

/* jsarlo { */
void garray_arrayviewlist_new(t_garray *x)
{/*
    int i, xonset=0, yonset=0, type=0, elemsize=0;
    t_float yval;
    char cmdbuf[200];
    t_symbol *arraytype;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);

    if (!a)
    {
        //
        post_error ("error in garray_arrayviewlist_new()");
    }
    x->x_listviewing = 1;
    sprintf(cmdbuf,
            "::ui_array::pdtk_array_listview_new %%s %s %d\n",
            x->x_realname->s_name,
            0);
    gfxstub_new(&x->x_gobj.g_pd, x, cmdbuf);
    for (i = 0; i < ARRAYPAGESIZE && i < a->a_n; i++)
    {
        yval = *(t_float *)(a->a_vec +
               elemsize * i + yonset);
        sys_vgui(".%sArrayWindow.lb insert %d {%d) %g}\n",
                 x->x_realname->s_name,
                 i,
                 i,
                 yval);
    }*/
}

void garray_arrayviewlist_fillpage(t_garray *x,
                                   t_float page,
                                   t_float fTopItem)
{/*
    int i, xonset=0, yonset=0, type=0, elemsize=0, topItem;
    t_float yval;
    char cmdbuf[200];
    t_symbol *arraytype;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    
    topItem = (int)fTopItem;
    if (!a)
    {
        //
        post_error ("error in garray_arrayviewlist_new()");
    }

    if (page < 0) {
      page = 0;
      sys_vgui("::ui_array::pdtk_array_listview_setpage %s %d\n",
               x->x_realname->s_name,
               (int)page);
    }
    else if ((page * ARRAYPAGESIZE) >= a->a_n) {
      page = (int)(((int)a->a_n - 1)/ (int)ARRAYPAGESIZE);
      sys_vgui("::ui_array::pdtk_array_listview_setpage %s %d\n",
               x->x_realname->s_name,
               (int)page);
    }
    sys_vgui(".%sArrayWindow.lb delete 0 %d\n",
             x->x_realname->s_name,
             ARRAYPAGESIZE - 1);
    for (i = page * ARRAYPAGESIZE;
         (i < (page + 1) * ARRAYPAGESIZE && i < a->a_n);
         i++)
    {
        yval = *(t_float *)(a->a_vec + \
               elemsize * i + yonset);
        sys_vgui(".%sArrayWindow.lb insert %d {%d) %g}\n",
                 x->x_realname->s_name,
                 i % ARRAYPAGESIZE,
                 i,
                 yval);
    }
    sys_vgui(".%sArrayWindow.lb yview %d\n",
             x->x_realname->s_name,
             topItem);*/
}

void garray_arrayviewlist_close(t_garray *x)
{
    x->x_listviewing = 0;
    /*sys_vgui("::ui_array::pdtk_array_listview_closeWindow %s\n",
             x->x_realname->s_name);*/
}
/* } jsarlo */

static void garray_free(t_garray *x)
{
    t_pd *x2;
        sys_unqueuegui(&x->x_gobj);
    /* jsarlo { */
        if (x->x_listviewing)
    {
        // garray_arrayviewlist_close(x);
    }
    /* } jsarlo */
    gfxstub_deleteforkey(x);
    pd_unbind(&x->x_gobj.g_pd, x->x_realname);
        /* just in case we're still bound to #A from loading... */
    while (x2 = pd_findByClass(gensym("#A"), garray_class))
        pd_unbind(x2, gensym("#A"));
    pd_free(&x->x_scalar->sc_g.g_pd);
}

/* ------------- code used by both array and plot widget functions ---- */

void array_redraw(t_array *a, t_glist *glist)
{
    while (a->a_gp.gp_stub->gs_type == POINTER_ARRAY)
        a = a->a_gp.gp_stub->gs_un.gs_array;
    scalar_redraw(a->a_gp.gp_un.gp_scalar, glist);
}

    /* routine to get screen coordinates of a point in an array */
void array_getcoordinate(t_glist *glist,
    char *elem, int xonset, int yonset, int wonset, int indx,
    t_float basex, t_float basey, t_float xinc,
    t_fielddesc *xfielddesc, t_fielddesc *yfielddesc, t_fielddesc *wfielddesc,
    t_float *xp, t_float *yp, t_float *wp)
{
    t_float xval, yval, ypix, wpix;
    if (xonset >= 0)
        xval = *(t_float *)(elem + xonset);
    else xval = indx * xinc;
    if (yonset >= 0)
        yval = *(t_float *)(elem + yonset);
    else yval = 0;
    ypix = glist_ytopixels(glist, basey +
        fielddesc_cvttocoord(yfielddesc, yval));
    if (wonset >= 0)
    {
            /* found "w" field which controls linewidth. */
        t_float wval = *(t_float *)(elem + wonset);
        wpix = glist_ytopixels(glist, basey + 
            fielddesc_cvttocoord(yfielddesc, yval) +
                fielddesc_cvttocoord(wfielddesc, wval)) - ypix;
        if (wpix < 0)
            wpix = -wpix;
    }
    else wpix = 1;
    *xp = glist_xtopixels(glist, basex +
        fielddesc_cvttocoord(xfielddesc, xval));
    *yp = ypix;
    *wp = wpix;
}

static void array_getrect(t_array *array, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_float x1 = 0x7fffffff, y1 = 0x7fffffff, x2 = -0x7fffffff, y2 = -0x7fffffff;
    t_canvas *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset, i;

    if (!array_getfields(array->a_templatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize, 0, 0, 0, &xonset, &yonset, &wonset))
    {
        int incr;
            /* if it has more than 2000 points, just check 300 of them. */
        if (array->a_n < 2000)
            incr = 1;
        else incr = array->a_n / 300;
        for (i = 0; i < array->a_n; i += incr)
        {
            t_float pxpix, pypix, pwpix, dx, dy;
            array_getcoordinate(glist, (char *)(array->a_vec) +
                i * elemsize,
                xonset, yonset, wonset, i, 0, 0, 1,
                0, 0, 0,
                &pxpix, &pypix, &pwpix);
            if (pwpix < 2)
                pwpix = 2;
            if (pxpix < x1)
                x1 = pxpix;
            if (pxpix > x2)
                x2 = pxpix;
            if (pypix - pwpix < y1)
                y1 = pypix - pwpix;
            if (pypix + pwpix > y2)
                y2 = pypix + pwpix;
        }
    }
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

/* -------------------- widget behavior for garray ------------ */

static void garray_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_garray *x = (t_garray *)z;
    gobj_getrect(&x->x_scalar->sc_g, glist, xp1, yp1, xp2, yp2);
}

static void garray_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    /* refuse */
}

static void garray_select(t_gobj *z, t_glist *glist, int state)
{
    t_garray *x = (t_garray *)z;
    /* fill in later */
}

static void garray_activate(t_gobj *z, t_glist *glist, int state)
{
}

static void garray_delete(t_gobj *z, t_glist *glist)
{
    /* nothing to do */
}

static void garray_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_garray *x = (t_garray *)z;
    gobj_vis(&x->x_scalar->sc_g, glist, vis);
}

static int garray_click(t_gobj *z, t_glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_garray *x = (t_garray *)z;
    return (gobj_click(&x->x_scalar->sc_g, glist,
        xpix, ypix, shift, alt, dbl, doit));
}

#define ARRAYWRITECHUNKSIZE 1000

void garray_savecontentsto(t_garray *x, t_binbuf *b)
{
    if (x->x_saveit)
    {
        t_array *array = garray_getarray(x);
        int n = array->a_n, n2 = 0;
        if (n > 200000)
            post("warning: I'm saving an array with %d points!\n", n);
        while (n2 < n)
        {
            int chunk = n - n2, i;
            if (chunk > ARRAYWRITECHUNKSIZE)
                chunk = ARRAYWRITECHUNKSIZE;
            binbuf_addv(b, "si", gensym("#A"), n2);
            for (i = 0; i < chunk; i++)
                binbuf_addv(b, "f", ((t_word *)(array->a_vec))[n2+i].w_float);
            binbuf_addv(b, ";");
            n2 += chunk;
        }
    }
}

static void garray_save(t_gobj *z, t_binbuf *b)
{
    int style, filestyle;
    t_garray *x = (t_garray *)z;
    t_array *array = garray_getarray(x);
    t_template *scalartemplate;
    if (x->x_scalar->sc_template != gensym("pd-float-array"))
    {
            /* LATER "save" the scalar as such */ 
        post_error ("can't save arrays of type %s yet", 
            x->x_scalar->sc_template->s_name);
        return;
    }
    if (!(scalartemplate = template_findbyname(x->x_scalar->sc_template)))
    {
        post_error ("array: no template of type %s",
            x->x_scalar->sc_template->s_name);
        return;
    }
    style = template_getfloat(scalartemplate, gensym("style"),
            x->x_scalar->sc_vector, 0);    
    filestyle = (style == PLOT_POINTS ? 1 : 
        (style == PLOT_POLYGONS ? 0 : style)); 
    binbuf_addv(b, "sssisi;", gensym("#X"), gensym("array"),
        x->x_name, array->a_n, &s_float,
            x->x_saveit + 2 * filestyle + 8*x->x_hidename);
    garray_savecontentsto(x, b);
}

t_widgetbehavior garray_widgetbehavior =
{
    garray_getrect,
    garray_displace,
    garray_select,
    garray_activate,
    garray_delete,
    garray_vis,
    garray_click,
};

/* ----------------------- public functions -------------------- */

void garray_usedindsp(t_garray *x)
{
    x->x_usedindsp = 1;
}

static void garray_doredraw(t_gobj *client, t_glist *glist)
{
    t_garray *x = (t_garray *)client;
    if (glist_isvisible(x->x_glist) && gobj_shouldvis(client, glist))
    {
        garray_vis(&x->x_gobj, x->x_glist, 0); 
        garray_vis(&x->x_gobj, x->x_glist, 1);
    }
}

void garray_redraw(t_garray *x)
{
    if (glist_isvisible(x->x_glist))
        sys_queuegui(&x->x_gobj, x->x_glist, garray_doredraw);
    /* jsarlo { */
    /* this happens in garray_vis() when array is visible for
       performance reasons */
    //else
    //{
      /* if (x->x_listviewing)
        sys_vgui("::ui_array::pdtk_array_listview_fillpage %s\n",
                 x->x_realname->s_name);*/
    //}
    /* } jsarlo */
}

   /* This functiopn gets the template of an array; if we can't figure
   out what template an array's elements belong to we're in grave trouble
   when it's time to free or resize it.  */
t_template *garray_template(t_garray *x)
{
    t_array *array = garray_getarray(x);
    t_template *template = 
        (array ? template_findbyname(array->a_templatesym) : 0);
    if (!template) { PD_BUG; }
    return (template);
}

int garray_npoints(t_garray *x) /* get the length */
{
    t_array *array = garray_getarray(x);
    return (array->a_n);
}

char *garray_vec(t_garray *x) /* get the contents */
{
    t_array *array = garray_getarray(x);
    return ((char *)(array->a_vec));
}

    /* routine that checks if we're just an array of floats and if
    so returns the goods */

int garray_getfloatwords(t_garray *x, int *size, t_word **vec)
{
    int yonset, type, elemsize;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!a)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return (0);
    }
    else if (elemsize != sizeof(t_word))
    {
        post_error ("%s: has more than one field", x->x_realname->s_name);
        return (0);
    }
    *size = garray_npoints(x);
    *vec =  (t_word *)garray_vec(x);
    return (1);
}
    /* older, non-64-bit safe version, supplied for older externs */

int garray_getfloatarray(t_garray *x, int *size, t_float **vec)
{
    if (sizeof(t_word) != sizeof(t_float))
    {
        t_symbol *patchname;
        if (x->x_glist->gl_owner)
            patchname = x->x_glist->gl_owner->gl_name;
        else
            patchname = x->x_glist->gl_name;
        post_error ("An operation on the array '%s' in the patch '%s'",
              x->x_name->s_name, patchname->s_name);
        post_error ("failed since it uses garray_getfloatarray while running 64-bit!");
    }
    return (garray_getfloatwords(x, size, (t_word **)vec));
}

    /* set the "saveit" flag */
void garray_setsaveit(t_garray *x, int saveit)
{
    if (x->x_saveit && !saveit)
        post("warning: array %s: clearing save-in-patch flag",
            x->x_name->s_name);
    x->x_saveit = saveit;
}

/*------------------- Pd messages ------------------------ */
static void garray_const(t_garray *x, t_floatarg g)
{
    int yonset, i, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
    else for (i = 0; i < array->a_n; i++)
        *((t_float *)((char *)array->a_vec
            + elemsize * i) + yonset) = g;
    garray_redraw(x);
}

    /* sum of Fourier components; called from routines below */
static void garray_dofo(t_garray *x, long npoints, t_float dcval,
    int nsin, t_float *vsin, int sineflag)
{
    double phase, phaseincr, fj;
    int yonset, i, j, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return;
    }
    if (npoints == 0)
        npoints = 512;  /* dunno what a good default would be... */
    if (npoints != (1 << ilog2(npoints)))
        post("%s: rounnding to %d points", array->a_templatesym->s_name,
            (npoints = (1<<ilog2(npoints))));
    garray_resize_long(x, npoints + 3);
    phaseincr = 2. * 3.14159 / npoints;
    for (i = 0, phase = -phaseincr; i < array->a_n; i++, phase += phaseincr)
    {
        double sum = dcval;
        if (sineflag)
            for (j = 0, fj = phase; j < nsin; j++, fj += phase)
                sum += vsin[j] * sin(fj);
        else
            for (j = 0, fj = 0; j < nsin; j++, fj += phase)
                sum += vsin[j] * cos(fj);
        *((t_float *)((array->a_vec + elemsize * i)) + yonset)
            = sum;
    }
    garray_redraw(x);
}

static void garray_sinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    t_float *svec;
    long npoints;
    int i;
    if (argc < 2)
    {
        post_error ("sinesum: %s: need number of points and partial strengths",
            x->x_realname->s_name);
        return;
    }

    npoints = atom_getfloatarg(0, argc, argv);
    argv++, argc--;
    
    svec = (t_float *)PD_MEMORY_GET(sizeof(t_float) * argc);
    if (!svec) return;
    
    for (i = 0; i < argc; i++)
        svec[i] = atom_getfloatarg(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 1);
    PD_MEMORY_FREE(svec, sizeof(t_float) * argc);
}

static void garray_cosinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float *svec;
    long npoints;
    int i;
    if (argc < 2)
    {
        post_error ("sinesum: %s: need number of points and partial strengths",
            x->x_realname->s_name);
        return;
    }

    npoints = atom_getfloatarg(0, argc, argv);
    argv++, argc--;
    
    svec = (t_float *)PD_MEMORY_GET(sizeof(t_float) * argc);
    if (!svec) return;

    for (i = 0; i < argc; i++)
        svec[i] = atom_getfloatarg(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 0);
    PD_MEMORY_FREE(svec, sizeof(t_float) * argc);
}

static void garray_normalize(t_garray *x, t_float f)
{
    int type, i;
    double maxv, renormer;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return;
    }

    if (f <= 0)
        f = 1;

    for (i = 0, maxv = 0; i < array->a_n; i++)
    {
        double v = *((t_float *)(array->a_vec + elemsize * i)
            + yonset);
        if (v > maxv)
            maxv = v;
        if (-v > maxv)
            maxv = -v;
    }
    if (maxv > 0)
    {
        renormer = f / maxv;
        for (i = 0; i < array->a_n; i++)
            *((t_float *)(array->a_vec + elemsize * i) + yonset)
                *= renormer;
    }
    garray_redraw(x);
}

    /* list -- the first value is an index; subsequent values are put in
    the "y" slot of the array.  This generalizes Max's "table", sort of. */
static void garray_list(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return;
    }
    if (argc < 2) return;
    else
    {
        int firstindex = atom_getfloat(argv);
        argc--;
        argv++;
            /* drop negative x values */
        if (firstindex < 0)
        {
            argc += firstindex;
            argv -= firstindex;
            firstindex = 0;
            if (argc <= 0) return;
        }
        if (argc + firstindex > array->a_n)
        {
            argc = array->a_n - firstindex;
            if (argc <= 0) return;
        }
        for (i = 0; i < argc; i++)
            *((t_float *)(array->a_vec + elemsize * (i + firstindex)) + yonset)
                = atom_getfloat(argv + i);
    }
    garray_redraw(x);
}

    /* forward a "bounds" message to the owning graph */
static void garray_bounds(t_garray *x, t_floatarg x1, t_floatarg y1,
    t_floatarg x2, t_floatarg y2)
{
    pd_variadicMessage(&x->x_glist->gl_obj.te_g.g_pd, gensym("bounds"), "ffff", x1, y1, x2, y2);
}

    /* same for "xticks", etc */
static void garray_xticks(t_garray *x,
    t_floatarg point, t_floatarg inc, t_floatarg f)
{
    pd_variadicMessage(&x->x_glist->gl_obj.te_g.g_pd, gensym("xticks"), "fff", point, inc, f);
}

static void garray_yticks(t_garray *x,
    t_floatarg point, t_floatarg inc, t_floatarg f)
{
    pd_variadicMessage(&x->x_glist->gl_obj.te_g.g_pd, gensym("yticks"), "fff", point, inc, f);
}

static void garray_xlabel(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_message(&x->x_glist->gl_obj.te_g.g_pd, s, argc, argv);
}

static void garray_ylabel(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_message(&x->x_glist->gl_obj.te_g.g_pd, s, argc, argv);
}
    /* change the name of a garray. */
static void garray_rename(t_garray *x, t_symbol *s)
{
    /* jsarlo { */
    if (x->x_listviewing)
    {
        /*garray_arrayviewlist_close(x);*/
    }
    /* } jsarlo */
    pd_unbind(&x->x_gobj.g_pd, x->x_realname);
    pd_bind(&x->x_gobj.g_pd, x->x_realname = x->x_name = s);
    garray_redraw(x);
}

static void garray_read(t_garray *x, t_symbol *filename)
{
    int nelem, filedesc, i;
    FILE *fd;
    char buf[PD_STRING], *bufptr;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return;
    }
    nelem = array->a_n;
    if ((filedesc = canvas_open(glist_getcanvas(x->x_glist),
            filename->s_name, "", buf, &bufptr, PD_STRING, 0)) < 0 
                || !(fd = fdopen(filedesc, "r")))
    {
        post_error ("%s: can't open", filename->s_name);
        return;
    }
    for (i = 0; i < nelem; i++)
    {
        double f;
        if (!fscanf(fd, "%lf", &f))
        {
            post("%s: read %d elements into table of size %d",
                filename->s_name, i, nelem);
            break;
        }
        else *((t_float *)(array->a_vec + elemsize * i) + yonset) = f;
    }
    while (i < nelem)
        *((t_float *)(array->a_vec +
            elemsize * i) + yonset) = 0, i++;
    fclose(fd);
    garray_redraw(x);
}

static void garray_write(t_garray *x, t_symbol *filename)
{
    FILE *fd;
    char buf[PD_STRING];
    int yonset, elemsize, i;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_realname->s_name);
        return;
    }
    canvas_makefilename(glist_getcanvas(x->x_glist), filename->s_name,
        buf, PD_STRING);
    if (!(fd = sys_fopen(buf, "w")))
    {
        post_error ("%s: can't create", buf);
        return;
    }
    for (i = 0; i < array->a_n; i++)
    {
        if (fprintf(fd, "%g\n",
            *(t_float *)(((array->a_vec + sizeof(t_word) * i)) + yonset)) < 1)
        {
            post("%s: write error", filename->s_name);
            break;
        }
    }
    fclose(fd);
}


    /* this should be renamed and moved... */
int garray_ambigendian(void)
{
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return (c==0);
}

void garray_resize_long(t_garray *x, long n)
{
    t_array *array = garray_getarray(x);
    t_glist *gl = x->x_glist;
    if (n < 1)
        n = 1;
    garray_fittograph(x, n, template_getfloat(
        template_findbyname(x->x_scalar->sc_template),
            gensym("style"), x->x_scalar->sc_vector, 1));
    array_resize_and_redraw(array, x->x_glist, n);
    if (x->x_usedindsp)
        canvas_update_dsp();
}

    /* float version to use as Pd method */
void garray_resize(t_garray *x, t_floatarg f)
{
    garray_resize_long(x, f);
}

static void garray_print(t_garray *x)
{
    t_array *array = garray_getarray(x);
    post("garray %s: template %s, length %d",
        x->x_realname->s_name, array->a_templatesym->s_name, array->a_n);
}

void g_array_setup(void)
{
    garray_class = class_new(gensym("array"), 0, (t_method)garray_free,
        sizeof(t_garray), CLASS_GRAPHIC, 0);
    class_setWidget(garray_class, &garray_widgetbehavior);
    class_addMethod(garray_class, (t_method)garray_const, gensym("const"),
        A_DEFFLOAT, A_NULL);
    class_addList(garray_class, garray_list);
    class_addMethod(garray_class, (t_method)garray_bounds, gensym("bounds"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(garray_class, (t_method)garray_xticks, gensym("xticks"),
        A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(garray_class, (t_method)garray_xlabel, gensym("xlabel"),
        A_GIMME, 0);
    class_addMethod(garray_class, (t_method)garray_yticks, gensym("yticks"),
        A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(garray_class, (t_method)garray_ylabel, gensym("ylabel"),
        A_GIMME, 0);
    class_addMethod(garray_class, (t_method)garray_rename, gensym("rename"),
        A_SYMBOL, 0);
    class_addMethod(garray_class, (t_method)garray_read, gensym("read"),
        A_SYMBOL, A_NULL);
    class_addMethod(garray_class, (t_method)garray_write, gensym("write"),
        A_SYMBOL, A_NULL);
    class_addMethod(garray_class, (t_method)garray_resize, gensym("resize"),
        A_FLOAT, A_NULL);
    class_addMethod(garray_class, (t_method)garray_print, gensym("print"),
        A_NULL);
    class_addMethod(garray_class, (t_method)garray_sinesum, gensym("sinesum"),
        A_GIMME, 0);
    class_addMethod(garray_class, (t_method)garray_cosinesum,
        gensym("cosinesum"), A_GIMME, 0);
    class_addMethod(garray_class, (t_method)garray_normalize,
        gensym("normalize"), A_DEFFLOAT, 0);
    class_addMethod(garray_class, (t_method)garray_arraydialog,
        gensym("arraydialog"), A_SYMBOL, A_FLOAT, A_FLOAT, A_NULL);
/* jsarlo { */
    class_addMethod(garray_class, (t_method)garray_arrayviewlist_new,
        gensym("arrayviewlistnew"), A_NULL);
    class_addMethod(garray_class, (t_method)garray_arrayviewlist_fillpage,
        gensym("arrayviewlistfillpage"), A_FLOAT, A_DEFFLOAT, A_NULL);
    class_addMethod(garray_class, (t_method)garray_arrayviewlist_close,
      gensym("arrayviewclose"), A_NULL);
/* } jsarlo */
    class_setSaveFunction(garray_class, garray_save);
}


