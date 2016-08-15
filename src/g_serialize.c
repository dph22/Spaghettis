
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *scalar_class;
extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_findTemplatesAppendPerform (t_symbol *templateIdentifier, int *n, t_symbol ***v)
{
    int t = *n;
    t_symbol **templates = *v;
    int alreadyExist = 0;
    int i;
    
    for (i = 0; i < t; i++) { if (templates[i] == templateIdentifier) { alreadyExist = 1; break; } }

    if (!alreadyExist) {
    //
    int oldSize = sizeof (t_symbol *) * (t);
    int newSize = sizeof (t_symbol *) * (t + 1);
        
    templates    = (t_symbol **)PD_MEMORY_RESIZE (templates, oldSize, newSize);
    templates[t] = templateIdentifier;
    
    *v = templates;
    *n = t + 1;
    //
    }
}

static void canvas_findTemplatesAppendRecursive (t_template *tmpl, int *n, t_symbol ***v)
{
    int i;

    canvas_findTemplatesAppendPerform (template_getTemplateIdentifier (tmpl), n, v);

    for (i = 0; i < template_getSize (tmpl); i++) {
        t_template *t = template_getTemplateIfArrayAtIndex (tmpl, i);
        if (t) {
            canvas_findTemplatesAppendRecursive (t, n, v);
        }
    }
}

