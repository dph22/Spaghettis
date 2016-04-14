
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *global_object;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void global_default (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    post_error (PD_TRANSLATE ("%s: unknown method '%s'"), class_getName (pd_class (x)), s->s_name); // --
}

static void global_dummy (void *dummy)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_initialize (void)
{
    t_class *c = class_new (gensym ("pd"), NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);

    class_addMethod (c, (t_method)global_newPatch,          gensym ("new"),  A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)buffer_openFile,          gensym ("open"), A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_dsp,               gensym ("dsp"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_key,               gensym ("key"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)interface_quit,           gensym ("quit"), A_NULL);
    
    class_addMethod (c, (t_method)font_withHostMeasured,    gensym ("_font"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)audio_requireDialog,      gensym ("_audioProperties"), A_NULL);
    class_addMethod (c, (t_method)audio_fromDialog,         gensym ("_audioDialog"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)midi_requireDialog,       gensym ("_midiProperties"),  A_NULL);
    class_addMethod (c, (t_method)midi_fromDialog,          gensym ("_midiDialog"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)path_setSearchPath,       gensym ("_path"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,        gensym ("_quit"),            A_NULL);
    class_addMethod (c, (t_method)preferences_save,         gensym ("_savePreferences"), A_NULL);
    class_addMethod (c, (t_method)global_dummy,             gensym ("_dummy"),           A_NULL);
    
    #if PD_WATCHDOG
    
    class_addMethod (c, (t_method)interface_watchdog, gensym ("_watchdog"), A_NULL);
        
    #endif

    class_addAnything (c, global_default);
    
    global_object = c;
        
    pd_bind (&global_object, gensym ("pd"));
}

void global_release (void)
{
    pd_unbind (&global_object, gensym ("pd"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
