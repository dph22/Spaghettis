/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* my_numbox.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"

#include "g_iem.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define IEM_NUMBER_COLOR_EDITED            16711680

/*------------------ global varaibles -------------------------*/


/*------------------ global functions -------------------------*/

static void my_numbox_key(void *z, t_float fkey);
static void my_numbox_draw_update(t_gobj *client, t_glist *glist);

/* ------------ nmx gui-my number box ----------------------- */

t_widgetbehavior my_numbox_widgetbehavior;
static t_class *my_numbox_class;

/* widget helper functions */

static void my_numbox_tick_reset(t_my_numbox *x)
{
    if(x->x_hasChanged && x->x_gui.iem_glist)
    {
        x->x_hasChanged = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    }
}

static void my_numbox_tick_wait(t_my_numbox *x)
{
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
}

void my_numbox_clip(t_my_numbox *x)
{
    if(x->x_val < x->x_min)
        x->x_val = x->x_min;
    if(x->x_val > x->x_max)
        x->x_val = x->x_max;
}

void my_numbox_calc_fontwidth(t_my_numbox *x)
{
    int w, f=31;

    if(x->x_gui.iem_fontStyle == 1)
        f = 27;
    else if(x->x_gui.iem_fontStyle == 2)
        f = 25;

    w = x->x_gui.iem_fontSize * f * x->x_gui.iem_width;
    w /= 36;
    x->x_numwidth = w + (x->x_gui.iem_height / 2) + 4;
}

void my_numbox_ftoa(t_my_numbox *x)
{
    double f=x->x_val;
    int bufsize, is_exp=0, i, idecimal;

    sprintf(x->x_buf, "%g", f);
    bufsize = strlen(x->x_buf);
    if(bufsize >= 5)/* if it is in exponential mode */
    {
        i = bufsize - 4;
        if((x->x_buf[i] == 'e') || (x->x_buf[i] == 'E'))
            is_exp = 1;
    }
    if(bufsize > x->x_gui.iem_width)/* if to reduce */
    {
        if(is_exp)
        {
            if(x->x_gui.iem_width <= 5)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            i = bufsize - 4;
            for(idecimal=0; idecimal < i; idecimal++)
                if(x->x_buf[idecimal] == '.')
                    break;
            if(idecimal > (x->x_gui.iem_width - 4))
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
            {
                int new_exp_index=x->x_gui.iem_width-4, old_exp_index=bufsize-4;

                for(i=0; i < 4; i++, new_exp_index++, old_exp_index++)
                    x->x_buf[new_exp_index] = x->x_buf[old_exp_index];
                x->x_buf[x->x_gui.iem_width] = 0;
            }

        }
        else
        {
            for(idecimal=0; idecimal < bufsize; idecimal++)
                if(x->x_buf[idecimal] == '.')
                    break;
            if(idecimal > x->x_gui.iem_width)
            {
                x->x_buf[0] = (f < 0.0 ? '-' : '+');
                x->x_buf[1] = 0;
            }
            else
                x->x_buf[x->x_gui.iem_width] = 0;
        }
    }
}

static void my_numbox_draw_update(t_gobj *client, t_glist *glist)
{
    t_my_numbox *x = (t_my_numbox *)client;
    if (glist_isvisible(glist))
    {
        if(x->x_hasChanged)
        {
            if(x->x_buf[0])
            {
                char *cp=x->x_buf;
                int sl = strlen(x->x_buf);

                x->x_buf[sl] = '>';
                x->x_buf[sl+1] = 0;
                if(sl >= x->x_gui.iem_width)
                    cp += sl - x->x_gui.iem_width + 1;
                sys_vGui(
                    ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                         glist_getcanvas(glist), x, IEM_NUMBER_COLOR_EDITED, cp);
                x->x_buf[sl] = 0;
            }
            else
            {
                my_numbox_ftoa(x);
                sys_vGui(
                    ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                    glist_getcanvas(glist), x, IEM_NUMBER_COLOR_EDITED, x->x_buf);
                x->x_buf[0] = 0;
            }
        }
        else
        {
            my_numbox_ftoa(x);
            sys_vGui(
                ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                glist_getcanvas(glist), x,
                x->x_gui.iem_isSelected?
                    IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground,
                x->x_buf);
            x->x_buf[0] = 0;
        }
    }
}

