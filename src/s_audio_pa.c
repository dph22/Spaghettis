
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
#include "m_alloca.h"
#include "s_system.h"
#include "s_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <portaudio.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PA_WITH_FAKEBLOCKING

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_sample         *audio_soundIn;
extern t_sample         *audio_soundOut;

extern int              audio_channelsIn;
extern int              audio_channelsOut;
extern t_float          audio_sampleRate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaStream         *pa_stream;                     /* Shared. */
static float            *pa_soundIn;                    /* Shared. */
static float            *pa_soundOut;                   /* Shared. */
static char             *pa_bufferIn;                   /* Shared. */
static char             *pa_bufferOut;                  /* Shared. */

static sys_ringbuf      pa_ringOut;                     /* Shared. */
static sys_ringbuf      pa_ringIn;                      /* Shared. */

static int              pa_channelsIn;                  /* Shared. */
static int              pa_channelsOut;                 /* Shared. */
static int              pa_started;                     /* Shared. */
static int              pa_advanceInNumberOfBlocks;     /* Shared. */

static t_audiocallback  pa_callback;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int pa_lowLevelCallback (const void *inputBuffer,
    void *outputBuffer, unsigned long nframes,
    const PaStreamCallbackTimeInfo *outTime, PaStreamCallbackFlags myflags, 
    void *userData)
{
    int i; 
    unsigned int n, j;
    float *fbuf, *fp2, *fp3, *soundiop;
    if (nframes % AUDIO_DEFAULT_BLOCKSIZE)
    {
        post("warning: audio nframes %ld not a multiple of blocksize %d",
            nframes, (int)AUDIO_DEFAULT_BLOCKSIZE);
        nframes -= (nframes % AUDIO_DEFAULT_BLOCKSIZE);
    }
    for (n = 0; n < nframes; n += AUDIO_DEFAULT_BLOCKSIZE)
    {
        if (inputBuffer != NULL)
        {
            fbuf = ((float *)inputBuffer) + n*pa_channelsIn;
            soundiop = pa_soundIn;
            for (i = 0, fp2 = fbuf; i < pa_channelsIn; i++, fp2++)
                    for (j = 0, fp3 = fp2; j < AUDIO_DEFAULT_BLOCKSIZE;
                        j++, fp3 += pa_channelsIn)
                            *soundiop++ = *fp3;
        }
        else memset((void *)pa_soundIn, 0,
            AUDIO_DEFAULT_BLOCKSIZE * pa_channelsIn * sizeof(float));
        memset((void *)pa_soundOut, 0,
            AUDIO_DEFAULT_BLOCKSIZE * pa_channelsOut * sizeof(float));
        (*pa_callback)();
        if (outputBuffer != NULL)
        {
            fbuf = ((float *)outputBuffer) + n*pa_channelsOut;
            soundiop = pa_soundOut;
            for (i = 0, fp2 = fbuf; i < pa_channelsOut; i++, fp2++)
                for (j = 0, fp3 = fp2; j < AUDIO_DEFAULT_BLOCKSIZE;
                    j++, fp3 += pa_channelsOut)
                        *fp3 = *soundiop++;
        }
    }
    return 0;
}

