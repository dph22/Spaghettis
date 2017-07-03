
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int dsp_status;  /* Global */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void dsp_setState (int n)
{
    n = (n != 0);
    
    if (n != dsp_status) {
    //
    if (n) { if (audio_start() == PD_ERROR_NONE) { instance_dspStart(); dsp_status = 1; } }
    else {
        instance_dspStop(); dsp_status = 0; audio_stop();
    }
    
    gui_vAdd ("set ::var(isDsp) %d\n", dsp_status);     // --
    //
    }
}

int dsp_getState (void)
{
    return dsp_status;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_update (void)
{
    dsp_resume (dsp_suspend());
}

int dsp_suspend (void)
{
    int n = dsp_status; if (n) { instance_dspStop(); dsp_status = 0; } return n;
}

void dsp_resume (int n)
{
    if (n) { instance_dspStart(); dsp_status = 1; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
