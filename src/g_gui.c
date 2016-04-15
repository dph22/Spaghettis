
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _guiconnect
{
    t_object        x_obj;
    t_pd            *x_owner;
    t_symbol        *x_bound;
    t_clock         *x_clock;
};

typedef struct _gfxstub
{
    t_pd            x_pd;
    t_pd            *x_owner;
    void            *x_key;
    t_symbol        *x_bound;
    struct _gfxstub *x_next;
} t_gfxstub;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *gfxstub_class;
static t_class      *guiconnect_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gfxstub    *gfxstub_list;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_task (t_guiconnect *x)
{
    pd_free (cast_pd (x));
}

void guiconnect_release (t_guiconnect *x, double timeOut)
{
    if (!x->x_bound) { pd_free (cast_pd (x)); }
    else {
        x->x_owner = NULL;
        
        if (timeOut > 0) {
            x->x_clock = clock_new (x, (t_method)guiconnect_task);
            clock_delay (x->x_clock, timeOut);
        }
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_anything (t_guiconnect *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

static void guiconnect_signoff (t_guiconnect *x)
{
    if (!x->x_owner) { pd_free (cast_pd (x)); }
    else {
        pd_unbind (cast_pd (x), x->x_bound); x->x_bound = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect *guiconnect_new (t_pd *owner, t_symbol *bindTo)
{
    t_guiconnect *x = (t_guiconnect *)pd_new (guiconnect_class);
    
    PD_ASSERT (owner);
    PD_ASSERT (bindTo);
    
    x->x_owner = owner;
    x->x_bound = bindTo;
    
    pd_bind (cast_pd (x), x->x_bound);
    
    return x;
}

static void guiconnect_free (t_guiconnect *x)
{
    if (x->x_bound) { pd_unbind (cast_pd (x), x->x_bound); }
    if (x->x_clock) { clock_free (x->x_clock); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guiconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("guiconnect"),
        NULL,
        (t_method)guiconnect_free,
        sizeof (t_guiconnect), 
        CLASS_PURE,
        A_NULL);
        
    class_addAnything (c, guiconnect_anything);
    class_addMethod (c, (t_method)guiconnect_signoff, gensym ("signoff"), A_NULL);
        
    guiconnect_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* --------------------- graphics responder  ---------------- */

/* make one of these if you want to put up a dialog window but want to be
protected from getting deleted and then having the dialog call you back.  In
this design the calling object doesn't have to keep the address of the dialog
window around; instead we keep a list of all open dialogs.  Any object that
might have dialogs, when it is deleted, simply checks down the dialog window
list and breaks off any dialogs that might later have sent messages to it. 
Only when the dialog window itself closes do we delete the gfxstub object. */



    /* create a new one.  the "key" is an address by which the owner
    will identify it later; if the owner only wants one dialog, this
    could just be a pointer to the owner itself.  The string "cmd"
    is a TK command to create the dialog, with "%s" embedded in
    it so we can provide a name by which the GUI can send us back
    messages; e.g., "pdtk_canvas_dofont %s 10". */

void gfxstub_new(t_pd *owner, void *key, const char *cmd)
{
    char buf[4*PD_STRING];
    char namebuf[80];
    char sprintfbuf[PD_STRING];
    char *afterpercent;
    t_int afterpercentlen;
    t_gfxstub *x;
    t_symbol *s;
        /* if any exists with matching key, burn it. */
    for (x = gfxstub_list; x; x = x->x_next)
        if (x->x_key == key)
            gfxstub_deleteforkey(key);
    if (strlen(cmd) + 50 > 4*PD_STRING)
    {
        PD_BUG;
        return;
    }
    x = (t_gfxstub *)pd_new(gfxstub_class);
    sprintf(namebuf, ".gfxstub%lx", (t_int)x);

    s = gensym(namebuf);
    pd_bind(&x->x_pd, s);
    x->x_owner = owner;
    x->x_bound = s;
    x->x_key = key;
    x->x_next = gfxstub_list;
    gfxstub_list = x;
    /* only replace first %s so sprintf() doesn't crash */
    afterpercent = strchr(cmd, '%') + 2;
    afterpercentlen = afterpercent - cmd;
    strncpy(sprintfbuf, cmd, afterpercentlen);
    sprintfbuf[afterpercentlen] = '\0';
    sprintf(buf, sprintfbuf, s->s_name);
    strncat(buf, afterpercent, (4*PD_STRING) - afterpercentlen);
    sys_gui(buf);
}

static void gfxstub_offlist(t_gfxstub *x)
{
    t_gfxstub *y1, *y2;
    if (gfxstub_list == x)
        gfxstub_list = x->x_next;
    else for (y1 = gfxstub_list; y2 = y1->x_next; y1 = y2)
        if (y2 == x) 
    {
        y1->x_next = y2->x_next;
        break;
    }
}

    /* if the owner disappears, we still may have to stay around until our
    dialog window signs off.  Anyway we can now tell the GUI to destroy the
    window.  */
void gfxstub_deleteforkey(void *key)
{
    t_gfxstub *y;
    int didit = 1;
    while (didit)
    {
        didit = 0;
        for (y = gfxstub_list; y; y = y->x_next)
        {
            if (y->x_key == key)
            {
                sys_vGui("destroy .gfxstub%lx\n", y);
                y->x_owner = 0;
                gfxstub_offlist(y);
                didit = 1;
                break;
            }
        }
    }
}

/* --------- pd messages for gfxstub (these come from the GUI) ---------- */

    /* "cancel" to request that we close the dialog window. */
static void gfxstub_cancel(t_gfxstub *x)
{
    gfxstub_deleteforkey(x->x_key);
}

    /* "signoff" comes from the GUI to say the dialog window closed. */
static void gfxstub_signoff(t_gfxstub *x)
{
    gfxstub_offlist(x);
    pd_free(&x->x_pd);
}

static t_buffer *gfxstub_binbuf;

    /* a series of "data" messages rebuilds a scalar */
static void gfxstub_data(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!gfxstub_binbuf)
        gfxstub_binbuf = buffer_new();
    buffer_append(gfxstub_binbuf, argc, argv);
    buffer_appendSemicolon(gfxstub_binbuf);
}
    /* the "end" message terminates rebuilding the scalar */
static void gfxstub_end(t_gfxstub *x)
{
    canvas_dataproperties((t_glist *)x->x_owner,
        (t_scalar *)x->x_key, gfxstub_binbuf);
    buffer_free(gfxstub_binbuf);
    gfxstub_binbuf = 0;
}

    /* anything else is a message from the dialog window to the owner;
    just forward it. */
static void gfxstub_anything(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner)
        pd_message(x->x_owner, s, argc, argv);
}

static void gfxstub_free(t_gfxstub *x)
{
    pd_unbind(&x->x_pd, x->x_bound);
}

void gfxstub_setup(void)
{
    gfxstub_class = class_new(gensym("gfxstub"), 0, (t_method)gfxstub_free,
        sizeof(t_gfxstub), CLASS_PURE, 0);
    class_addAnything(gfxstub_class, gfxstub_anything);
    class_addMethod(gfxstub_class, (t_method)gfxstub_signoff,
        gensym("signoff"), 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_data,
        gensym("data"), A_GIMME, 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_end,
        gensym("end"), 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_cancel,
        gensym("cancel"), 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
