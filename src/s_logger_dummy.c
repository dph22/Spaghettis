
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_WITH_DEBUG && PD_WITH_LOGGER )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error logger_initializeNative (void)
{
    return PD_ERROR_NONE;
}

void logger_releaseNative (void)
{
}

void logger_appendStringNative (const char *s)
{
}

void logger_appendFloatNative (double f)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
