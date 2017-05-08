
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_system_h_
#define __s_system_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DACS_NO                         0
#define DACS_YES                        1 
#define DACS_SLEPT                      2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INTERNAL_BLOCKSIZE              64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_AUDIO_NONE            0
#define SCHEDULER_AUDIO_POLL            1 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_BLOCKSIZE         64
#define AUDIO_DEFAULT_SAMPLERATE        44100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* LCM (32000, 44100, 48000, 88200, 96000). */
    
#define SYSTIME_PER_MILLISECOND         (32.0 * 441.0)
#define SYSTIME_PER_SECOND              (SYSTIME_PER_MILLISECOND * 1000.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_pollfn)        (void *p, int fd);
typedef void (*t_notifyfn)      (void *owner, int fd);
typedef void (*t_receivefn)     (void *owner, t_buffer *b);
typedef void (*t_clockfn)       (void *owner);
typedef void (*t_drawfn)        (t_gobj *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _receiver {
    void                *r_owner;
    t_buffer            *r_message;
    char                *r_inRaw;
    int                 r_inHead;
    int                 r_inTail;
    int                 r_fd;
    int                 r_isUdp;
    int                 r_isBinary;
    int                 r_isClosed;
    t_notifyfn          r_fnNotify;
    t_receivefn         r_fnReceive;
    } t_receiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void sys_closeSocket (int fd)
{
    #if PD_WINDOWS
        closesocket (fd);
    #else
        close (fd);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         main_entry                              (int argc, char **argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     scheduler_main                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Usable in DSP perform. */

t_systime   scheduler_getLogicalTime                (void);
t_systime   scheduler_getLogicalTimeAfter           (double ms);
double      scheduler_getMillisecondsSince          (t_systime systime);    
double      scheduler_getUnitsSince                 (t_systime systime, double unit, int isSamples);
void        scheduler_setAudioMode                  (int flag);
void        scheduler_needToExit                    (void);
void        scheduler_needToExitWithError           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     priority_privilegeStart                 (void);
t_error     priority_privilegeDrop                  (void);
t_error     priority_privilegeRestore               (void);
t_error     priority_privilegeRelinquish            (void);

t_error     priority_setPolicy                      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_setSignalHandlers                   (void);
double      sys_getRealTimeInSeconds                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_receiver  *receiver_new                           (void *owner,
                                                            int fd,
                                                            t_notifyfn notify,          /* Socket closed. */
                                                            t_receivefn receive,        /* Data received. */
                                                            int isUdp, 
                                                            int isBinary);

void        receiver_free                           (t_receiver *x);
int         receiver_isClosed                       (t_receiver *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         monitor_blocking                        (int microseconds);
int         monitor_nonBlocking                     (void);
void        monitor_addPoller                       (int fd, t_pollfn fn, void *ptr);
void        monitor_removePoller                    (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        defer_addJob                            (void *owner, t_glist *glist, t_drawfn f);
void        defer_removeJob                         (void *owner);
int         defer_flushJobs                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         sys_guiPollOrFlush                      (void);
void        sys_guiFlush                            (void);
void        sys_vGui                                (char *format, ...);
void        sys_gui                                 (char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        interface_watchdog                      (void *dummy);
t_error     interface_start                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_start                              (void);
void        midi_poll                               (void);
void        midi_receive                            (int port, int byte);
void        midi_broadcast                          (int port, int hasOneByte, int a, int b, int c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        inmidi_noteOn                           (int port, int channel, int pitch, int velocity);
void        inmidi_controlChange                    (int port, int channel, int control, int value);
void        inmidi_programChange                    (int port, int channel, int value);
void        inmidi_pitchBend                        (int port, int channel, int value);
void        inmidi_afterTouch                       (int port, int channel, int value);
void        inmidi_polyPressure                     (int port, int channel, int pitch, int value);
void        inmidi_byte                             (int port, int byte);
void        inmidi_sysex                            (int port, int byte);
void        inmidi_realTime                         (int port, int byte);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        outmidi_noteOn                          (int channel, int pitch, int velocity);
void        outmidi_controlChange                   (int channel, int control, int value);
void        outmidi_programChange                   (int channel, int value);
void        outmidi_pitchBend                       (int channel, int value);
void        outmidi_afterTouch                      (int channel, int value);
void        outmidi_polyPressure                    (int channel, int pitch, int value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     audio_initialize                        (void);
void        audio_release                           (void);
int         audio_poll                              (void);
t_error     audio_stop                              (void);
t_error     audio_start                             (void);

void        audio_setSampleRate                     (t_float sampleRate);
void        audio_setBlockSize                      (int blockSize);

t_float     audio_getSampleRate                     (void);
int         audio_getBlockSize                      (void);
int         audio_getTotalOfChannelsIn              (void);
int         audio_getTotalOfChannelsOut             (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

FILE        *file_openWrite                         (const char *filepath);
int         file_openRaw                            (const char *filepath, int oflag);

int         file_openConsideringSearchPath          (const char *directory, 
                                                        const char *name,
                                                        const char *extension,
                                                        t_fileproperties *p);

void        file_openHelpPatch                      (t_gobj *y);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        path_slashToBackslashIfNecessary        (char *s);
void        path_backslashToSlashIfNecessary        (char *s);
int         path_isFileExist                        (const char *filepath);
int         path_isFileExistAsRegularFile           (const char *filepath);

t_error     path_withDirectoryAndName               (char *dest, 
                                                        size_t size, 
                                                        const char *directory, 
                                                        const char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         loader_load                             (t_glist *canvas, t_symbol *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     properties_loadBegin                    (void);
void        properties_loadClose                    (void);
t_error     properties_saveBegin                    (void);
void        properties_saveClose                    (void);
int         properties_getKey                       (const char *key, char *value, int size);
void        properties_setKey                       (const char *key, const char *value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        preferences_load                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        *sys_getMemoryChecked                   (size_t n, const char *f, int line);
void        *sys_getMemoryResizeChecked             (void *ptr,
                                                        size_t oldSize,
                                                        size_t newSize,
                                                        const char *f,
                                                        int line);

void        sys_freeMemoryChecked                   (void *ptr, const char *f, int line);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        leak_initialize                         (void);
void        leak_release                            (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        font_setDefaultFontSize                 (int size);
t_fontsize  font_getDefaultFontSize                 (void);
t_fontsize  font_getNearestValidFontSize            (int size);
int         font_getHostFontSize                    (t_fontsize fontSize);
double      font_getHostFontWidth                   (t_fontsize fontSize);
double      font_getHostFontHeight                  (t_fontsize fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     logger_initialize                       (void);
void        logger_release                          (void);
int         logger_isRunning                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_clock.h"
#include "s_devices.h"
#include "s_apis.h"
#include "s_midi_apis.h"
#include "s_audio_apis.h"
#include "s_logger_apis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
