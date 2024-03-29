
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

void preferences_load (void)
{
    t_devices midi;    
    t_devices audio;   

    devices_initAsMidi (&midi);
    devices_initAsAudio (&audio);
    
    if (properties_loadBegin() == PD_ERROR_NONE) {
    //
    int i, t;

    char k[PD_STRING] = { 0 };
    char v[PD_STRING] = { 0 };
    
    /* Audio settings. */
    
    if (properties_getKey ("SampleRate", v, PD_STRING)) { 
        if (sscanf (v, "%d", &t) == 1) { devices_setSampleRate (&audio, t); }
    }
    
    #if PD_APPLE
    
    if (properties_getKey ("VectorSize", v, PD_STRING)) {
        if (sscanf (v, "%d", &t) == 1) { devices_setVectorSize (&audio, t); }
    }
    
    #endif
    
    /* GUI settings. */
    
    if (properties_getKey ("SnapToGrid", v, PD_STRING)) {
        if (sscanf (v, "%d", &t) == 1) { snap_setSnapToGrid ((t != 0)); }
    }
    
    if (properties_getKey ("GridSize", v, PD_STRING)) {
        if (sscanf (v, "%d", &t) == 1) { snap_setStep (t); }
    }
    
    if (properties_getKey ("FontDefaultSize", v, PD_STRING)) {
        if (sscanf (v, "%d", &t) == 1) { font_setDefaultSize (t); }
    }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {

        string_sprintf (k, PD_STRING, "Path%d", i + 1);
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else {
            searchpath_appendRoot (v);
        }
    }
    
    /* Recent files. */
    
    for (i = 0; 1; i++) {

        string_sprintf (k, PD_STRING, "RecentFile%d", i + 1);
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else {
            recentfiles_appendPath (v);
        }
    }
    
    /* Audio devices. */
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        int channels;
        
        string_sprintf (k, PD_STRING, "AudioInDeviceChannels%d", i + 1);
        if (properties_getKey (k, v, PD_STRING)) {
            if (sscanf (v, "%d", &channels) == 1) {    
                string_sprintf (k, PD_STRING, "AudioInDeviceName%d", i + 1);
                if (properties_getKey (k, v, PD_STRING)) {
                    devices_appendAudioInWithString (&audio, v, channels);
                }
            }
        }
    }
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        int channels;
        
        string_sprintf (k, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
        if (properties_getKey (k, v, PD_STRING)) {
            if (sscanf (v, "%d", &channels) == 1) {    
                string_sprintf (k, PD_STRING, "AudioOutDeviceName%d", i + 1);
                if (properties_getKey (k, v, PD_STRING)) {
                    devices_appendAudioOutWithString (&audio, v, channels);
                }
            }
        }
    }
        
    /* MIDI devices. */
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        string_sprintf (k, PD_STRING, "MidiInDeviceName%d", i + 1);
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else {
            devices_appendMidiInWithString (&midi, v);
        }
    }

    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        string_sprintf (k, PD_STRING, "MidiOutDeviceName%d", i + 1);
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else { 
            devices_appendMidiOutWithString (&midi, v);
        }
    }
    //
    }
    
    devices_setDefaultsIfNone (&midi);
    devices_setDefaultsIfNone (&audio);
    
    midi_setDevices (&midi, 1);
    audio_setDevices (&audio, 1);
    
    properties_loadClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void preferences_save (void)
{
    t_devices midi;    
    t_devices audio;   

    devices_initAsMidi (&midi);
    devices_initAsAudio (&audio);
    
    if (properties_saveBegin() == PD_ERROR_NONE) {
    //
    int i;
    t_pathlist *l = NULL;
    
    char k[PD_STRING] = { 0 };
    char v[PD_STRING] = { 0 };
    
    midi_getDevices (&midi);
    audio_getDevices (&audio);
    
    /* Audio settings. */
    
    string_sprintf (v, PD_STRING, "%d", devices_getSampleRate (&audio));
    properties_setKey ("SampleRate", v);
    
    #if PD_APPLE
    
    string_sprintf (v, PD_STRING, "%d", devices_getVectorSize (&audio));
    properties_setKey ("VectorSize", v);
    
    #endif
    
    /* GUI settings. */
    
    string_sprintf (v, PD_STRING, "%d", snap_hasSnapToGrid());
    properties_setKey ("SnapToGrid", v);
    
    string_sprintf (v, PD_STRING, "%d", snap_getStep());
    properties_setKey ("GridSize", v);
    
    string_sprintf (v, PD_STRING, "%d", font_getDefaultSize());
    properties_setKey ("FontDefaultSize", v);
    
    /* Search paths. */
    
    l = searchpath_getRoots();
    
    for (i = 0; 1; i++) {

        if (!pathlist_getPath (l)) { break; }
        else {
            string_sprintf (k, PD_STRING, "Path%d", i + 1);
            properties_setKey (k, pathlist_getPath (l));
        }
        
        l = pathlist_getNext (l);
    }
    
    /* Recent files. */
    
    l = recentfiles_get();
    
    for (i = 0; 1; i++) {

        if (!pathlist_getPath (l)) { break; }
        else {
            string_sprintf (k, PD_STRING, "RecentFile%d", i + 1);
            properties_setKey (k, pathlist_getPath (l));
        }
        
        l = pathlist_getNext (l);
    }
    
    /* Audio devices. */
    
    for (i = 0; i < devices_getInSize (&audio); i++) {

        string_sprintf (k, PD_STRING, "AudioInDeviceName%d", i + 1);
        if (devices_getInAtIndexAsString (&audio, i, v, PD_STRING)) { break; }
        else {
            properties_setKey (k, v);
            string_sprintf (k, PD_STRING, "AudioInDeviceChannels%d", i + 1);
            string_sprintf (v, PD_STRING, "%d", devices_getInChannelsAtIndex (&audio, i));
            properties_setKey (k, v);
        }
    }

    for (i = 0; i < devices_getOutSize (&audio); i++) {

        string_sprintf (k, PD_STRING, "AudioOutDeviceName%d", i + 1);
        if (devices_getOutAtIndexAsString (&audio, i, v, PD_STRING)) { break; }
        else {
            properties_setKey (k, v);
            string_sprintf (k, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
            string_sprintf (v, PD_STRING, "%d", devices_getOutChannelsAtIndex (&audio, i));
            properties_setKey (k, v);
        }
    }

    /* MIDI devices. */
    
    for (i = 0; i < devices_getInSize (&midi); i++) {

        string_sprintf (k, PD_STRING, "MidiInDeviceName%d", i + 1);
        devices_getInAtIndexAsString (&midi, i, v, PD_STRING);
        properties_setKey (k, v);
    }

    for (i = 0; i < devices_getOutSize (&midi); i++) {

        string_sprintf (k, PD_STRING, "MidiOutDeviceName%d", i + 1);
        devices_getOutAtIndexAsString (&midi, i, v, PD_STRING);
        properties_setKey (k, v);
    }
    //
    }
    
    properties_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
