
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_delay_h_
#define __d_delay_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that all values (after creation) of the delay lines are used in DSP thread only. */
/* Except the size but that is read only. */
/* Thus no need of mutex machinery. */

typedef struct delwrite_tilde_control {
    int                         dw_size;
    int                         dw_phase;
    t_sample                    *dw_vector;
    } t_delwrite_tilde_control;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _delwrite_tilde {
    t_object                    dw_obj;                     /* Must be the first. */
    t_id                        dw_identifier;
    t_delwrite_tilde_control    dw_space;
    t_symbol                    *dw_name;
    } t_delwrite_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DELAY_EXTRA_SAMPLES     4       /* Required for 4-points interpolation. */
#define DELAY_ROUND_SAMPLES     4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_delay_h_
