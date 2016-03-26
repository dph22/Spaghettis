
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol *iemgui_empty (void)
{
    return gensym ("empty");
}

static t_symbol *iemgui_parseEmpty (t_symbol *s)
{
    if (s == &s_) { return iemgui_empty(); }
    else { 
        return s;
    }
}

static t_symbol *iemgui_expandDollar (t_iem *iem, t_symbol *s)
{
    t_symbol *t = canvas_realizedollar (iem, s); return  (t == NULL ? iemgui_empty() : t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Ensure compatibility with the original format. */
/* By the way legacy predefined colors are not supported. */
/* Only the 6 MSB are kept for each component. */

// RRRRRRRRGGGGGGGGBBBBBBBB 
// RRRRRR..GGGGGG..BBBBBB..
// RRRRRRGGGGGGBBBBBB

static int iemgui_colorEncode (int color)
{
    int n = 0;
    
    n |= ((0xfc0000 & color) >> 6);
    n |= ((0xfc00 & color) >> 4);
    n |= ((0xfc & color) >> 2);
                      
    return (-1 -n);
}

static int iemgui_colorDecode (int color)
{
    PD_ASSERT (color < 0);
    
    int n = 0;
    
    color = (-1 -color);
    
    n |= ((color & 0x3f000) << 6);
    n |= ((color & 0xfc0) << 4);
    n |= ((color & 0x3f) << 2);

    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* For convenience floats are loaded as integers (mainly in order to enumerate things). */

static t_symbol *iemgui_fetchName (int i, t_atom *argv)
{
    if (IS_SYMBOL_AT (argv, i))     { return (atom_getSymbol (argv + i)); }
    else if (IS_FLOAT_AT (argv, i)) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "%d", (int)atom_getFloat (argv + i));
        return gensym (t);
    } else {
        return iemgui_empty();
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Unexpanded names cannot be fetch at instantiation due to already substituted arguments. */
/* Consequently it must be done by looking up again the object's text buffer. */

static void iemgui_fetchUnexpanded (t_iem *iem, t_symbol **s, int i, t_symbol *fallback)
{
    if (!*s) {
        t_error err = PD_ERROR;
        t_buffer *b = iem->iem_obj.te_buffer;
        if (i < buffer_size (b)) {
            char t[PD_STRING];
            if (!(err = atom_toString (buffer_atoms (b) + i, t, PD_STRING))) { *s = gensym (t); }
        }
        if (err) {
            *s = (fallback ? fallback : iemgui_empty());
        }
    }
}

static void iemgui_fetchUnexpandedNames (t_iem *iem, t_iemnames *s)
{
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedSend, iem->iem_cacheIndex + 1, iem->iem_send);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedReceive, iem->iem_cacheIndex + 2, iem->iem_receive);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedLabel, iem->iem_cacheIndex + 3, iem->iem_label);
        
    s->n_unexpandedSend    = iem->iem_unexpandedSend;
    s->n_unexpandedReceive = iem->iem_unexpandedReceive;
    s->n_unexpandedLabel   = iem->iem_unexpandedLabel;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_serializeColors (t_iem *iem, t_iemcolors *c)
{
    c->c_colorBackground = iemgui_colorEncode (iem->iem_colorBackground);
    c->c_colorForeground = iemgui_colorEncode (iem->iem_colorForeground);
    c->c_colorLabel      = iemgui_colorEncode (iem->iem_colorLabel);
}

void iemgui_deserializeColors (t_iem *iem, t_iemcolors *c)
{
    iem->iem_colorBackground = iemgui_colorDecode (c->c_colorBackground);
    iem->iem_colorForeground = iemgui_colorDecode (c->c_colorForeground);
    iem->iem_colorLabel      = iemgui_colorDecode (c->c_colorLabel);
}

void iemgui_serializeFontStyle (t_iem *iem, int n)
{
    iem->iem_fontStyle = (char)n;
}

int iemgui_deserializeFontStyle (t_iem *iem)
{
    return (int)iem->iem_fontStyle;
}

void iemgui_serializeLoadOnStart (t_iem *iem, int n)
{
    iem->iem_loadOnStart = ((n & 1) != 0);
    iem->iem_scale = (n & 2);
}

int iemgui_deserializeLoadOnStart (t_iem *iem)
{
    return ((iem->iem_loadOnStart ? 1 : 0) | (iem->iem_scale ? 2 : 0));
}

void iemgui_serializeNames (t_iem *iem, t_iemnames *n)
{
    iemgui_fetchUnexpandedNames (iem, n);
    n->n_unexpandedSend    = dollar_toRaute (n->n_unexpandedSend);
    n->n_unexpandedReceive = dollar_toRaute (n->n_unexpandedReceive);
    n->n_unexpandedLabel   = dollar_toRaute (n->n_unexpandedLabel);
}

void iemgui_deserializeNamesByIndex (t_iem *iem, int i, t_atom *argv)
{
    iem->iem_send    = (argv ? iemgui_fetchName (i + 0, argv) : iemgui_empty());
    iem->iem_receive = (argv ? iemgui_fetchName (i + 1, argv) : iemgui_empty());
    iem->iem_label   = (argv ? iemgui_fetchName (i + 2, argv) : iemgui_empty());
    
    iem->iem_unexpandedSend    = NULL;
    iem->iem_unexpandedReceive = NULL;
    iem->iem_unexpandedLabel   = NULL;
    
    iem->iem_cacheIndex = i;    /* Cache this index for later lookup. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_checkSendReceiveLoop (t_iem *iem)
{
    iem->iem_goThrough = 1;
    
    if (iem->iem_canSend && iem->iem_canReceive) {
        if (iem->iem_send == iem->iem_receive) { iem->iem_goThrough = 0; }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_setSend (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    iem->iem_unexpandedSend = t;
    iem->iem_send = iemgui_expandDollar (iem->iem_glist, t);
    iem->iem_canSend = (s == iemgui_empty()) ? 0 : 1;
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setReceive (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    if (iem->iem_canReceive) { pd_unbind (pd_cast (iem), iem->iem_receive); }
    iem->iem_unexpandedReceive = t;
    iem->iem_receive = iemgui_expandDollar (iem->iem_glist, t);
    iem->iem_canReceive = (s == iemgui_empty()) ? 0 : 1;
    if (iem->iem_canReceive) { pd_bind (pd_cast (iem), iem->iem_receive); }
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setLabel (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    iem->iem_unexpandedLabel = t;
    iem->iem_label = iemgui_expandDollar (iem->iem_glist, t);

    if (glist_isvisible (iem->iem_glist)) {
        sys_vGui (".x%lx.c itemconfigure %lxLABEL -text {%s}\n",    // --
            glist_getcanvas (iem->iem_glist),
            x,
            iem->iem_label != iemgui_empty() ? iem->iem_label->s_name : s_.s_name);
    }
}

void iemgui_setLabelPosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_labelX = (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_labelY = (int)atom_getFloatAtIndex (1, argc, argv);
    
    if (glist_isvisible (iem->iem_glist)) {
        sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
            glist_getcanvas (iem->iem_glist),
            x,
            text_xpix (object_cast (x), iem->iem_glist) + iem->iem_labelX,
            text_ypix (object_cast (x), iem->iem_glist) + iem->iem_labelY);
    }
}

void iemgui_setLabelFont (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    int f = (int)atom_getFloatAtIndex (1, argc, argv);
    f = PD_MAX (f, IEM_MINIMUM_FONTSIZE);
    iem->iem_fontSize = f;
    if (glist_isvisible (iem->iem_glist)) {
        sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d]\n",      // --
            glist_getcanvas (iem->iem_glist), 
            x,
            iem->iem_fontSize);
    }
}

void iemgui_setColor (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (glist_isvisible (iem->iem_glist)) { (*iem->iem_draw) (x, iem->iem_glist, IEM_DRAW_CONFIG); }
}

void iemgui_setPosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_obj.te_xCoordinate = (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_obj.te_yCoordinate = (int)atom_getFloatAtIndex (1, argc, argv);
    
    iemgui_boxChanged (x, iem);
}

void iemgui_movePosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_obj.te_xCoordinate += (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_obj.te_yCoordinate += (int)atom_getFloatAtIndex (1, argc, argv);
    
    iemgui_boxChanged (x, iem);
}

void iemgui_boxChanged (void *x, t_iem *iem)
{
    if (glist_isvisible (iem->iem_glist)) {
    //
    (*iem->iem_draw) (x, iem->iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines (iem->iem_glist, object_cast (x));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_behaviorDisplace (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_iem *x = iem_cast (z);

    x->iem_obj.te_xCoordinate += deltaX;
    x->iem_obj.te_yCoordinate += deltaY;
    
    (*x->iem_draw) ((void *)z, glist, IEM_DRAW_MOVE);
    
    canvas_fixlines (glist, object_cast (z));
}

void iemgui_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_iem *x = iem_cast (z);

    x->iem_isSelected = isSelected;
    
    (*x->iem_draw) ((void *)z, glist, IEM_DRAW_SELECT);
}

void iemgui_behaviorVisible (t_gobj *z, t_glist *glist, int isVisible)
{
    t_iem *x = iem_cast (z);

    if (isVisible) { (*x->iem_draw) ((void *)z, glist, IEM_DRAW_NEW); }
    else {
        (*x->iem_draw) ((void *)z, glist, IEM_DRAW_ERASE);
        interface_guiQueueRemove (z);
    }
}

void iemgui_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    canvas_deletelines (glist, object_cast (z));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_serialize (t_iem *iem, t_iemnames *n, t_iemcolors *c)
{
    iemgui_fetchUnexpandedNames (iem, n);
    iemgui_serializeColors (iem, c);
}

void iemgui_fromDialog (t_iem *iem, int argc, t_atom *argv)
{
    int loadOnStart             = (int)atom_getFloatAtIndex (5,  argc, argv);
    int labelX                  = (int)atom_getFloatAtIndex (10, argc, argv);
    int labelY                  = (int)atom_getFloatAtIndex (11, argc, argv);
    int fontSize                = (int)atom_getFloatAtIndex (12, argc, argv);
    int backgroundColor         = (int)atom_getFloatAtIndex (13, argc, argv);
    int foregroundColor         = (int)atom_getFloatAtIndex (14, argc, argv);
    int labelColor              = (int)atom_getFloatAtIndex (15, argc, argv);
    int canSend                 = 1;
    int canReceive              = 1;

    t_symbol *s1 = dollar_fromRaute (iemgui_fetchName (7, argv));
    t_symbol *s2 = dollar_fromRaute (iemgui_fetchName (8, argv));
    t_symbol *s3 = dollar_fromRaute (iemgui_fetchName (9, argv));

    iem->iem_unexpandedSend     = s1;
    iem->iem_unexpandedReceive  = s2;
    iem->iem_unexpandedLabel    = s3;
    
    s1 = iemgui_expandDollar (iem->iem_glist, s1);
    s2 = iemgui_expandDollar (iem->iem_glist, s2);
    s3 = iemgui_expandDollar (iem->iem_glist, s3);
    
    if (s1 == iemgui_empty()) { canSend = 0;    }
    if (s2 == iemgui_empty()) { canReceive = 0; }
    
    if (canReceive) {
        if (iem->iem_canReceive) { pd_unbind (pd_cast (iem), iem->iem_receive); }
        iem->iem_receive = s2;
        pd_bind (pd_cast (iem), iem->iem_receive);
    }

    iem->iem_canSend            = canSend;
    iem->iem_canReceive         = canReceive;
    iem->iem_loadOnStart        = (loadOnStart != 0);
    iem->iem_labelX             = labelX;
    iem->iem_labelY             = labelY;
    iem->iem_fontSize           = PD_MAX (fontSize, IEM_MINIMUM_FONTSIZE);
    iem->iem_colorForeground    = foregroundColor & 0xffffff;
    iem->iem_colorBackground    = backgroundColor & 0xffffff;
    iem->iem_colorLabel         = labelColor & 0xffffff;
    iem->iem_send               = s1;
    iem->iem_label              = s3;
    
    iemgui_checkSendReceiveLoop (iem);
    
    canvas_dirty (iem->iem_glist, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
