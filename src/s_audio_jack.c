
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "jack/weakjack.h"
#include "jack/jack.h"
#include <regex.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_MAXIMUM_CLIENTS    128
#define JACK_MAXIMUM_PORTS      128
#define JACK_MAXIMUM_FRAMES     64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char                 *jack_clientNames[JACK_MAXIMUM_CLIENTS];            /* Shared. */
static jack_client_t        *jack_client;                                       /* Shared. */

static t_sample             *jack_bufferIn;                                     /* Shared. */
static t_sample             *jack_bufferOut;                                    /* Shared. */
static jack_port_t          *jack_portsIn[JACK_MAXIMUM_PORTS];                  /* Shared. */
static jack_port_t          *jack_portsOut[JACK_MAXIMUM_PORTS];                 /* Shared. */

static int                  jack_numberOfPortsIn;                               /* Shared. */
static int                  jack_numberOfPortsOut;                              /* Shared. */

static jack_nframes_t       jack_maximumNumberOfFrames;                         /* Shared. */
static jack_nframes_t       jack_filledFrames;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static pthread_cond_t       jack_cond;                                          /* Shared. */
static pthread_mutex_t      jack_mutex;                                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_BUFFER_SIZE    4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pollprocess (jack_nframes_t nframes, void *arg)
{
    int j;
    jack_default_audio_sample_t *out, *in;

    pthread_mutex_lock(&jack_mutex);
    if (nframes > JACK_MAXIMUM_FRAMES) jack_maximumNumberOfFrames = nframes;
    else jack_maximumNumberOfFrames = JACK_MAXIMUM_FRAMES;
    if (jack_filledFrames >= nframes)
    {
        if (jack_filledFrames != nframes)
            fprintf(stderr,"Partial read\n");
        /* hmm, how to find out whether 't_sample' and
            'jack_default_audio_sample_t' are actually the same type??? */
        if (sizeof(t_sample)==sizeof(jack_default_audio_sample_t)) 
        {
            for (j = 0; j < audio_getChannelsOut();  j++)
            {
                if (out = jack_port_get_buffer(jack_portsOut[j], nframes))
                    memcpy(out, jack_bufferOut + (j * JACK_BUFFER_SIZE),
                        sizeof (jack_default_audio_sample_t) * nframes);
            }
            for (j = 0; j < audio_getChannelsIn(); j++)
            {
                if (in = jack_port_get_buffer(jack_portsIn[j], nframes))
                    memcpy(jack_bufferIn + (j * JACK_BUFFER_SIZE), in,
                        sizeof (jack_default_audio_sample_t) * nframes);
            }
        } 
        else
        {
            unsigned int frame=0;
            t_sample*data;
            for (j = 0; j < audio_getChannelsOut();  j++)
            {
                if (out = jack_port_get_buffer (jack_portsOut[j], nframes))
                {
                    data = jack_bufferOut + (j * JACK_BUFFER_SIZE);
                    for (frame=0; frame<nframes; frame++)
                        *out++ = *data++;
                }
            }
            for (j = 0; j < audio_getChannelsIn(); j++)
            {
                if (in = jack_port_get_buffer( jack_portsIn[j], nframes))
                {
                    data = jack_bufferIn + (j * JACK_BUFFER_SIZE);
                    for (frame=0; frame<nframes; frame++)
                        *data++ = *in++;
                }
            }
        }
        jack_filledFrames -= nframes;
    }
    else
    {           /* PD could not keep up ! */
        //if (jack_started) jack_dio_error = 1;
        for (j = 0; j < jack_numberOfPortsOut;  j++)
        {
            if (out = jack_port_get_buffer (jack_portsOut[j], nframes))
                memset(out, 0, sizeof (float) * nframes); 
            memset(jack_bufferOut + j * JACK_BUFFER_SIZE, 0, JACK_BUFFER_SIZE * sizeof(t_sample));
        }
        jack_filledFrames = 0;
    }
    pthread_cond_broadcast(&jack_cond);
    pthread_mutex_unlock(&jack_mutex);
    return 0;
}

static int jack_srate (jack_nframes_t srate, void *arg)
{
    audio_setSampleRate (srate);
    return 0;
}

static void jack_shutdown (void *arg)
{
  post_error ("JACK: server shut down");
  
  jack_deactivate (jack_client);
  //jack_client_close(jack_client); /* likely to hang if the server shut down */
  jack_client = NULL;

    // audio_setAPI (NULL, API_NONE); // set pd_whichapi 0
    
    audio_close();
}

