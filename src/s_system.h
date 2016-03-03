
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_system_h_
#define __s_system_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DACS_NO                                 0
#define DACS_YES                                1 
#define DACS_SLEPT                              2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_AUDIO_NONE                    0
#define SCHEDULER_AUDIO_POLL                    1 
#define SCHEDULER_AUDIO_CALLBACK                2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXIMUM_MIDI_IN                         8
#define MAXIMUM_MIDI_OUT                        8
#define MAXIMUM_AUDIO_IN                        4
#define MAXIMUM_AUDIO_OUT                       4
#define MAXIMUM_CHANNELS_IN                     32
#define MAXIMUM_CHANNELS_OUT                    32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXIMUM_DEVICES                         16
#define MAXIMUM_DESCRIPTION                     128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_SAMPLERATE                44100
#define AUDIO_DEFAULT_BLOCKSIZE                 64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS
    #define AUDIO_DEFAULT_ADVANCE               80
#elif PD_APPLE
    #define AUDIO_DEFAULT_ADVANCE               5
#else
    #define AUDIO_DEFAULT_ADVANCE               25
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define API_NONE                                0
#define API_ALSA                                1
#define API_OSS                                 2
#define API_PORTAUDIO                           4
#define API_JACK                                5
#define API_DUMMY                               9

//#define API_MMIO                              3
//#define API_SGI                               6
//#define API_AUDIOUNIT                         7
//#define API_ESD                               8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined ( USEAPI_ALSA )
    #define API_DEFAULT_AUDIO                   API_ALSA
    
#elif defined ( USEAPI_OSS )
    #define API_DEFAULT_AUDIO                   API_OSS
    
#elif defined ( USEAPI_PORTAUDIO )
    #define API_DEFAULT_AUDIO                   API_PORTAUDIO
    
#elif defined ( USEAPI_JACK )
    #define API_DEFAULT_AUDIO                   API_JACK
    
#elif defined ( USEAPI_DUMMY )
    #define API_DEFAULT_AUDIO                   API_DUMMY
#else
    #error "Unknown Audio API"
#endif 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined ( USEAPI_ALSA )
    #define API_WITH_ALSA                       1
#else
    #define API_WITH_ALSA                       0
#endif

#if defined ( USEAPI_OSS )
    #define API_WITH_OSS                        1
#else
    #define API_WITH_OSS                        0
#endif

#if defined ( USEAPI_PORTAUDIO )
    #define API_WITH_PORTAUDIO                  1
#else
    #define API_WITH_PORTAUDIO                  0
#endif

#if defined ( USEAPI_JACK )
    #define API_WITH_JACK                       1
#else
    #define API_WITH_JACK                       0
#endif

#if defined ( USEAPI_DUMMY )
    #define API_WITH_DUMMY                      1
#else
    #define API_WITH_DUMMY                      0
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* LCM (32000, 44100, 48000, 88200, 96000). */
    
