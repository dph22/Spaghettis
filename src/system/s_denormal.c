
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( PD_INTEL ) && defined ( __SSE3__ )

#include <pmmintrin.h>
#include <xmmintrin.h>

/* < https://www.carlh.net/plugins/denormals.php > */
/* < https://en.wikipedia.org/wiki/Denormal_number > */

void denormal_setPolicy (void)
{
    _MM_SET_DENORMALS_ZERO_MODE (_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
}

#else

void denormal_setPolicy (void)
{
    // -- TODO: Manage ARM case (RPI for instance).
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------