static void my_numbox_draw_new(t_my_numbox *x, t_glist *glist)
{
    int half=x->x_gui.iem_height/2, d=1+x->x_gui.iem_height/34;
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(
".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d -outline #%6.6x \
-fill #%6.6x -tags %lxBASE1\n",
             canvas, xpos, ypos,
             xpos + x->x_numwidth-4, ypos,
             xpos + x->x_numwidth, ypos+4,
             xpos + x->x_numwidth, ypos + x->x_gui.iem_height,
             xpos, ypos + x->x_gui.iem_height,
             IEM_COLOR_NORMAL, x->x_gui.iem_colorBackground, x);
    sys_vGui(
        ".x%lx.c create line %d %d %d %d %d %d -fill #%6.6x -tags %lxBASE2\n",
        canvas, xpos, ypos,
        xpos + half, ypos + half,
        xpos, ypos + x->x_gui.iem_height,
        x->x_gui.iem_colorForeground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
        -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
        canvas, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY,
        strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
        x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);
    my_numbox_ftoa(x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
        -font [::getFont %d] -fill #%6.6x -tags %lxNUMBER\n",
        canvas, xpos+half+2, ypos+half+d,
        x->x_buf, x->x_gui.iem_fontSize,
        x->x_gui.iem_colorForeground, x);

        /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas,
             xpos, ypos + x->x_gui.iem_height-1,
             xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height,
             x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos, ypos,
             xpos+INLETS_WIDTH, ypos+1,
             x, 0);*/
}

static void my_numbox_draw_move(t_my_numbox *x, t_glist *glist)
{
    int half = x->x_gui.iem_height/2, d=1+x->x_gui.iem_height/34;
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c coords %lxBASE1 %d %d %d %d %d %d %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + x->x_numwidth-4, ypos,
             xpos + x->x_numwidth, ypos+4,
             xpos + x->x_numwidth, ypos + x->x_gui.iem_height,
             xpos, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxBASE2 %d %d %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + half, ypos + half,
             xpos, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY);
    sys_vGui(".x%lx.c coords %lxNUMBER %d %d\n",
             canvas, x, xpos+half+2, ypos+half+d);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos + x->x_gui.iem_height-1,
             xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos,
             xpos+INLETS_WIDTH, ypos+1);*/
}

static void my_numbox_draw_erase(t_my_numbox* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE1\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxBASE2\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxNUMBER\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void my_numbox_draw_config(t_my_numbox* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxNUMBER -font [::getFont %d] -fill #%6.6x \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground);
    sys_vGui(".x%lx.c itemconfigure %lxBASE1 -fill #%6.6x\n", canvas,
             x, x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n", canvas,
             x, x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground);
}

static void my_numbox_draw_io(t_my_numbox* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas,
        xpos, ypos + x->x_gui.iem_height-1,
        xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height,
        x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas,
        xpos, ypos,
        xpos+INLETS_WIDTH, ypos+1,
        x, 0);*/
}

static void my_numbox_draw_select(t_my_numbox *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_isSelected)
    {
        if(x->x_hasChanged)
        {
            x->x_hasChanged = 0;
            clock_unset(x->x_clock_reset);
            x->x_buf[0] = 0;
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        }
        sys_vGui(".x%lx.c itemconfigure %lxBASE1 -outline #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE1 -outline #%6.6x\n",
            canvas, x, IEM_COLOR_NORMAL);
        sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorForeground);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorLabel);
        sys_vGui(".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorForeground);
    }
}

