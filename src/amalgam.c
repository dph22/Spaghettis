
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Core. */

#include "m_symbols.c"
#include "m_instance.c"
#include "m_stack.c"
#include "m_pd.c"
#include "m_message.c"
#include "m_class.c"
#include "m_object.c"
#include "m_atom.c"
#include "m_buffer.c"
#include "m_parse.c"
#include "m_eval.c"
#include "m_setup.c"
#include "m_global.c"
#include "m_string.c"
#include "m_utils.c"
#include "m_dollar.c"
#include "m_bindlist.c"

/* System. */

#include "s_entry.c"
#include "s_main.c"
#include "s_scheduler.c"
#include "s_priority.c"
#include "s_handlers.c"
#include "s_time.c"
#include "s_receiver.c"
#include "s_interface.c"
#include "s_file.c"
#include "s_path.c"
#include "s_loader.c"
#include "s_preferences.c"
#include "s_midi.c"
#include "s_midiAPI.c"
#include "s_audio.c"
#include "s_audioAPI.c"
#include "s_memory.c"
#include "s_pathlist.c"
#include "s_font.c"
#include "s_post.c"
#include "s_logger.c"
#include "s_utf8.c"

/* Graphics. */

#include "g_gui.c"
#include "g_base.c"
#include "g_draw.c"
#include "g_select.c"
#include "g_editor.c"
#include "g_file.c"
#include "g_new.c"
#include "g_canvas.c"
#include "g_dsp.c"
#include "g_object.c"
#include "g_box.c"
#include "g_message.c"
#include "g_gatom.c"
#include "g_text.c"
#include "g_serialize.c"
#include "g_iem.c"
#include "g_bang.c"
#include "g_toggle.c"
#include "g_radio.c"
#include "g_slider.c"
#include "g_dial.c"
#include "g_vu.c"
#include "g_panel.c"

/* Control. */

/* DSP. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
