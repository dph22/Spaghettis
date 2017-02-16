
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_apis_h_
#define __s_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_open                               (void);
void        midi_close                              (void);

void        midi_getDevices                         (t_devicesproperties *p);
void        midi_setDevices                         (t_devicesproperties *p);

int         midi_deviceAsNumberWithString           (int isOutput, char *name);
t_error     midi_deviceAsStringWithNumber           (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     audio_open                              (void);
void        audio_close                             (void);
int         audio_isOpened                          (void);

void        audio_getDevices                        (t_devicesproperties *p);
void        audio_setDevices                        (t_devicesproperties *p);

int         audio_deviceAsNumberWithString          (int isOutput, char *name);
t_error     audio_deviceAsStringWithNumber          (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_apis_h_