
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_symbols_h_
#define __m_symbols_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void symbols_initialize (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol s_pointer;
extern t_symbol s_float;
extern t_symbol s_symbol;
extern t_symbol s_bang;
extern t_symbol s_list;
extern t_symbol s_anything;
extern t_symbol s_signal;
extern t_symbol s__N;
extern t_symbol s__X;
extern t_symbol s__A;
extern t_symbol s_;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *sym___comma__;
extern t_symbol *sym___semicolon__;
extern t_symbol *sym__arraydialog;
extern t_symbol *sym__audiodialog;
extern t_symbol *sym__audioproperties;
extern t_symbol *sym__canvasdialog;
extern t_symbol *sym__cut;
extern t_symbol *sym__copy;
extern t_symbol *sym__data;
extern t_symbol *sym__duplicate;
extern t_symbol *sym__dummy;
extern t_symbol *sym__end;
extern t_symbol *sym__font;
extern t_symbol *sym__key;
extern t_symbol *sym__keyup;
extern t_symbol *sym__keyname;
extern t_symbol *sym__map;
extern t_symbol *sym__mididialog;
extern t_symbol *sym__midiproperties;
extern t_symbol *sym__paste;
extern t_symbol *sym__path;
extern t_symbol *sym__pop;
extern t_symbol *sym__popupdialog;
extern t_symbol *sym__quit;
extern t_symbol *sym__savepreferences;
extern t_symbol *sym__selectall;
extern t_symbol *sym__signoff;
extern t_symbol *sym__watchdog;
extern t_symbol *sym__A;
extern t_symbol *sym__N;
extern t_symbol *sym__X;
extern t_symbol *sym_array;
extern t_symbol *sym_atom;
extern t_symbol *sym_bindlist;
extern t_symbol *sym_bng;
extern t_symbol *sym_bounds;
extern t_symbol *sym_canvas;
extern t_symbol *sym_canvasmaker;
extern t_symbol *sym_clear;
extern t_symbol *sym_click;
extern t_symbol *sym_close;
extern t_symbol *sym_cnv;
extern t_symbol *sym_connect;
extern t_symbol *sym_coords;
extern t_symbol *sym_data;
extern t_symbol *sym_dirty;
extern t_symbol *sym_disconnect;
extern t_symbol *sym_dsp;
extern t_symbol *sym_editmode;
extern t_symbol *sym_f;
extern t_symbol *sym_floatatom;
extern t_symbol *sym_guiconnect;
extern t_symbol *sym_guistub;
extern t_symbol *sym_graph;
extern t_symbol *sym_hold;
extern t_symbol *sym_hradio;
extern t_symbol *sym_hslider;
extern t_symbol *sym_inlet;
extern t_symbol *sym_inlet__tilde__;
extern t_symbol *sym_key;
extern t_symbol *sym_linear;
extern t_symbol *sym_loadbang;
extern t_symbol *sym_mergefile;
extern t_symbol *sym_motion;
extern t_symbol *sym_mouse;
extern t_symbol *sym_mouseup;
extern t_symbol *sym_msg;
extern t_symbol *sym_nbx;
extern t_symbol *sym_new;
extern t_symbol *sym_obj;
extern t_symbol *sym_objectmaker;
extern t_symbol *sym_open;
extern t_symbol *sym_outlet;
extern t_symbol *sym_outlet__tilde__;
extern t_symbol *sym_pad;
extern t_symbol *sym_pd;
extern t_symbol *sym_quit;
extern t_symbol *sym_read;
extern t_symbol *sym_rename;
extern t_symbol *sym_restore;
extern t_symbol *sym_saveto;
extern t_symbol *sym_savetofile;
extern t_symbol *sym_scalar;
extern t_symbol *sym_send;
extern t_symbol *sym_set;
extern t_symbol *sym_setbounds;
extern t_symbol *sym_sort;
extern t_symbol *sym_struct;
extern t_symbol *sym_subpatch;
extern t_symbol *sym_symbolatom;
extern t_symbol *sym_template;
extern t_symbol *sym_text;
extern t_symbol *sym_tgl;
extern t_symbol *sym_visible;
extern t_symbol *sym_vradio;
extern t_symbol *sym_vslider;
extern t_symbol *sym_vu;
extern t_symbol *sym_write;
extern t_symbol *sym_xticks;
extern t_symbol *sym_yticks;
extern t_symbol *sym_BackSpace;
extern t_symbol *sym_Delete;
extern t_symbol *sym_Down;
extern t_symbol *sym_Escape;
extern t_symbol *sym_Left;
extern t_symbol *sym_Return;
extern t_symbol *sym_Right;
extern t_symbol *sym_Space;
extern t_symbol *sym_Tab;
extern t_symbol *sym_Up;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_LEGACY

extern t_symbol *sym_lin;
extern t_symbol *sym_menu__dash__open;
extern t_symbol *sym_menuarray;
extern t_symbol *sym_menuclose;
extern t_symbol *sym_menusave;
extern t_symbol *sym_menusaveas;
extern t_symbol *sym_mycnv;
extern t_symbol *sym_numbox;
extern t_symbol *sym_page;
extern t_symbol *sym_toggle;
extern t_symbol *sym_vis;
extern t_symbol *sym_vumeter;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_symbols_h_