#define SYSTIME_CLOCKS_PER_MILLISECOND          (double)(32.0 * 441.0)
#define SYSTIME_CLOCKS_PER_SECOND               (SYSTIME_CLOCKS_PER_MILLISECOND * 1000.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_pollfn)        (void *p, int fd);
typedef void (*t_notifyfn)      (void *owner, int fd);
typedef void (*t_receivefn)     (void *owner, t_buffer *b);
typedef void (*t_clockfn)       (void *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _clock {
    double          c_systime;          /* Negative for unset clocks. */
    double          c_unit;             /* A positive value is in ticks, negative for number of samples. */
    t_clockfn       c_fn;
    void            *c_owner;
    struct _clock   *c_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _receiver {
    void            *r_owner;
    t_buffer        *r_message;
    char            *r_inRaw;
    int             r_inHead;
    int             r_inTail;
    int             r_fd;
    int             r_isUdp;
    int             r_isClosed;
    t_notifyfn      r_fnNotify;
    t_receivefn     r_fnReceive;
    } t_receiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pathlist {
    struct _pathlist    *pl_next;
    char                *pl_string;
    } t_pathlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef int t_fontsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pathlist  *pathlist_newAppend                     (t_pathlist *x, const char *s);
t_pathlist  *pathlist_newAppendEncoded              (t_pathlist *x, t_symbol *s);
char        *pathlist_getFileAtIndex                (t_pathlist *x, int n);
char        *pathlist_getFile                       (t_pathlist *x);
t_pathlist  *pathlist_getNext                       (t_pathlist *x);

void        pathlist_free                           (t_pathlist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        font_withHostMeasured                   (void *dummy, t_symbol *s, int argc, t_atom *argv);
t_fontsize  font_getNearestValidFontSize            (int size);
int         font_getHostFontSize                    (t_fontsize fontSize);
int         font_getHostFontWidth                   (t_fontsize fontSize);
int         font_getHostFontHeight                  (t_fontsize fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         main_entry                              (int argc, char **argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double      scheduler_getLogicalTime                (void);
double      scheduler_getLogicalTimeAfter           (double ms);
double      scheduler_getMillisecondsSince          (double systime);
double      scheduler_getUnitsSince                 (double systime, double unit, int isSamples);
void        scheduler_setAudioMode                  (int flag);
void        scheduler_needToRestart                 (void);
void        scheduler_needToExit                    (void);
void        scheduler_needToExitWithError           (void);
void        scheduler_lock                          (void);
void        scheduler_unlock                        (void);
void        scheduler_audioCallback                 (void);
t_error     scheduler_main                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        ugen_tick                               (void);

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

void        clock_setUnitAsSamples                  (t_clock *x, double samples);
void        clock_setUnitAsMilliseconds             (t_clock *x, double ms);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_setSignalHandlers                   (void);
double      sys_getRealTimeInSeconds                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        post_atoms                              (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_receiver  *receiver_new                           (void *owner,
                                                        int fd,
                                                        t_notifyfn notify,          /* Socket closed. */
                                                        t_receivefn receive,        /* Data received. */
                                                        int isUdp);

void        receiver_free                           (t_receiver *x);
void        receiver_read                           (t_receiver *x, int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         interface_monitorBlocking               (int microseconds);
int         interface_monitorNonBlocking            (void);
void        interface_monitorAddPoller              (int fd, t_pollfn fn, void *ptr);
void        interface_monitorRemovePoller           (int fd);
void        interface_guiQueueAddIfNotAlreadyThere  (void *owner, t_glist *glist, t_guifn f);
void        interface_guiQueueRemove                (void *owner);
int         interface_pollOrFlushGui                (void);
void        interface_closeSocket                   (int fd);
void        interface_quit                          (void *dummy);
void        interface_watchdog                      (void *dummy);
t_error     interface_start                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

FILE        *file_openWrite                         (const char *filepath);
int         file_openRaw                            (const char *filepath, int oflag);

int         file_openWithDirectoryAndName           (const char *directory,
                                                        const char *name,
                                                        const char *extension,
                                                        char *directoryResult,
                                                        char **nameResult,
                                                        size_t size);
                                                        
int         file_openConsideringSearchPath          (const char *directory, 
                                                        const char *name,
                                                        const char *extension,
                                                        char *directoryResult,
                                                        char **nameResult,
                                                        size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        path_slashToBackslashIfNecessary        (char *dest, char *src);
void        path_backslashToSlashIfNecessary        (char *dest, char *src);
int         path_isFileExist                        (const char *filepath);
int         path_isFileExistAsRegularFile           (const char *filepath);
t_error     path_withDirectoryAndName               (char *dest, 
                                                        size_t size, 
                                                        const char *directory, 
                                                        const char *name,
                                                        int expandEnvironment);
                                                        
t_error     path_expandEnvironment                  (char *dest, size_t size, const char *src);
void        path_setSearchPath                      (void *dummy, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         loader_loadExternal                     (t_canvas *canvas, char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        preferences_load                        (void);
void        preferences_save                        (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_initialize                         (void);
void        midi_synchronise                        (void);
void        midi_poll                               (void);
void        midi_receive                            (int port, int byte);
void        midi_broadcast                          (int port, int hasOneByte, int a, int b, int c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        midi_requireDialog                      (void *dummy);
void        midi_fromDialog                         (void *dummy, t_symbol *s, int argc, t_atom *argv);
int         midi_numberWithName                     (int isOutput, const char *name);
t_error     midi_numberToName                       (int isOutput, int k, char *dest, size_t size);
void        midi_open                               (void);
void        midi_close                              (void);

void        midi_openWithDevices                    (int numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int numberOfDevicesOut,
                                                        int *devicesOut);
                                                        
void        midi_getDevices                         (int *numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *numberOfDevicesOut,
                                                        int *devicesOut);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        inmidi_noteOn                           (int port, int channel, int pitch, int velocity);
void        inmidi_controlChange                    (int port, int channel, int control, int value);
void        inmidi_programChange                    (int port, int channel, int value);
void        inmidi_pitchBend                        (int port, int channel, int value);
void        inmidi_afterTouch                       (int port, int channel, int value);
void        inmidi_polyPressure                     (int port, int channel, int pitch, int value);
void        inmidi_realTimeIn                       (int port, int command);
void        inmidi_byte                             (int port, int byte);
void        inmidi_sysex                            (int port, int byte);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        outmidi_noteOn                          (int port, int channel, int pitch, int velocity);
void        outmidi_controlChange                   (int port, int channel, int control, int value);
void        outmidi_programChange                   (int port, int channel, int value);
void        outmidi_pitchBend                       (int port, int channel, int value);
void        outmidi_afterTouch                      (int port, int channel, int value);
void        outmidi_polyPressure                    (int port, int channel, int pitch, int value);
void        outmidi_clock                           (int port);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        audio_release                           (void);
int         audio_pollDSP                           (void);
t_error     audio_stopDSP                           (void);
t_error     audio_startDSP                          (void);
t_float     audio_getSampleRate                     (void);
int         audio_getChannelsIn                     (void);
int         audio_getChannelsOut                    (void);

void        audio_setAPI                            (void *dummy, t_float f);
t_error     audio_getAPIAvailables                  (char *dest, size_t size);
int         audio_isAPIAvailable                    (int api);
void        audio_requireDialog                     (void *dummy);
void        audio_fromDialog                        (void *dummy, t_symbol *s, int argc, t_atom *argv);
int         audio_numberWithName                    (int isOutput, const char *name);
t_error     audio_numberToName                      (int isOutput, int k, char *dest, size_t size);
t_error     audio_open                              (void);
void        audio_close                             (void);
int         audio_isOpened                          (void);

void        audio_getDevices                        (int *numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *channelsIn,
                                                        int *numberOfDevicesOut,
                                                        int *devicesOut,
                                                        int *channelsOut,
                                                        int *sampleRate,
                                                        int *advance,
                                                        int *withCallback,
                                                        int *blockSize);

void        audio_setDefaultDevicesAndParameters    (int numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *channelsIn,
                                                        int numberOfDevicesOut,
                                                        int *devicesOut,
                                                        int *channelsOut,
                                                        int sampleRate,
                                                        int advance,
                                                        int withCallback,
                                                        int blockSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_apis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