static void canvas_findTemplatesRecursive (t_glist *glist, int *n, t_symbol ***v)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == scalar_class) {
            canvas_findTemplatesAppendRecursive (scalar_getTemplate (cast_scalar (y)), n, v);
        }
        if (pd_class (y) == canvas_class) { 
            canvas_findTemplatesRecursive (cast_glist (y), n, v);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* return true if two dataslot definitions match */
static int dataslot_matches(t_dataslot *ds1, t_dataslot *ds2,
    int nametoo)
{
    return ((!nametoo || ds1->ds_fieldName == ds2->ds_fieldName) &&
        ds1->ds_type == ds2->ds_type &&
            (ds1->ds_type != DATA_ARRAY ||
                ds1->ds_templateIdentifier == ds2->ds_templateIdentifier));
}

    /* stringent check to see if a "saved" template, x2, matches the current
        one (x1).  It's OK if x1 has additional scalar elements but not (yet)
        arrays.  This is used for reading in "data files". */
static int template_equals(t_template *x1, t_template *x2)
{
    int i;
    if (x1->tp_size < x2->tp_size)
        return (0);
    for (i = x2->tp_size; i < x1->tp_size; i++)
    {
        if (x1->tp_vector[i].ds_type == DATA_ARRAY)
                return (0);
    }
    if (x2->tp_size > x1->tp_size)
        post("add elements...");
    for (i = 0; i < x2->tp_size; i++)
        if (!dataslot_matches(&x1->tp_vector[i], &x2->tp_vector[i], 1))
            return (0);
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int canvas_scanbinbuf(int natoms, t_atom *vec, int *p_indexout, int *p_next)
{
    int i, j;
    int indexwas = *p_next;
    *p_indexout = indexwas;
    if (indexwas >= natoms)
        return (0);
    for (i = indexwas; i < natoms && vec[i].a_type != A_SEMICOLON; i++)
        ;
    if (i >= natoms)
        *p_next = i;
    else *p_next = i + 1;
    return (i - indexwas);
}

static void canvas_readerror (int natoms, t_atom *vec, int message, int nline, char *s)
{
    post_error ("%s", s);
    post("line was:");
    post_atoms(nline, vec + message);
}

    /* fill in the contents of the scalar into the vector w. */

static void glist_readatoms(t_glist *x, int natoms, t_atom *vec,
    int *p_nextmsg, t_symbol *templatesym, t_word *w, int argc, t_atom *argv)
{
    int message, nline, n, i;

    t_template *template = template_findByIdentifier(templatesym);
    if (!template)
    {
        post_error ("%s: no such template", templatesym->s_name);
        *p_nextmsg = natoms;
        return;
    }
    word_restore(w, template, argc, argv);
    n = template->tp_size;
    for (i = 0; i < n; i++)
    {
        if (template->tp_vector[i].ds_type == DATA_ARRAY)
        {
            int j;
            t_array *a = w[i].w_array;
            int elemsize = a->a_stride, nitems = 0;
            t_symbol *arraytemplatesym = template->tp_vector[i].ds_templateIdentifier;
            t_template *arraytemplate =
                template_findByIdentifier(arraytemplatesym);
            if (!arraytemplate)
            {
                post_error ("%s: no such template", arraytemplatesym->s_name);
            }
            else while (1)
            {
                t_word *element;
                int nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
                    /* empty line terminates array */
                if (!nline)
                    break;
                array_resize(a, nitems + 1);
                element = (t_word *)(((char *)a->a_vector) +
                    nitems * elemsize);
                glist_readatoms(x, natoms, vec, p_nextmsg, arraytemplatesym,
                    element, nline, vec + message);
                nitems++;
            }
        }
        else if (template->tp_vector[i].ds_type == DATA_TEXT)
        {
            t_buffer *z = buffer_new();
            int first = *p_nextmsg, last;
            for (last = first; last < natoms && vec[last].a_type != A_SEMICOLON;
                last++);
            buffer_deserialize(z, last-first, vec+first);
            buffer_append(w[i].w_buffer, buffer_size(z), buffer_atoms(z));
            buffer_free(z);
            last++;
            if (last > natoms) last = natoms;
            *p_nextmsg = last;
        }
    }
}

int canvas_readscalar(t_glist *x, int natoms, t_atom *vec,
    int *p_nextmsg, int selectit)
{
    int message, i, j, nline;
    t_template *template;
    t_symbol *templatesym;
    t_scalar *sc;
    int nextmsg = *p_nextmsg;
    int wasvis = canvas_isMapped(x);

    if (nextmsg >= natoms || vec[nextmsg].a_type != A_SYMBOL)
    {
        if (nextmsg < natoms)
            post("stopping early: type %d", vec[nextmsg].a_type);
        *p_nextmsg = natoms;
        return (0);
    }
    templatesym = utils_makeBindSymbol(vec[nextmsg].a_w.w_symbol);
    *p_nextmsg = nextmsg + 1;
    
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("canvas_read: %s: no such template", templatesym->s_name);
        *p_nextmsg = natoms;
        return (0);
    }
    sc = scalar_new(x, templatesym);
    if (!sc)
    {
        post_error ("couldn't create scalar \"%s\"", templatesym->s_name);
        *p_nextmsg = natoms;
        return (0);
    }
    if (wasvis)
    {
            /* temporarily lie about vis flag while this is built */
        canvas_getView(x)->gl_isMapped = 0;
    }
    canvas_addObject (x, &sc->sc_g);
    
    nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
    glist_readatoms(x, natoms, vec, p_nextmsg, templatesym, sc->sc_vector, 
        nline, vec + message);
    if (wasvis)
    {
            /* reset vis flag as before */
        canvas_getView(x)->gl_isMapped = 1;
        gobj_visibilityChanged(&sc->sc_g, x, 1);
    }
    if (selectit)
    {
        canvas_selectObject(x, &sc->sc_g);
    }
    return (1);
}

static void glist_readfrombinbuf(t_glist *x, t_buffer *b, char *filename, int selectem)
{
    t_glist *canvas = canvas_getView(x);
    int cr = 0, natoms, nline, message, nextmsg = 0, i, j, nitems;
    t_atom *vec;
    t_gobj *gobj;

    natoms = buffer_size(b);
    vec = buffer_atoms(b);

    
            /* check for file type */
    nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
    if (nline != 1 && vec[message].a_type != A_SYMBOL &&
        strcmp(vec[message].a_w.w_symbol->s_name, "data"))
    {
        post_error ("%s: file apparently of wrong type", filename);
        buffer_free(b);
        return;
    }
        /* read in templates and check for consistency */
    while (1)
    {
        t_template *newtemplate, *existtemplate;
        t_symbol *templatesym;
        t_atom *templateargs = PD_MEMORY_GET(0);
        int ntemplateargs = 0, newnargs;
        nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
        if (nline < 2)
            break;
        else if (nline > 2)
            canvas_readerror(natoms, vec, message, nline,
                "extra items ignored");
        else if (vec[message].a_type != A_SYMBOL ||
            strcmp(vec[message].a_w.w_symbol->s_name, "template") ||
            vec[message + 1].a_type != A_SYMBOL)
        {
            canvas_readerror(natoms, vec, message, nline,
                "bad template header");
            continue;
        }
        templatesym = utils_makeBindSymbol(vec[message + 1].a_w.w_symbol);
        while (1)
        {
            nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
            if (nline != 2 && nline != 3)
                break;
            newnargs = ntemplateargs + nline;
            templateargs = (t_atom *)PD_MEMORY_RESIZE(templateargs,
                sizeof(*templateargs) * ntemplateargs,
                sizeof(*templateargs) * newnargs);
            templateargs[ntemplateargs] = vec[message];
            templateargs[ntemplateargs + 1] = vec[message + 1];
            if (nline == 3)
                templateargs[ntemplateargs + 2] = vec[message + 2];
            ntemplateargs = newnargs;
        }
        if (!(existtemplate = template_findByIdentifier(templatesym)))
        {
            post_error ("%s: template not found in current patch",
                templatesym->s_name);
            PD_MEMORY_FREE(templateargs);
            return;
        }
        newtemplate = template_new(templatesym, ntemplateargs, templateargs);
        PD_MEMORY_FREE(templateargs);
        if (!template_equals(existtemplate, newtemplate))
        {
            post_error ("%s: template doesn't match current one",
                templatesym->s_name);
            template_free(newtemplate);
            return;
        }
        template_free(newtemplate);
    }
    while (nextmsg < natoms)
    {
        canvas_readscalar(x, natoms, vec, &nextmsg, selectem);
    }
}

static void glist_doread(t_glist *x, t_symbol *filename, int clearme)
{
    t_buffer *b = buffer_new();
    t_glist *canvas = canvas_getView(x);
    int wasvis = canvas_isMapped(canvas);
    int natoms, nline, message, nextmsg = 0, i, j;
    t_atom *vec;

    if (buffer_read(b, filename->s_name, canvas))
    {
        post_error ("read failed");
        buffer_free(b);
        return;
    }
    if (wasvis)
        canvas_visible(canvas, 0);
    if (clearme)
        canvas_clear(x);
    glist_readfrombinbuf(x, b, filename->s_name, 0);
    if (wasvis)
        canvas_visible(canvas, 1);
    buffer_free(b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Read and write scalars. */

void canvas_scalarsRead (t_glist *glist, t_symbol *filename)
{
    glist_doread (glist, filename, 1);
}

void canvas_scalarsMerge (t_glist *glist, t_symbol *filename)
{
    glist_doread (glist, filename, 0);
}

void canvas_scalarsWrite (t_glist *glist, t_symbol *filename)
{
    t_buffer *b = buffer_new();
    
    char t[PD_STRING] = { 0 };
    canvas_makeFilePath (canvas_getView (glist), filename->s_name, t, PD_STRING);
    
    canvas_serializeScalarsAll (glist, b);
    if (buffer_write (b, t, "")) { post_error (PD_TRANSLATE ("%s: write failed"), filename->s_name); }
    
    buffer_free (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_serializeScalars (t_glist *glist, t_buffer *b, int allScalars)
{
    t_symbol **v = PD_MEMORY_GET(0);
    int i, n = 0;
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == scalar_class) {
            if (allScalars || canvas_isObjectSelected (glist, y)) {
                canvas_findTemplatesAppendRecursive (scalar_getTemplate (cast_scalar (y)), &n, &v);
            }
        }
    }
    
    buffer_vAppend (b, "s;", sym_data);
    
    for (i = 0; i < n; i++) { template_serializeForFile (template_findByIdentifier (v[i]), b); }
    
    buffer_appendSemicolon (b);

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == scalar_class) {
            if (allScalars || canvas_isObjectSelected (glist, y)) { scalar_serialize (cast_scalar (y), b); }
        }
    }
}

void canvas_serializeScalarsAll (t_glist *glist, t_buffer *b)
{
    canvas_serializeScalars (glist, b, 1);
}

void canvas_serializeScalarsSelected (t_glist *glist, t_buffer *b)
{
    canvas_serializeScalars (glist, b, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_serializeTemplates (t_glist *glist, t_buffer *b)
{
    t_symbol **v = PD_MEMORY_GET (0);
    int i, n = 0;
    
    canvas_findTemplatesRecursive (glist, &n, &v);
    
    for (i = 0; i < n; i++) { template_serializeForPatch (template_findByIdentifier (v[i]), b); }
    
    PD_MEMORY_FREE (v);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
