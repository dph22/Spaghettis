
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_dspthread_h_
#define __d_dspthread_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_dspthread     *dspthread_new      (void);

t_error         dspthread_create    (t_dspthread *x);
void            dspthread_free      (t_dspthread *x);

void            dspthread_run       (t_dspthread *x);
void            dspthread_stop      (t_dspthread *x);
t_systime       dspthread_time      (t_dspthread *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dspthread_h_
