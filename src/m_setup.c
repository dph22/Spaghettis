
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void bindlist_setup (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void g_array_setup (void);
void g_canvas_setup (void);
void g_guiconnect_setup (void);
void g_bang_setup (void);
void g_hradio_setup (void);
void g_hslider_setup (void);
void g_mycanvas_setup (void);
void g_numbox_setup (void);
void g_toggle_setup (void);
void g_vradio_setup (void);
void g_vslider_setup (void);
void g_vumeter_setup (void);
void g_io_setup (void);
void g_scalar_setup (void);
void g_template_setup (void);
void g_text_setup (void);
void g_traversal_setup (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void x_acoustics_setup (void);
void x_interface_setup (void);
void x_connective_setup (void);
void x_time_setup (void);
void x_arithmetic_setup (void);
void x_array_setup (void);
void x_midi_setup (void);
void x_misc_setup (void);
void x_net_setup (void);
void x_qlist_setup (void);
void x_gui_setup (void);
void x_list_setup (void);
void x_scalar_setup (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void d_arithmetic_setup (void);
void d_array_setup (void);
void d_ctl_setup (void);
void d_dac_setup (void);
void d_delay_setup (void);
void d_fft_setup (void);
void d_filter_setup (void);
void d_global_setup (void);
void d_math_setup (void);
void d_misc_setup (void);
void d_osc_setup (void);
void d_soundfile_setup (void);
void d_ugen_setup (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setup_initialize (void)
{
    bindlist_setup();
        
    g_array_setup();
    g_canvas_setup();
    g_guiconnect_setup();
    g_bang_setup();
    g_hradio_setup();
    g_hslider_setup();
    g_mycanvas_setup();
    g_numbox_setup();
    g_toggle_setup();
    g_vradio_setup();
    g_vslider_setup();
    g_vumeter_setup();
    g_io_setup();
    g_scalar_setup();
    g_template_setup();
    g_text_setup();
    g_traversal_setup();
    
    x_acoustics_setup();
    x_interface_setup();
    x_connective_setup();
    x_time_setup();
    x_arithmetic_setup();
    x_array_setup();
    x_midi_setup();
    x_misc_setup();
    x_net_setup();
    x_qlist_setup();
    x_gui_setup();
    x_list_setup();
    x_scalar_setup();
    
    d_arithmetic_setup();
    d_array_setup();
    d_ctl_setup();
    d_dac_setup();
    d_delay_setup();
    d_fft_setup();
    d_filter_setup();
    d_global_setup();
    d_math_setup();
    d_misc_setup();
    d_osc_setup();
    d_soundfile_setup();
    d_ugen_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
