
/* Copyright (c) 2010-2017 Peter Brinkmann. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *audio_nameNative (void)
{
    static char *name = "Dummy"; return name;
}

int audio_getPriorityNative (int min, int max, int isWatchdog)
{
    return (isWatchdog ? max - 5 : max - 7);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_initializeNative (void)
{
    return PD_ERROR_NONE;
}

void audio_releaseNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_openNative (t_devicesproperties *p) 
{
    return PD_ERROR_NONE;
}

void audio_closeNative() 
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_pollNative() 
{
    return DACS_NO;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_getListsNative (t_deviceslist *p) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= deviceslist_appendAudioIn (p,  "Dummy", 0);
    err |= deviceslist_appendAudioOut (p, "Dummy", 0);
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

