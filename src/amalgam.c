
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
#include "m_global.c"
#include "m_bind.c"
#include "m_setup.c"
#include "m_dollar.c"
#include "m_string.c"
#include "m_utils.c"
#include "m_math.c"
#include "m_error.c"
#include "m_color.c"

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
#include "s_font.c"
#include "s_post.c"
#include "s_logger.c"
#include "s_utf8.c"

/* Helpers. */

#include "h_iterator.c"
#include "h_pathlist.c"
#include "h_heapstring.c"

/* Graphics. */

#include "g_guistub.c"
#include "g_guiconnect.c"
#include "g_rectangle.c"
#include "g_base.c"
#include "g_draw.c"
#include "g_select.c"
#include "g_editor.c"
#include "g_file.c"
#include "g_new.c"
#include "g_graph.c"
#include "g_canvas.c"
#include "g_dsp.c"
#include "g_object.c"
#include "g_box.c"
#include "g_message.c"
#include "g_text.c"
#include "g_table.c"
#include "g_gatom.c"
#include "g_garray.c"
#include "g_vinlet.c"
#include "g_voutlet.c"

#include "g_iem.c"
#include "g_bang.c"
#include "g_toggle.c"
#include "g_radio.c"
#include "g_slider.c"
#include "g_dial.c"
#include "g_vu.c"
#include "g_panel.c"

#include "g_word.c"
#include "g_scalar.c"
#include "g_field.c"
#include "g_array.c"
#include "g_gpointer.c"
#include "g_template.c"
#include "g_struct.c"
#include "g_pointer.c"
#include "g_get.c"
#include "g_set.c"
#include "g_element.c"
#include "g_getsize.c"
#include "g_setsize.c"
#include "g_append.c"
#include "g_drawpolygon.c"
#include "g_plot.c"
#include "g_drawnumber.c"
#include "g_paint.c"
#include "g_serialize.c"

/* Control. */

#include "x_textbuffer.c"
#include "x_textclient.c"
#include "x_arrayclient.c"
#include "x_arrayrange.c"
#include "x_listinlet.c"
#include "x_atomoutlet.c"

#include "x_int.c"
#include "x_float.c"
#include "x_symbol.c"
#include "x_bang.c"

#include "x_list.c"
#include "x_listappend.c"
#include "x_listprepend.c"
#include "x_listsplit.c"
#include "x_listtrim.c"
#include "x_listlength.c"
#include "x_listfromsymbol.c"
#include "x_listtosymbol.c"

#include "x_scalar.c"

#include "x_text.c"
#include "x_textget.c"
#include "x_textset.c"
#include "x_textsize.c"
#include "x_textlist.c"
#include "x_textsearch.c"
#include "x_textsequence.c"
#include "x_qlist.c"
#include "x_textfile.c"

#include "x_array.c"
#include "x_arraysize.c"
#include "x_arraysum.c"
#include "x_arrayget.c"
#include "x_arrayset.c"
#include "x_arrayquantile.c"
#include "x_arrayrandom.c"
#include "x_arraymax.c"
#include "x_arraymin.c"

#include "x_acoustic.c"
#include "x_math.c"
#include "x_atan2.c"
#include "x_binop1.c"
#include "x_binop2.c"
#include "x_binop3.c"

#include "x_metro.c"
#include "x_delay.c"
#include "x_line.c"
#include "x_timer.c"
#include "x_pipe.c"

#include "x_send.c"
#include "x_receive.c"
#include "x_select.c"
#include "x_route.c"
#include "x_pack.c"
#include "x_unpack.c"
#include "x_trigger.c"
#include "x_spigot.c"
#include "x_moses.c"
#include "x_until.c"
#include "x_swap.c"
#include "x_change.c"
#include "x_clip.c"
#include "x_value.c"

#include "x_print.c"
#include "x_makefilename.c"
#include "x_gui.c"
#include "x_keyname.c"

/* DSP. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