void my_numbox_draw(t_my_numbox *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        interface_guiQueueAddIfNotAlreadyThere(x, glist, my_numbox_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        my_numbox_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        my_numbox_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        my_numbox_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        my_numbox_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        my_numbox_draw_config(x, glist);
    else if(mode >= IEM_DRAW_IO)
        my_numbox_draw_io(x, glist);
}

/* ------------------------ nbx widgetbehaviour----------------------------- */


static void my_numbox_getrect(t_gobj *z, t_glist *glist,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_my_numbox* x = (t_my_numbox*)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist);
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_numwidth;
    *yp2 = *yp1 + x->x_gui.iem_height;
}

static void my_numbox_save(t_gobj *z, t_buffer *b)
{
    t_my_numbox *x = (t_my_numbox *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iem_save(&x->x_gui, srl, bflcol);
    if(x->x_hasChanged)
    {
        x->x_hasChanged = 0;
        clock_unset(x->x_clock_reset);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    }
    buffer_vAppend(b, "ssiisiiffiisssiiiiiiifi", gensym("#X"),gensym("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym("nbx"), x->x_gui.iem_width, x->x_gui.iem_height,
                (t_float)x->x_min, (t_float)x->x_max,
                x->x_isLogarithmic, iemgui_saveLoadAtStart(&x->x_gui),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_saveFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_val, x->x_log_height);
    buffer_vAppend(b, ";");
}

int my_numbox_check_minmax(t_my_numbox *x, double min, double max)
{
    int ret=0;

    if(x->x_isLogarithmic)
    {
        if((min == 0.0)&&(max == 0.0))
            max = 1.0;
        if(max > 0.0)
        {
            if(min <= 0.0)
                min = 0.01*max;
        }
        else
        {
            if(min > 0.0)
                max = 0.01*min;
        }
    }
    x->x_min = min;
    x->x_max = max;
    if(x->x_val < x->x_min)
    {
        x->x_val = x->x_min;
        ret = 1;
    }
    if(x->x_val > x->x_max)
    {
        x->x_val = x->x_max;
        ret = 1;
    }
    if(x->x_isLogarithmic)
        x->x_k = exp(log(x->x_max/x->x_min)/(double)(x->x_log_height));
    else
        x->x_k = 1.0;
    return(ret);
}

static void my_numbox_properties(t_gobj *z, t_glist *owner)
{
    t_my_numbox *x = (t_my_numbox *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);
    if(x->x_hasChanged)
    {
        x->x_hasChanged = 0;
        clock_unset(x->x_clock_reset);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);

    }
    sprintf(buf, "::ui_iem::create %%s Number \
            %d %d Digits %d %d {Box Height} \
            %g {Value Low} %g {Value High} \
            %d Linear Logarithmic \
            %d \
            %d 1024 {Logarithmic Steps} \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, 1, x->x_gui.iem_height, 8,
            x->x_min, x->x_max,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadOnStart,
            x->x_log_height, /*no multi, but iem-characteristic*/
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground,
                0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void my_numbox_bang(t_my_numbox *x)
{
    outlet_float(x->x_gui.iem_obj.te_outlet, x->x_val);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_float(x->x_gui.iem_send->s_thing, x->x_val);
}

static void my_numbox_dialog(t_my_numbox *x, t_symbol *s, int argc,
    t_atom *argv)
{
    t_symbol *srl[3];
    int w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
    double min = (double)atom_getFloatAtIndex(2, argc, argv);
    double max = (double)atom_getFloatAtIndex(3, argc, argv);
    int lilo = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
    int log_height = (int)(t_int)atom_getFloatAtIndex(6, argc, argv);

    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    iem_dialog(&x->x_gui, srl, argc, argv);
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(h < 8)
        h = 8;
    x->x_gui.iem_height = h;
    if(log_height < 10)
        log_height = 10;
    x->x_log_height = log_height;
    my_numbox_calc_fontwidth(x);
    /*if(my_numbox_check_minmax(x, min, max))
     my_numbox_bang(x);*/
    my_numbox_check_minmax(x, min, max);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_IO);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_text*)x);
}

