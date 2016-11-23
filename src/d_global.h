
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_global_h_
#define __d_global_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _catch_tilde {
    t_object    x_obj;              /* Must be the first. */
    t_sample    *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_catch_tilde;

typedef struct _send_tilde {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    int         x_vectorSize;
    t_sample    *x_vector;
    t_symbol    *x_name;
    } t_send_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DSP_SEND_SIZE   64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_global_h_
