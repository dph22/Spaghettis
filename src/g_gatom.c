
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_utf8.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void gatom_float             (t_gatom *, t_float);
static void gatom_set               (t_gatom *, t_symbol *, int, t_atom *);
static void gatom_motion            (void *, t_float, t_float, t_float);
static void gatom_key               (void *, t_float);
static void gatom_behaviorDisplace  (t_gobj *, t_glist *, int, int);
static void gatom_behaviorVisible   (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *gatom_class;                                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior gatom_widgetBehavior =          /* Shared. */
    {
        text_getrect,
        gatom_behaviorDisplace,
        text_select,
        text_activate,
        text_delete,
        gatom_behaviorVisible,
        text_click
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define ATOM_LABEL_LEFT         0
#define ATOM_LABEL_RIGHT        1
#define ATOM_LABEL_UP           2
#define ATOM_LABEL_DOWN         3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ATOM_WIDTH_FLOAT        5
#define ATOM_WIDTH_SYMBOL       10
#define ATOM_WIDTH_MAXIMUM      80

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_drawJob (t_gobj *z, t_glist *glist)
{
    t_gatom *x = cast_gatom (z);
    
    if (canvas_isMapped (glist)) {
    //
    glist_retext (x->a_owner, cast_object (x));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_symbol *gatom_parse (t_symbol *s)
{
    if (s == utils_empty() || s == utils_dash()) { return &s_; }
    else { 
        return (dollar_fromHash (s));
    }
}

static void gatom_update (t_gatom *x)
{
    buffer_reset (cast_object (x)->te_buffer);
    buffer_append (cast_object (x)->te_buffer, 1, &x->a_atom);
    
    if (canvas_isMapped (x->a_owner)) { 
    //
    interface_guiQueueAddIfNotAlreadyThere (x, x->a_owner, gatom_drawJob);
    //
    }
}

static void gatom_setFloat (t_gatom *x, t_float f)
{
    if (x->a_lowRange != 0.0 || x->a_highRange != 0.0) { f = PD_CLAMP (f, x->a_lowRange, x->a_highRange); }
    
    gatom_float (x, f);
}

static void gatom_getPostion (t_gatom *x, t_glist *glist, int *positionX, int *positionY)
{
    int a, b, c, d;
    
    double width = font_getHostFontWidth (x->a_fontSize);
    double height = font_getHostFontHeight (x->a_fontSize);
    
    text_getrect (cast_gobj (x), glist, &a, &b, &c, &d);
    
    if (x->a_position == ATOM_LABEL_LEFT) {
        *positionX = a - 3 - (int)(strlen (x->a_label->s_name) * width);
        *positionY = b + 2;
        
    } else if (x->a_position == ATOM_LABEL_RIGHT) {
        *positionX = c + 3;
        *positionY = b + 2;
        
    } else if (x->a_position == ATOM_LABEL_UP) {
        *positionX = a;
        *positionY = b - 3 - (int)height;
        
    } else {
        *positionX = a;
        *positionY = d + 3;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_bang(t_gatom *x)
{
    if (x->a_atom.a_type == A_FLOAT)
    {
        outlet_float(x->a_obj.te_outlet, x->a_atom.a_w.w_float);
        
        if (*x->a_send->s_name && x->a_send->s_thing)
        {
            if (x->a_unexpandedSend == x->a_unexpandedReceive)
                post_error ("%s: atom with same send/receive name (infinite loop)",
                        x->a_unexpandedSend->s_name);
            else pd_float(x->a_send->s_thing, x->a_atom.a_w.w_float);
        }
    }
    else if (x->a_atom.a_type == A_SYMBOL)
    {
        outlet_symbol(x->a_obj.te_outlet, x->a_atom.a_w.w_symbol);
        
        if (*x->a_unexpandedSend->s_name && x->a_send->s_thing)
        {
            if (x->a_unexpandedSend == x->a_unexpandedReceive)
                post_error ("%s: atom with same send/receive name (infinite loop)",
                        x->a_unexpandedSend->s_name);
            else pd_symbol(x->a_send->s_thing, x->a_atom.a_w.w_symbol);
        }
    }
}

static void gatom_float(t_gatom *x, t_float f)
{
    t_atom at;
    SET_FLOAT(&at, f);
    gatom_set(x, 0, 1, &at);
    gatom_bang(x);
}

static void gatom_symbol(t_gatom *x, t_symbol *s)
{
    t_atom at;
    SET_SYMBOL(&at, s);
    gatom_set(x, 0, 1, &at);
    gatom_bang(x);
}

void gatom_click(t_gatom *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{
    if (x->a_obj.te_width == 1)
    {
        if (x->a_atom.a_type == A_FLOAT)
            gatom_float(x, (x->a_atom.a_w.w_float == 0));
    }
    else
    {
        if (alt)
        {
            if (x->a_atom.a_type != A_FLOAT) return;
            if (x->a_atom.a_w.w_float != 0)
            {
                x->a_toggledValue = x->a_atom.a_w.w_float;
                gatom_float(x, 0);
                return;
            }
            else gatom_float(x, x->a_toggledValue);
        }
        x->a_string[0] = 0;
        glist_grab(x->a_owner, &x->a_obj.te_g, (t_motionfn)gatom_motion, gatom_key,
            xpos, ypos);
    }
}

static void gatom_set(t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom oldatom = x->a_atom;
    int changed = 0;
    if (!argc) return;

    /*
    if (x->a_atom.a_type == A_FLOAT)
    {
        x->a_atom.a_w.w_float = atom_getFloat(argv);
        changed = ((x->a_atom.a_w.w_float != oldatom.a_w.w_float));
        if (isnan(x->a_atom.a_w.w_float) != isnan(oldatom.a_w.w_float))
            changed = 1;
    }*/
    
    if (x->a_atom.a_type == A_FLOAT)
        x->a_atom.a_w.w_float = atom_getFloat(argv),
            changed = (x->a_atom.a_w.w_float != oldatom.a_w.w_float);
    else if (x->a_atom.a_type == A_SYMBOL)
        x->a_atom.a_w.w_symbol = atom_getSymbol(argv),
            changed = (x->a_atom.a_w.w_symbol != oldatom.a_w.w_symbol);
    if (changed)
        gatom_update(x);
    x->a_string[0] = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* message back from dialog window */
static void gatom_param(t_gatom *x, t_symbol *sel, int argc, t_atom *argv)
{
    t_float width = atom_getFloatAtIndex(0, argc, argv);
    t_float draglo = atom_getFloatAtIndex(1, argc, argv);
    t_float draghi = atom_getFloatAtIndex(2, argc, argv);
    t_symbol *symto = gatom_parse(atom_getSymbolAtIndex(3, argc, argv));
    t_symbol *symfrom = gatom_parse(atom_getSymbolAtIndex(4, argc, argv));
    t_symbol *label = gatom_parse(atom_getSymbolAtIndex(5, argc, argv));
    t_float wherelabel = atom_getFloatAtIndex(6, argc, argv);

    gobj_visibilityChanged(&x->a_obj.te_g, x->a_owner, 0);

    if (draglo >= draghi)
        draglo = draghi = 0;
    x->a_lowRange = draglo;
    x->a_highRange = draghi;

    x->a_obj.te_width = PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM);
    x->a_position = ((int)wherelabel & 3);
    x->a_unexpandedLabel = label;
    if (*x->a_unexpandedReceive->s_name)
        pd_unbind(&x->a_obj.te_g.g_pd,
            canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));
    x->a_unexpandedReceive = symfrom;
    if (*x->a_unexpandedReceive->s_name)
        pd_bind(&x->a_obj.te_g.g_pd,
            canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));
    x->a_unexpandedSend = symto;
    x->a_send = canvas_expandDollar(x->a_owner, x->a_unexpandedSend);
    gobj_visibilityChanged(&x->a_obj.te_g, x->a_owner, 1);
    canvas_dirty(x->a_owner, 1);

    /* glist_retext(x->a_owner, &x->a_obj); */
}

static void gatom_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_gatom *x = (t_gatom *)z;
    if (dy == 0) return;
    if (x->a_atom.a_type == A_FLOAT)
    {
        if ((int)modifier & MODIFIER_SHIFT)
        {
            double nval = x->a_atom.a_w.w_float - 0.01 * dy;
            double trunc = 0.01 * (floor(100. * nval + 0.5));
            if (trunc < nval + 0.0001 && trunc > nval - 0.0001) nval = trunc;
            gatom_setFloat(x, nval);
        }
        else
        {
            double nval = x->a_atom.a_w.w_float - dy;
            double trunc = 0.01 * (floor(100. * nval + 0.5));
            if (trunc < nval + 0.0001 && trunc > nval - 0.0001) nval = trunc;
            trunc = floor(nval + 0.5);
            if (trunc < nval + 0.001 && trunc > nval - 0.001) nval = trunc;
            gatom_setFloat(x, nval);
        }
    }
}

static void gatom_key(void *z, t_float f)
{
    t_gatom *x = (t_gatom *)z;
    int c = f;
    int len = strlen(x->a_string);
    t_atom at;
    char sbuf[ATOM_BUFFER_SIZE + 4];
    if (c == 0)
    {
        /* we're being notified that no more keys will come for this grab */
        if (x->a_string[0])
            gatom_update(x);
        return;
    }
    else if (c == '\b')
    {
        if (len > 0)
        x->a_string[len-1] = 0;
        goto redraw;
    }
    else if (c == '\n')
    {
        if (x->a_atom.a_type == A_FLOAT)
            x->a_atom.a_w.w_float = atof(x->a_string);
        else if (x->a_atom.a_type == A_SYMBOL)
            x->a_atom.a_w.w_symbol = gensym (x->a_string);
        else { PD_BUG; }
        gatom_bang(x);
        gatom_update(x);
        x->a_string[0] = 0;
    }
    else if (len < (ATOM_BUFFER_SIZE-1))
    {
            /* for numbers, only let reasonable characters through */
        if ((x->a_atom.a_type == A_SYMBOL) ||
            (c >= '0' && c <= '9' || c == '.' || c == '-'
                || c == 'e' || c == 'E'))
        {
            /* the wchar could expand to up to 4 bytes, which
             * which might overrun our a_string;
             * therefore we first expand into a temporary buffer, 
             * and only if the resulting utf8 string fits into a_string
             * we apply it
             */
            char utf8[UTF8_MAXIMUM_BYTES];
            int utf8len = u8_wc_toutf8(utf8, c);
            if((len+utf8len) < (ATOM_BUFFER_SIZE-1))
            {
                int j=0;
                for(j=0; j<utf8len; j++)
                    x->a_string[len+j] = utf8[j];
                 
                x->a_string[len+utf8len] = 0;
            }
            goto redraw;
        }
    }
    return;
redraw:
        /* LATER figure out how to avoid creating all these symbols! */
    sprintf(sbuf, "%s...", x->a_string);
    SET_SYMBOL(&at, gensym (sbuf));
    buffer_reset(x->a_obj.te_buffer);
    buffer_append(x->a_obj.te_buffer, 1, &at);
    glist_retext(x->a_owner, &x->a_obj);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_behaviorDisplace (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_gatom *x = cast_gatom (z);
    
    text_displace (z, glist, deltaX, deltaY);
    
    sys_vGui (".x%lx.c move %lx.l %d %d\n", 
                    canvas_getView (glist), 
                    x,
                    deltaX,
                    deltaY);
}

static void gatom_behaviorVisible (t_gobj *z, t_glist *glist, int isVisible)
{
    t_gatom *x = cast_gatom (z);
    
    text_vis (z, glist, isVisible);
    
    if (x->a_label != &s_) {
    //
    if (!isVisible) { sys_vGui (".x%lx.c delete %lx.l\n", canvas_getView (glist), x); }
    else { 
        int positionX = 0;
        int positionY = 0;
        
        gatom_getPostion (x, glist, &positionX, &positionY);
        
        sys_vGui ("::ui_box::newText .x%lx.c {%lx.l label text} %f %f {%s} %d #%06x\n",
                        canvas_getView (glist),
                        x,
                        (double)positionX,
                        (double)positionY,
                        x->a_label->s_name,
                        font_getHostFontSize (x->a_fontSize),
                        COLOR_NORMAL);
    }
    //
    }
    
    if (!isVisible) { interface_guiQueueRemove (x); }
}

static void gatom_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_gatom *x = (t_gatom *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    t_symbol *send    = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedSend, 0));
    t_symbol *receive = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedReceive, 0));
    t_symbol *label   = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedLabel, 0));
    
    err = string_sprintf (t, PD_STRING, 
            "::ui_atom::show %%s %d %g %g {%s} {%s} {%s} %d\n",     // --
            cast_object (x)->te_width,
            x->a_lowRange,
            x->a_highRange,
            send->s_name,
            receive->s_name,
            label->s_name, 
            x->a_position);
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gatom_makeObject (t_glist *glist, t_atomtype type, t_symbol *s, int argc, t_atom *argv)
{
    t_gatom *x = (t_gatom *)pd_new (gatom_class);
    
    cast_object (x)->te_buffer  = buffer_new();
    cast_object (x)->te_width   = (type == A_FLOAT) ? ATOM_WIDTH_FLOAT : ATOM_WIDTH_SYMBOL;
    cast_object (x)->te_type    = TYPE_ATOM;
    x->a_owner                  = glist;
    x->a_atom.a_type            = type;
    x->a_lowRange               = 0;
    x->a_highRange              = 0;
    x->a_toggledValue           = 1;
    x->a_send                   = &s_;
    x->a_receive                = &s_;
    x->a_label                  = &s_;
    x->a_unexpandedSend         = &s_;
    x->a_unexpandedReceive      = &s_;
    x->a_unexpandedLabel        = &s_;
    x->a_fontSize               = canvas_getFontSize (x->a_owner);
    x->a_position               = ATOM_LABEL_RIGHT;
        
    if (type == A_FLOAT) {
        t_atom a;
        SET_FLOAT (&x->a_atom, 0);
        SET_FLOAT (&a, 0);
        buffer_append (cast_object (x)->te_buffer, 1, &a);
        
    } else {
        t_atom a;
        SET_SYMBOL (&x->a_atom, &s_symbol);
        SET_SYMBOL (&a, &s_symbol);
        buffer_append (cast_object (x)->te_buffer, 1, &a);
    }
    
    if (argc > 1) {                                                             /* File creation. */
    
        int width    = (int)atom_getFloatAtIndex (2, argc, argv);
        int position = (int)atom_getFloatAtIndex (5, argc, argv);
        
        cast_object (x)->te_xCoordinate     = atom_getFloatAtIndex (0, argc, argv);
        cast_object (x)->te_yCoordinate     = atom_getFloatAtIndex (1, argc, argv);
        cast_object (x)->te_width           = PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM);
        x->a_lowRange                       = atom_getFloatAtIndex (3, argc, argv);
        x->a_highRange                      = atom_getFloatAtIndex (4, argc, argv);
        x->a_position                       = PD_CLAMP (position, ATOM_LABEL_LEFT, ATOM_LABEL_DOWN);
        x->a_unexpandedLabel                = gatom_parse (atom_getSymbolAtIndex (6, argc, argv));
        x->a_unexpandedReceive              = gatom_parse (atom_getSymbolAtIndex (7, argc, argv));
        x->a_unexpandedSend                 = gatom_parse (atom_getSymbolAtIndex (8, argc, argv));
        x->a_send                           = canvas_expandDollar (x->a_owner, x->a_unexpandedSend);
        x->a_receive                        = canvas_expandDollar (x->a_owner, x->a_unexpandedReceive);
        x->a_label                          = canvas_expandDollar (x->a_owner, x->a_unexpandedLabel);
                
        if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }

        outlet_new (cast_object (x), IS_FLOAT (&x->a_atom) ? &s_float : &s_symbol);
        
        glist_add (glist, cast_gobj (x));
        
    } else {                                                                    /* Interactive creation. */
    
        int positionX = 0;
        int positionY = 0;

        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll(glist);
        
        cast_object (x)->te_xCoordinate = positionX;
        cast_object (x)->te_yCoordinate = positionY;
        
        outlet_new (cast_object (x), IS_FLOAT (&x->a_atom) ? &s_float : &s_symbol);
                
        glist_add (glist, cast_gobj (x));
        
        canvas_selectObject (glist, cast_gobj (x));
    }
}

static void gatom_free (t_gatom *x)
{
    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gatom_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_gatom,
            NULL,
            (t_method)gatom_free,
            sizeof (t_gatom),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, gatom_bang);
    class_addFloat (c, gatom_float);
    class_addSymbol (c, gatom_symbol);
    class_addClick (c, gatom_click);
        
    class_addMethod (c, (t_method)gatom_set,    sym_set,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_param,  sym__gatomdialog,   A_GIMME, A_NULL);

    class_setWidgetBehavior (c, &gatom_widgetBehavior);
    class_setPropertiesFunction (c, gatom_behaviorProperties);
    
    gatom_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