static void my_numbox_motion(t_my_numbox *x, t_float dx, t_float dy)
{
    double k2=1.0;

    if(x->x_gui.iem_accurateMoving)
        k2 = 0.01;
    if(x->x_isLogarithmic)
        x->x_val *= pow(x->x_k, -k2*dy);
    else
        x->x_val -= k2*dy;
    my_numbox_clip(x);
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    my_numbox_bang(x);
    clock_unset(x->x_clock_reset);
}

static void my_numbox_click(t_my_numbox *x, t_float xpos, t_float ypos,
                            t_float shift, t_float ctrl, t_float alt)
{
    glist_grab(x->x_gui.iem_glist, &x->x_gui.iem_obj.te_g,
        (t_glistmotionfn)my_numbox_motion, my_numbox_key, xpos, ypos);
}

static int my_numbox_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_my_numbox* x = (t_my_numbox *)z;

    if(doit)
    {
        my_numbox_click( x, (t_float)xpix, (t_float)ypix,
            (t_float)shift, 0, (t_float)alt);
        if(shift)
            x->x_gui.iem_accurateMoving = 1;
        else
            x->x_gui.iem_accurateMoving = 0;
        if(!x->x_hasChanged)
        {
            clock_delay(x->x_clock_wait, 50);
            x->x_hasChanged = 1;
            clock_delay(x->x_clock_reset, 3000);

            x->x_buf[0] = 0;
        }
        else
        {
            x->x_hasChanged = 0;
            clock_unset(x->x_clock_reset);
            x->x_buf[0] = 0;
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        }
    }
    return (1);
}

static void my_numbox_set(t_my_numbox *x, t_float f)
{
    if(x->x_val != f)
    {
        x->x_val = f;
        my_numbox_clip(x);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    }
}

static void my_numbox_log_height(t_my_numbox *x, t_float lh)
{
    if(lh < 10.0)
        lh = 10.0;
    x->x_log_height = (int)lh;
    if(x->x_isLogarithmic)
        x->x_k = exp(log(x->x_max/x->x_min)/(double)(x->x_log_height));
    else
        x->x_k = 1.0;
    
}

static void my_numbox_float(t_my_numbox *x, t_float f)
{
    my_numbox_set(x, f);
    if(x->x_gui.iem_goThrough)
        my_numbox_bang(x);
}