static void pd_jack_error_callback(const char *desc) {
  post_error ("JACKerror: %s", desc);
  return;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void jack_releaseClientNames (void)
{
    int i;
    for (i = 0; i < JACK_MAXIMUM_CLIENTS; i++) {
        if (jack_clientNames[i]) { PD_MEMORY_FREE (jack_clientNames[i]); } jack_clientNames[i] = NULL;
    }
}

static void jack_initializeClientNames (void)
{
    const char **ports;
    
    jack_releaseClientNames();
    
    ports = jack_get_ports (jack_client, "", "", 0);
    
    if (ports) {
    //
    int i, n = 0;
    
    regex_t e; regcomp (&e, "^[^:]*", REG_EXTENDED);

    for (i = 0; ports[i] != NULL && n < JACK_MAXIMUM_CLIENTS; i++) {
    //
    int j, seen = 0;
    regmatch_t info;
    size_t size = 0;
    char t[PD_STRING] = { 0 };
    
    /* Parse "clientname:portname" syntax (i.e. "system:playback_1" to "system"). */ 
    
    regexec (&e, ports[i], 1, &info, 0);
    size = PD_MIN (info.rm_eo - info.rm_so, PD_STRING - 1);
    memcpy (t, &ports[i][info.rm_so], size);
    t[size] = 0;
        
    /* Do we know about this port's client yet? */

    for (j = 0; j < n; j++) { if (strcmp (t, jack_clientNames[j]) == 0 ) { seen = 1; } }

    /* Append the new ones. */
    
    if (!seen) {
    //
    jack_clientNames[n] = PD_MEMORY_GET (strlen (t) + 1);

    if ((strcmp ( "alsa_pcm", t) == 0) && (n > 0)) {        /* The "alsa_pcm" client MUST be the first. */
        char *tmp = jack_clientNames[n];
        jack_clientNames[n] = jack_clientNames[0];
        jack_clientNames[0] = tmp;
        strcpy (jack_clientNames[0], t);
        
    } else {
        strcpy (jack_clientNames[n], t);
    }
    
    n++;
    //
    }
    //
    }

    jack_free (ports);
    //
    }
}

static int jack_connectPorts (char *client)
{
    char  regex_pattern[100]; /* its always the same, ... */
    int i;
    const char **jack_ports;

    if (strlen(client) > 96)  return -1;

    sprintf( regex_pattern, "%s:.*", client );

    jack_ports = jack_get_ports( jack_client, regex_pattern,
                                 NULL, JackPortIsOutput);
    if (jack_ports)
    {
        for (i=0;jack_ports[i] != NULL && i < audio_getChannelsIn();i++)      
            if (jack_connect (jack_client, jack_ports[i],
               jack_port_name (jack_portsIn[i]))) 
                  post_error ("JACK: cannot connect input ports %s -> %s",
                      jack_ports[i],jack_port_name (jack_portsIn[i]));
        free(jack_ports);
    }
    jack_ports = jack_get_ports( jack_client, regex_pattern,
                                 NULL, JackPortIsInput);
    if (jack_ports)
    {
        for (i=0;jack_ports[i] != NULL && i < audio_getChannelsOut();i++)      
          if (jack_connect (jack_client, jack_port_name (jack_portsOut[i]),
            jack_ports[i])) 
              post_error ( "JACK: cannot connect output ports %s -> %s",
                jack_port_name (jack_portsOut[i]),jack_ports[i]);

        free(jack_ports);
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error jack_open (int numberOfChannelsIn, int numberOfChannelsOut)
{
    #if PD_APPLE    /* Jackmp linked as a weak framework. */
        
    if (!jack_client_open) {
        post_error (PD_TRANSLATE ("audio: can't find JACK framework")); return PD_ERROR;    // --
    }
    
    #endif

    if (numberOfChannelsIn || numberOfChannelsOut) {
    //
    jack_status_t status;
    
    numberOfChannelsIn  = PD_MIN (numberOfChannelsIn, JACK_MAXIMUM_PORTS);
    numberOfChannelsOut = PD_MIN (numberOfChannelsOut, JACK_MAXIMUM_PORTS);
 
    PD_ASSERT (!jack_client);

    jack_client = jack_client_open ("puredata", JackNoStartServer, &status, NULL);

    if (jack_client) {
    //
    int i;
    
    PD_ASSERT (!jack_bufferIn);
    PD_ASSERT (!jack_bufferOut);
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); }
    
    if (numberOfChannelsIn) { 
        jack_bufferIn = PD_MEMORY_GET (numberOfChannelsIn * JACK_BUFFER_SIZE * sizeof (t_sample));
    }

    if (numberOfChannelsOut) {
        jack_bufferOut = PD_MEMORY_GET (numberOfChannelsOut * JACK_BUFFER_SIZE * sizeof (t_sample));
    }

    jack_set_process_callback (jack_client, pollprocess, NULL);
    jack_set_error_function (pd_jack_error_callback);
    jack_set_sample_rate_callback (jack_client, jack_srate, NULL);
    jack_on_shutdown (jack_client, jack_shutdown, NULL);

    audio_setSampleRate (jack_get_sample_rate (jack_client));

    for (i = 0; i < numberOfChannelsIn; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "input%d", i);
    jack_portsIn[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if (!jack_portsIn[i]) {
        post_error (PD_TRANSLATE ("audio: JACK can only register %d input ports"), i);
        break;
    }
    //
    }

    audio_shrinkChannelsIn (jack_numberOfPortsIn = numberOfChannelsIn = i);
    
    for (i = 0; i < numberOfChannelsOut; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "output%d", i);
    jack_portsOut[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (!jack_portsOut[i]) {
        post_error (PD_TRANSLATE ("audio: JACK can only register %d output ports"), i);
        break;  
    }
    //
    }
    
    audio_shrinkChannelsOut (jack_numberOfPortsOut = numberOfChannelsOut = i);
    
    if (!jack_activate (jack_client)) {
    //
    jack_initializeClientNames();
    
    if (jack_clientNames[0]) { jack_connectPorts (jack_clientNames[0]); }
    
    pthread_mutex_init (&jack_mutex, NULL);
    pthread_cond_init (&jack_cond, NULL);
    
    return PD_ERROR_NONE;
    }
    //
    }
    //
    }
    
    audio_shrinkChannelsIn (0); audio_shrinkChannelsOut (0); 
    
    return PD_ERROR;
}

void jack_close (void) 
{
    if (jack_client) {
    //
    int i;
    jack_deactivate (jack_client);
    for (i = 0; i < jack_numberOfPortsIn; i++)  { 
        jack_port_unregister (jack_client, jack_portsIn[i]); 
        jack_portsIn[i] = NULL;
    }
    for (i = 0; i < jack_numberOfPortsOut; i++) { 
        jack_port_unregister (jack_client, jack_portsOut[i]);
        jack_portsOut[i] = NULL;
    }
    jack_client_close (jack_client);
    jack_client = NULL;
    
    pthread_cond_broadcast (&jack_cond);
    pthread_cond_destroy (&jack_cond);
    pthread_mutex_destroy (&jack_mutex);
    //
    }
    
    jack_releaseClientNames();
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  jack_bufferIn = NULL;  }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); jack_bufferOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int jack_pollDSP (void)
{
    t_sample * fp;
    int j;
    int rtnval =  DACS_YES;
    int timenow;
    int timeref = sys_getRealTimeInSeconds();
    if (!jack_client) return DACS_NO;
    if (!audio_getChannelsIn() && !audio_getChannelsOut()) return (DACS_NO); 
    if (0 /*jack_dio_error*/)
    {
        //sys_log_error(ERROR_RESYNC);
        //jack_dio_error = 0;
    }
    pthread_mutex_lock(&jack_mutex);
    if (jack_filledFrames >= jack_maximumNumberOfFrames)
        pthread_cond_wait(&jack_cond,&jack_mutex);

    if (!jack_client)
    {
        pthread_mutex_unlock(&jack_mutex);
        return DACS_NO;
    }

    fp = audio_soundOut;
    for (j = 0; j < audio_getChannelsOut(); j++)
    {
        memcpy(jack_bufferOut + (j * JACK_BUFFER_SIZE) + jack_filledFrames, fp,
            AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));
        fp += AUDIO_DEFAULT_BLOCKSIZE;  
    }
    fp = audio_soundIn;
    for (j = 0; j < audio_getChannelsIn(); j++)
    {
        memcpy(fp, jack_bufferIn + (j * JACK_BUFFER_SIZE) + jack_filledFrames,
            AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));
        fp += AUDIO_DEFAULT_BLOCKSIZE;
    }
    jack_filledFrames += AUDIO_DEFAULT_BLOCKSIZE;
    pthread_mutex_unlock(&jack_mutex);

    if ((timenow = sys_getRealTimeInSeconds()) - timeref > 0.002)
    {
        rtnval = DACS_SLEPT;
    }
    memset(audio_soundOut, 0, AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample)*audio_getChannelsOut());
    return rtnval;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error jack_getLists (char *devicesIn, 
    int *numberOfDevicesIn,
    char *devicesOut,
    int *numberOfDevicesOut,
    int *canMultiple) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (devicesIn,  MAXIMUM_DESCRIPTION, "JACK port");
    err |= string_copy (devicesOut, MAXIMUM_DESCRIPTION, "JACK port");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
    *canMultiple        = 0;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
