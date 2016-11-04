
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define POLL_PERIOD     47.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void poll_task (void *dummy)
{
    if (pd_isThingQuiet (sym__polling)) {
    //
    pd_message (pd_getThing (sym__polling), sym__polling, 0, NULL);
    //
    }  
    
    clock_delay (pd_this->pd_polling, POLL_PERIOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void poll_run (void)
{
    pd_this->pd_polling = clock_new ((void *)NULL, (t_method)poll_task);
    
    clock_delay (pd_this->pd_polling, POLL_PERIOD);
}

void poll_stop (void)
{
    clock_free (pd_this->pd_polling); pd_this->pd_polling = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void poll_add (t_pd *x)
{
    pd_bind (x, sym__polling);
}

void poll_remove (t_pd *x)
{
    pd_unbind (x, sym__polling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