static int pa_fakeBlockingCallback (const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    unsigned long availableOut = ringbuffer_getReadAvailable (&pa_ringOut);
    unsigned long requiredIn   = frameCount * sizeof (float) * pa_channelsIn;
    unsigned long requiredOut  = frameCount * sizeof (float) * pa_channelsOut;
    
    if (availableOut >= requiredOut) {
        if (output) { ringbuffer_read (&pa_ringOut, output, requiredOut, pa_bufferOut); }
        if (input)  { ringbuffer_write (&pa_ringIn, input, requiredOut, pa_bufferIn);   }
            
    } else { 
        if (output) {   /* Fill with zeros. */
            int i;
            for (i = 0; i < pa_channelsOut; i++) {
                unsigned long j;
                float *p = ((float *)output) + i;
                for (j = 0; j < frameCount; j++, p += pa_channelsOut) { *p = 0; }
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaError pa_openWithCallback (double sampleRate, 
    int numberOfChannelsIn,
    int numberOfChannelsOut,
    int blockSize,
    int advanceInNumberOfBlocks,
    int deviceIn,
    int deviceOut,
    PaStreamCallback *callback)
{
    PaError err;
    PaStreamParameters parametersIn;
    PaStreamParameters parametersOut;
    PaStreamParameters *parametersInPointer  = NULL;
    PaStreamParameters *parametersOutPointer = NULL;

    parametersIn.device                     = deviceIn;
    parametersIn.channelCount               = numberOfChannelsIn;
    parametersIn.sampleFormat               = paFloat32;
    parametersIn.suggestedLatency           = 0;
    parametersIn.hostApiSpecificStreamInfo  = 0;

    parametersOut.device                    = deviceOut;
    parametersOut.channelCount              = numberOfChannelsOut;
    parametersOut.sampleFormat              = paFloat32;
    parametersOut.suggestedLatency          = 0;
    parametersOut.hostApiSpecificStreamInfo = 0;

    if (numberOfChannelsIn > 0)  { parametersInPointer  = &parametersIn;  }
    if (numberOfChannelsOut > 0) { parametersOutPointer = &parametersOut; }

    err = Pa_IsFormatSupported (parametersInPointer, parametersOutPointer, sampleRate);

    if (err == paFormatIsSupported) {
    //
    err = Pa_OpenStream (&pa_stream, 
            parametersInPointer, 
            parametersOutPointer, 
            sampleRate, 
            blockSize,
            paNoFlag,
            callback,
            NULL);
              
    if (err == paNoError) {
        err = Pa_StartStream (pa_stream);
        if (err == paNoError) { return paNoError; }
        else {
            pa_close();
        }
    }
    //
    }
    
    pa_stream = NULL; 
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* On Mac OS Pa_Initialize() closes file descriptor 1 (standard output). */
/* As a workaround, dup it to another number and dup2 it back afterward. */
    
void pa_initialize (void)
{
    #if PD_APPLE
    
    int f = dup (1);
    int dummy = open ("/dev/null", 0);
    dup2 (dummy, 1);
    
    PaError err = Pa_Initialize();
    
    close (1);
    close (dummy);
    if (f >= 0) { fflush (stdout); dup2 (f, 1); close (f); }
    
    #else
    
    PaError err = Pa_Initialize();
    
    #endif

    if (err != paNoError) { post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error pa_open (int numberOfChannelsIn, 
    int numberOfChannelsOut,
    int sampleRate,
    t_sample *soundIn,
    t_sample *soundOut,
    int blockSize,
    int advanceInNumberOfBlocks,
    int deviceIn,
    int deviceOut,
    t_audiocallback callback)
{
    int t;
    int n;
    int i = -1;
    int o = -1;
    
    if (pa_stream) { pa_close(); PD_BUG; }

    if (numberOfChannelsIn > 0) {
    //
    for (t = 0, n = 0; t < Pa_GetDeviceCount(); t++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo (t);
        if (info->maxInputChannels > 0) {
            if (n == deviceIn) {
                numberOfChannelsIn = PD_MIN (numberOfChannelsIn, info->maxInputChannels);
                i = t;
                break;
            }
            n++;
        }
    }
    //
    }   
    
    if (numberOfChannelsOut > 0) {
    //
    for (t = 0, n = 0; t < Pa_GetDeviceCount(); t++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(t);
        if (info->maxOutputChannels > 0) {
            if (n == deviceOut) {
                numberOfChannelsOut = PD_MIN (numberOfChannelsOut, info->maxOutputChannels);
                o = t;
                break;
            }
            n++;
        }
    }
    //
    }   

    if (i == -1) { numberOfChannelsIn  = 0; }
    if (o == -1) { numberOfChannelsOut = 0; }
    
    PD_ASSERT (numberOfChannelsIn <= audio_channelsIn);
    PD_ASSERT (numberOfChannelsOut <= audio_channelsOut);
    
    pa_channelsIn   = audio_channelsIn  = numberOfChannelsIn;
    pa_channelsOut  = audio_channelsOut = numberOfChannelsOut;
    pa_soundIn      = soundIn;
    pa_soundOut     = soundOut;

    if (pa_bufferIn)  { PD_MEMORY_FREE (pa_bufferIn);  pa_bufferIn  = NULL; }
    if (pa_bufferOut) { PD_MEMORY_FREE (pa_bufferOut); pa_bufferOut = NULL; }

    if (numberOfChannelsIn || numberOfChannelsOut) {
    //
    PaError err;
    
    if (callback) {
    
        pa_callback = callback;
        
        err = pa_openWithCallback (sampleRate, 
                numberOfChannelsIn, 
                numberOfChannelsOut,
                blockSize, 
                advanceInNumberOfBlocks, 
                i, 
                o, 
                pa_lowLevelCallback);
            
    } else {
    
        if (pa_channelsIn) {
            size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsIn * sizeof (float);
            pa_bufferIn = PD_MEMORY_GET (k);
            ringbuffer_initialize (&pa_ringIn, k, pa_bufferIn, 0);
        }
        if (pa_channelsOut) {
            size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsOut * sizeof (float);
            pa_bufferOut = PD_MEMORY_GET (k);
            ringbuffer_initialize (&pa_ringOut, k, pa_bufferOut, 0);
        }
        
        err = pa_openWithCallback (sampleRate,
                numberOfChannelsIn, 
                numberOfChannelsOut,
                blockSize, 
                advanceInNumberOfBlocks,
                i,
                o,
                pa_fakeBlockingCallback);
    }
    
    pa_started = 0;
    pa_advanceInNumberOfBlocks = advanceInNumberOfBlocks;

    if (err != paNoError) { 
        post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); return PD_ERROR; 
    }
    //
    }
    
    return PD_ERROR_NONE;
}

void pa_close (void)
{
    if (pa_stream)    { Pa_AbortStream (pa_stream); Pa_CloseStream (pa_stream); pa_stream = NULL; }
    if (pa_bufferIn)  { PD_MEMORY_FREE (pa_bufferIn);  pa_bufferIn  = NULL; }
    if (pa_bufferOut) { PD_MEMORY_FREE (pa_bufferOut); pa_bufferOut = NULL; } 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int pa_pollDSP(void)
{
    t_sample *fp;
    float *fp2, *fp3;
    float *conversionbuf;
    int j, k;
    int rtnval =  DACS_YES;
#ifndef PA_WITH_FAKEBLOCKING
    double timebefore;
#endif /* PA_WITH_FAKEBLOCKING */
    if (!audio_channelsIn && !audio_channelsOut || !pa_stream)
        return (DACS_NO); 
    conversionbuf = (float *)alloca((audio_channelsIn > audio_channelsOut?
        audio_channelsIn:audio_channelsOut) * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float));

#ifdef PA_WITH_FAKEBLOCKING
    if (!audio_channelsIn)    /* if no input channels sync on output */
    {
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_lock(&pa_mutex);
#endif
        while (ringbuffer_getWriteAvailable(&pa_ringOut) <
            (long)(audio_channelsOut * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float)))
        {
            rtnval = DACS_SLEPT;
#ifdef PA_WITH_THREADSIGNAL
            pthread_cond_wait(&pa_cond, &pa_mutex);
#else
#ifdef _WIN32
            Sleep(1);
#else
            usleep(1000);
#endif /* _WIN32 */
#endif /* PA_WITH_THREADSIGNAL */
        }
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_unlock(&pa_mutex);
#endif
    }
        /* write output */
    if (audio_channelsOut)
    {
        for (j = 0, fp = audio_soundOut, fp2 = conversionbuf;
            j < audio_channelsOut; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsOut)
                        *fp3 = *fp;
        ringbuffer_write(&pa_ringOut, conversionbuf,
            audio_channelsOut*(AUDIO_DEFAULT_BLOCKSIZE*sizeof(float)), pa_bufferOut);
    }
    if (audio_channelsIn)    /* if there is input sync on it */
    {
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_lock(&pa_mutex);
#endif
        while (ringbuffer_getReadAvailable(&pa_ringIn) <
            (long)(audio_channelsIn * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float)))
        {
            rtnval = DACS_SLEPT;
#ifdef PA_WITH_THREADSIGNAL
            pthread_cond_wait(&pa_cond, &pa_mutex);
#else
#ifdef _WIN32
            Sleep(1);
#else
            usleep(1000);
#endif /* _WIN32 */
#endif /* PA_WITH_THREADSIGNAL */
        }
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_unlock(&pa_mutex);
#endif
    }
    if (audio_channelsIn)
    {
        ringbuffer_read(&pa_ringIn, conversionbuf,
            audio_channelsIn*(AUDIO_DEFAULT_BLOCKSIZE*sizeof(float)), pa_bufferIn);
        for (j = 0, fp = audio_soundIn, fp2 = conversionbuf;
            j < audio_channelsIn; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsIn)
                        *fp = *fp3;
    }

#else /* PA_WITH_FAKEBLOCKING */
    timebefore = sys_getRealTimeInSeconds();
        /* write output */
    if (audio_channelsOut)
    {
        if (!pa_started)
        {
            memset(conversionbuf, 0,
                audio_channelsOut * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float));
            for (j = 0; j < pa_advanceInNumberOfBlocks-1; j++)
                Pa_WriteStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
        }
        for (j = 0, fp = audio_soundOut, fp2 = conversionbuf;
            j < audio_channelsOut; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsOut)
                        *fp3 = *fp;
        Pa_WriteStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
    }

    if (audio_channelsIn)
    {
        Pa_ReadStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
        for (j = 0, fp = audio_soundIn, fp2 = conversionbuf;
            j < audio_channelsIn; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsIn)
                        *fp = *fp3;
    }
    if (sys_getRealTimeInSeconds() - timebefore > 0.002)
    {
        rtnval = DACS_SLEPT;
    }
#endif /* PA_WITH_FAKEBLOCKING */
    pa_started = 1;

    memset(audio_soundOut, 0, AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample)*audio_channelsOut);
    return (rtnval);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error pa_getLists (char *devicesIn,
    int *numberOfDevicesIn,
    char *devicesOut,
    int *numberOfDevicesOut,
    int *canMultiple,
    int *canCallback)
{
    int i;
    int m = 0;
    int n = 0;
    
    t_error err = PD_ERROR_NONE;
        
    *canMultiple = 1;           /* One device each for input and for output. */
    *canCallback = 1;
    
    for (i = 0; i < Pa_GetDeviceCount(); i++) {
    //
    const PaDeviceInfo *info = Pa_GetDeviceInfo (i);
        
    if (info->maxInputChannels > 0 && m < MAXIMUM_DEVICES) {
        err |= string_copy (devicesIn + (m * MAXIMUM_DESCRIPTION),  MAXIMUM_DESCRIPTION, info->name);
        m++;
    }
    
    if (info->maxOutputChannels > 0 && n < MAXIMUM_DEVICES) {
        err |= string_copy (devicesOut + (n * MAXIMUM_DESCRIPTION), MAXIMUM_DESCRIPTION, info->name);
        n++;
    }
    //
    }
    
    *numberOfDevicesIn  = m;
    *numberOfDevicesOut = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
