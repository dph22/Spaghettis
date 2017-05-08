
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ( PD_WINDOWS || PD_CYGWIN )

void sys_setSignalHandlers (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void handlers_exit (int n)
{
    scheduler_needToExit();
}

static void handlers_hup (int n)        /* Watchdog barking. */
{
    #if PD_WATCHDOG
    
    struct timeval timeOut;
    
    timeOut.tv_sec  = 0;
    timeOut.tv_usec = 30000;
    
    select (1, NULL, NULL, NULL, &timeOut);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_setSignalHandlers (void)
{
    signal (SIGHUP,  handlers_hup);
    // signal (SIGTERM, handlers_exit);
    signal (SIGINT,  handlers_exit);
    signal (SIGQUIT, handlers_exit);
    signal (SIGILL,  handlers_exit);
    signal (SIGIOT,  handlers_exit);
    signal (SIGFPE,  SIG_IGN);
    signal (SIGPIPE, SIG_IGN);
    signal (SIGALRM, SIG_IGN);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