static void my_numbox_size(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{
    int h, w;

    w = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(ac > 1)
    {
        h = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
        if(h < 8)
            h = 8;
        x->x_gui.iem_height = h;
    }
    my_numbox_calc_fontwidth(x);
    iem_size((void *)x, &x->x_gui);
}

static void my_numbox_delta(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void my_numbox_pos(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void my_numbox_range(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{
    if(my_numbox_check_minmax(x, (double)atom_getFloatAtIndex(0, ac, av),
                              (double)atom_getFloatAtIndex(1, ac, av)))
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        /*my_numbox_bang(x);*/
    }
}

static void my_numbox_color(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void my_numbox_send(t_my_numbox *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void my_numbox_receive(t_my_numbox *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void my_numbox_label(t_my_numbox *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void my_numbox_label_pos(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void my_numbox_label_font(t_my_numbox *x,
    t_symbol *s, int ac, t_atom *av)
{
    int f = (int)(t_int)atom_getFloatAtIndex(1, ac, av);

    if(f < 4)
        f = 4;
    x->x_gui.iem_fontSize = f;
    f = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    if((f < 0) || (f > 2))
        f = 0;
    x->x_gui.iem_fontStyle = f;
    my_numbox_calc_fontwidth(x);
    iem_label_font((void *)x, &x->x_gui, s, ac, av);
}

static void my_numbox_log(t_my_numbox *x)
{
    x->x_isLogarithmic = 1;
    if(my_numbox_check_minmax(x, x->x_min, x->x_max))
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        /*my_numbox_bang(x);*/
    }
}

static void my_numbox_lin(t_my_numbox *x)
{
    x->x_isLogarithmic = 0;
}

static void my_numbox_init(t_my_numbox *x, t_float f)
{
    x->x_gui.iem_loadOnStart = (f==0.0)?0:1;
}

static void my_numbox_loadbang(t_my_numbox *x)
{
    if(x->x_gui.iem_loadOnStart)
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        my_numbox_bang(x);
    }
}

static void my_numbox_key(void *z, t_float fkey)
{
    t_my_numbox *x = z;
    char c=fkey;
    char buf[3];
    buf[1] = 0;

    if (c == 0)
    {
        x->x_hasChanged = 0;
        clock_unset(x->x_clock_reset);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        return;
    }
    if(((c>='0')&&(c<='9'))||(c=='.')||(c=='-')||
        (c=='e')||(c=='+')||(c=='E'))
    {
        if(strlen(x->x_buf) < (IEM_NUMBER_BUFFER_LENGTH-2))
        {
            buf[0] = c;
            strcat(x->x_buf, buf);
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
        }
    }
    else if((c=='\b')||(c==127))
    {
        int sl=strlen(x->x_buf)-1;

        if(sl < 0)
            sl = 0;
        x->x_buf[sl] = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    }
    else if((c=='\n')||(c==13))
    {
        x->x_val = atof(x->x_buf);
        x->x_buf[0] = 0;
        x->x_hasChanged = 0;
        clock_unset(x->x_clock_reset);
        my_numbox_clip(x);
        my_numbox_bang(x);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, my_numbox_draw_update);
    }
    clock_delay(x->x_clock_reset, 3000);
}

static void my_numbox_list(t_my_numbox *x, t_symbol *s, int ac, t_atom *av)
{
    if (IS_FLOAT_AT(av,0))
    {
        my_numbox_set(x, atom_getFloatAtIndex(0, ac, av));
        my_numbox_bang(x);
    }
}

static void *my_numbox_new(t_symbol *s, int argc, t_atom *argv)
{
    t_my_numbox *x = (t_my_numbox *)pd_new(my_numbox_class);
    int bflcol[]={-262144, -1, -1};
    int w=5, h=14;
    int lilo=0, f=0, ldx=0, ldy=-8;
    int fs=10;
    int log_height=256;
    double min=-1.0e+37, max=1.0e+37,v=0.0;
    char str[144];

    if((argc >= 17)&&IS_FLOAT_AT(argv,0)&&IS_FLOAT_AT(argv,1)
       &&IS_FLOAT_AT(argv,2)&&IS_FLOAT_AT(argv,3)
       &&IS_FLOAT_AT(argv,4)&&IS_FLOAT_AT(argv,5)
       &&(IS_SYMBOL_AT(argv,6)||IS_FLOAT_AT(argv,6))
       &&(IS_SYMBOL_AT(argv,7)||IS_FLOAT_AT(argv,7))
       &&(IS_SYMBOL_AT(argv,8)||IS_FLOAT_AT(argv,8))
       &&IS_FLOAT_AT(argv,9)&&IS_FLOAT_AT(argv,10)
       &&IS_FLOAT_AT(argv,11)&&IS_FLOAT_AT(argv,12)&&IS_FLOAT_AT(argv,13)
       &&IS_FLOAT_AT(argv,14)&&IS_FLOAT_AT(argv,15)&&IS_FLOAT_AT(argv,16))
    {
        w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        min = (double)atom_getFloatAtIndex(2, argc, argv);
        max = (double)atom_getFloatAtIndex(3, argc, argv);
        lilo = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
        iemgui_loadLoadAtStart(&x->x_gui, (t_int)atom_getFloatAtIndex(5, argc, argv));
        iemgui_loadNamesByIndex(&x->x_gui, 6, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(9, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
        iemgui_loadFontStyle(&x->x_gui, (t_int)atom_getFloatAtIndex(11, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
        bflcol[1] = (int)(t_int)atom_getFloatAtIndex(14, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(15, argc, argv);
        v = atom_getFloatAtIndex(16, argc, argv);
    }
    else iemgui_loadNamesByIndex(&x->x_gui, 6, 0);
    if((argc == 18)&&IS_FLOAT_AT(argv,17))
    {
        log_height = (int)(t_int)atom_getFloatAtIndex(17, argc, argv);
    }
    x->x_gui.iem_draw = (t_iemfn)my_numbox_draw;
    x->x_gui.iem_canSend = 1;
    x->x_gui.iem_canReceive = 1;
    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if(x->x_gui.iem_loadOnStart)
        x->x_val = v;
    else
        x->x_val = 0.0;
    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    if(log_height < 10)
        log_height = 10;
    x->x_log_height = log_height;
    if (!strcmp(x->x_gui.iem_send->s_name, "empty"))
        x->x_gui.iem_canSend = 0;
    if (!strcmp(x->x_gui.iem_receive->s_name, "empty"))
        x->x_gui.iem_canReceive = 0;

    if (x->x_gui.iem_canReceive)
        pd_bind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    x->x_gui.iem_labelX = ldx;
    x->x_gui.iem_labelY = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.iem_fontSize = fs;
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(h < 8)
        h = 8;
    x->x_gui.iem_height = h;
    x->x_buf[0] = 0;
    my_numbox_calc_fontwidth(x);
    my_numbox_check_minmax(x, min, max);
    iemgui_saveColors(&x->x_gui, bflcol);
    iemgui_checkSendReceiveLoop(&x->x_gui);
    x->x_clock_reset = clock_new(x, (t_method)my_numbox_tick_reset);
    x->x_clock_wait = clock_new(x, (t_method)my_numbox_tick_wait);
    x->x_hasChanged = 0;
    outlet_new(&x->x_gui.iem_obj, &s_float);
    return (x);
}

static void my_numbox_free(t_my_numbox *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    clock_free(x->x_clock_reset);
    clock_free(x->x_clock_wait);
    gfxstub_deleteforkey(x);
}

void g_numbox_setup(void)
{
    my_numbox_class = class_new(gensym("nbx"), (t_newmethod)my_numbox_new,
        (t_method)my_numbox_free, sizeof(t_my_numbox), 0, A_GIMME, 0);
    class_addCreator((t_newmethod)my_numbox_new, gensym("my_numbox"),
        A_GIMME, 0);
    class_addBang(my_numbox_class,my_numbox_bang);
    class_addFloat(my_numbox_class,my_numbox_float);
    class_addList(my_numbox_class, my_numbox_list);
    class_addMethod(my_numbox_class, (t_method)my_numbox_click,
        gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_motion,
        gensym("motion"), A_FLOAT, A_FLOAT, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_dialog,
        gensym("dialog"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_loadbang,
        gensym("loadbang"), 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_set,
        gensym("set"), A_FLOAT, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_size,
        gensym("size"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_delta,
        gensym("delta"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_pos,
        gensym("pos"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_range,
        gensym("range"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_color,
        gensym("color"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_send,
        gensym("send"), A_DEFSYMBOL, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_receive,
        gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_label,
        gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_label_pos,
        gensym("label_pos"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_label_font,
        gensym("label_font"), A_GIMME, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_log,
        gensym("log"), 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_lin,
        gensym("lin"), 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_init,
        gensym("init"), A_FLOAT, 0);
    class_addMethod(my_numbox_class, (t_method)my_numbox_log_height,
        gensym("log_height"), A_FLOAT, 0);
    my_numbox_widgetbehavior.w_getrectfn =    my_numbox_getrect;
    my_numbox_widgetbehavior.w_displacefn =   iem_displace;
    my_numbox_widgetbehavior.w_selectfn =     iem_select;
    my_numbox_widgetbehavior.w_activatefn =   NULL;
    my_numbox_widgetbehavior.w_deletefn =     iem_delete;
    my_numbox_widgetbehavior.w_visfn =        iem_vis;
    my_numbox_widgetbehavior.w_clickfn =      my_numbox_newclick;
    class_setWidget(my_numbox_class, &my_numbox_widgetbehavior);
    class_setHelpName(my_numbox_class, gensym("nbx"));
    class_setSaveFunction(my_numbox_class, my_numbox_save);
    class_setPropertiesFunction(my_numbox_class, my_numbox_properties);
}
