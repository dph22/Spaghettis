/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outmidi_noteOn (int port, int channel, int pitch, int velocity)
{
    pitch    = PD_CLAMP (pitch, 0, 127);
    velocity = PD_CLAMP (velocity, 0, 127);

    midi_broadcast (port, 0, MIDI_NOTEON + (channel & 0xf), pitch, velocity);
}

void outmidi_controlChange (int port, int channel, int control, int value)
{
    control = PD_CLAMP (control, 0, 127);
    value   = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_CONTROLCHANGE + (channel & 0xf), control, value);
}

void outmidi_programChange (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_PROGRAMCHANGE + (channel & 0xf), value, 0);
}

void outmidi_pitchBend (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 16383);     // 0x3fff 
    
    midi_broadcast (port, 0, MIDI_PITCHBEND + (channel & 0xf), (value & 127), ((value >> 7) & 127));
}

void outmidi_afterTouch (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_AFTERTOUCH + (channel & 0xf), value, 0);
}

void outmidi_polyPressure (int port, int channel, int pitch, int value)
{
    pitch = PD_CLAMP (pitch, 0, 127);
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_POLYPRESSURE + (channel & 0xf), pitch, value);
}

void outmidi_clock (int port)
{
    midi_broadcast (port, 1, MIDI_CLOCK, 0, 0);
}

/* ----------------------- midiin and sysexin ------------------------- */

static t_class *midiin_class, *sysexin_class;

typedef struct _midiin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midiin;

static void *midiin_new( void)
{
    t_midiin *x = (t_midiin *)pd_new(midiin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midiin);
    return (x);
}

static void midiin_list(t_midiin *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_float(x->x_outlet2, atom_getFloatAtIndex(1, ac, av) + 1);
    outlet_float(x->x_outlet1, atom_getFloatAtIndex(0, ac, av));
}

static void midiin_free(t_midiin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midiin);
}

static void *sysexin_new( void)
{
    t_midiin *x = (t_midiin *)pd_new(sysexin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__sysexin);
    return (x);
}

static void sysexin_free(t_midiin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__sysexin);
}

static void midiin_setup(void)
{
    midiin_class = class_new(sym_midiin, (t_newmethod)midiin_new,
        (t_method)midiin_free, sizeof(t_midiin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midiin_class, midiin_list);
    class_setHelpName(midiin_class, sym_midiout);

    sysexin_class = class_new(sym_sysexin, (t_newmethod)sysexin_new,
        (t_method)sysexin_free, sizeof(t_midiin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(sysexin_class, midiin_list);
    class_setHelpName(sysexin_class, sym_midiout);
}

void inmidi_byte(int portno, int byte)
{
    t_atom at[2];
    if (sym__midiin->s_thing)
    {
        SET_FLOAT(at, byte);
        SET_FLOAT(at+1, portno);
        pd_list(sym__midiin->s_thing, 2, at);
    }
}

void inmidi_sysex(int portno, int byte)
{
    t_atom at[2];
    if (sym__sysexin->s_thing)
    {
        SET_FLOAT(at, byte);
        SET_FLOAT(at+1, portno);
        pd_list(sym__sysexin->s_thing, 2, at);
    }
}

/* ----------------------- notein ------------------------- */

static t_class *notein_class;

typedef struct _notein
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_notein;

static void *notein_new(t_float f)
{
    t_notein *x = (t_notein *)pd_new(notein_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__notein);
    return (x);
}

static void notein_list(t_notein *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float pitch = atom_getFloatAtIndex(0, argc, argv);
    t_float velo = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet2, velo);
        outlet_float(x->x_outlet1, pitch);
    }
    else
    {
        outlet_float(x->x_outlet3, channel);
        outlet_float(x->x_outlet2, velo);
        outlet_float(x->x_outlet1, pitch);
    }
}

static void notein_free(t_notein *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__notein);
}

static void notein_setup(void)
{
    notein_class = class_new(sym_notein, (t_newmethod)notein_new,
        (t_method)notein_free, sizeof(t_notein), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(notein_class, notein_list);
    class_setHelpName(notein_class, sym_midiout);
}

void inmidi_noteOn(int portno, int channel, int pitch, int velo)
{
    if (sym__notein->s_thing)
    {
        t_atom at[3];
        SET_FLOAT(at, pitch);
        SET_FLOAT(at+1, velo);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(sym__notein->s_thing, 3, at);
    }
}

/* ----------------------- ctlin ------------------------- */

static t_class *ctlin_class;

typedef struct _ctlin
{
    t_object x_obj;
    t_float x_channel;
    t_float x_ctlno;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_ctlin;

static void *ctlin_new(t_symbol *s, int argc, t_atom *argv)
{
    int ctlno, channel;
    t_ctlin *x = (t_ctlin *)pd_new(ctlin_class);
    if (!argc) ctlno = -1;
    else ctlno = atom_getFloatAtIndex(0, argc, argv);
    channel = atom_getFloatAtIndex(1, argc, argv);
    x->x_channel = channel;
    x->x_ctlno = ctlno;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (!channel)
    {
        if (x->x_ctlno < 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
        x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    }
    pd_bind(&x->x_obj.te_g.g_pd, sym__ctlin);
    return (x);
}

static void ctlin_list(t_ctlin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float ctlnumber = atom_getFloatAtIndex(0, argc, argv);
    t_float value = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_ctlno >= 0 && x->x_ctlno != ctlnumber) return;
    if (x->x_channel > 0  && x->x_channel != channel) return;
    if (x->x_channel == 0) outlet_float(x->x_outlet3, channel);
    if (x->x_ctlno < 0) outlet_float(x->x_outlet2, ctlnumber);
    outlet_float(x->x_outlet1, value);
}

static void ctlin_free(t_ctlin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__ctlin);
}

static void ctlin_setup(void)
{
    ctlin_class = class_new(sym_ctlin, (t_newmethod)ctlin_new, 
        (t_method)ctlin_free, sizeof(t_ctlin),
            CLASS_NOINLET, A_GIMME, 0);
    class_addList(ctlin_class, ctlin_list);
    class_setHelpName(ctlin_class, sym_midiout);
}

void inmidi_controlChange(int portno, int channel, int ctlnumber, int value)
{
    if (sym__ctlin->s_thing)
    {
        t_atom at[3];
        SET_FLOAT(at, ctlnumber);
        SET_FLOAT(at+1, value);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(sym__ctlin->s_thing, 3, at);
    }
}

/* ----------------------- pgmin ------------------------- */

static t_class *pgmin_class;

typedef struct _pgmin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_pgmin;

static void *pgmin_new(t_float f)
{
    t_pgmin *x = (t_pgmin *)pd_new(pgmin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__pgmin);
    return (x);
}

static void pgmin_list(t_pgmin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet2, channel);
        outlet_float(x->x_outlet1, value);
    }
}

static void pgmin_free(t_pgmin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__pgmin);
}

static void pgmin_setup(void)
{
    pgmin_class = class_new(sym_pgmin, (t_newmethod)pgmin_new,
        (t_method)pgmin_free, sizeof(t_pgmin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(pgmin_class, pgmin_list);
    class_setHelpName(pgmin_class, sym_midiout);
}

void inmidi_programChange(int portno, int channel, int value)
{
    if (sym__pgmin->s_thing)
    {
        t_atom at[2];
        SET_FLOAT(at, value + 1);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(sym__pgmin->s_thing, 2, at);
    }
}

/* ----------------------- bendin ------------------------- */

static t_class *bendin_class;

typedef struct _bendin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_bendin;

static void *bendin_new(t_float f)
{
    t_bendin *x = (t_bendin *)pd_new(bendin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__bendin);
    return (x);
}

static void bendin_list(t_bendin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet2, channel);
        outlet_float(x->x_outlet1, value);
    }
}

static void bendin_free(t_bendin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__bendin);
}

static void bendin_setup(void)
{
    bendin_class = class_new(sym_bendin, (t_newmethod)bendin_new,
        (t_method)bendin_free, sizeof(t_bendin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(bendin_class, bendin_list);
    class_setHelpName(bendin_class, sym_midiout);
}

void inmidi_pitchBend(int portno, int channel, int value)
{
    if (sym__bendin->s_thing)
    {
        t_atom at[2];
        SET_FLOAT(at, value);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(sym__bendin->s_thing, 2, at);
    }
}

/* ----------------------- touchin ------------------------- */

static t_class *touchin_class;

typedef struct _touchin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_touchin;

static void *touchin_new(t_float f)
{
    t_touchin *x = (t_touchin *)pd_new(touchin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__touchin);
    return (x);
}

static void touchin_list(t_touchin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet2, channel);
        outlet_float(x->x_outlet1, value);
    }
}

static void touchin_free(t_touchin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__touchin);
}

static void touchin_setup(void)
{
    touchin_class = class_new(sym_touchin, (t_newmethod)touchin_new,
        (t_method)touchin_free, sizeof(t_touchin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(touchin_class, touchin_list);
    class_setHelpName(touchin_class, sym_midiout);
}

void inmidi_afterTouch(int portno, int channel, int value)
{
    if (sym__touchin->s_thing)
    {
        t_atom at[2];
        SET_FLOAT(at, value);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(sym__touchin->s_thing, 2, at);
    }
}

/* ----------------------- polytouchin ------------------------- */

static t_class *polytouchin_class;

typedef struct _polytouchin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_polytouchin;

static void *polytouchin_new(t_float f)
{
    t_polytouchin *x = (t_polytouchin *)pd_new(polytouchin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__polytouchin);
    return (x);
}

static void polytouchin_list(t_polytouchin *x, t_symbol *s, int argc,
    t_atom *argv)
{
    t_float pitch = atom_getFloatAtIndex(0, argc, argv);
    t_float value = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet2, pitch);
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet3, channel);
        outlet_float(x->x_outlet2, pitch);
        outlet_float(x->x_outlet1, value);
    }
}

static void polytouchin_free(t_polytouchin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__polytouchin);
}

static void polytouchin_setup(void)
{
    polytouchin_class = class_new(sym_polytouchin,
        (t_newmethod)polytouchin_new, (t_method)polytouchin_free,
        sizeof(t_polytouchin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(polytouchin_class, polytouchin_list);
    class_setHelpName(polytouchin_class, sym_midiout);
}

void inmidi_polyPressure(int portno, int channel, int pitch, int value)
{
    if (sym__polytouchin->s_thing)
    {
        t_atom at[3];
        SET_FLOAT(at, pitch);
        SET_FLOAT(at+1, value);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(sym__polytouchin->s_thing, 3, at);
    }
}

/*----------------------- midiclkin--(midi F8 message )---------------------*/

static t_class *midiclkin_class;

typedef struct _midiclkin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midiclkin;

static void *midiclkin_new(t_float f)
{
    t_midiclkin *x = (t_midiclkin *)pd_new(midiclkin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midiclkin);
    return (x);
}

static void midiclkin_list(t_midiclkin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float count = atom_getFloatAtIndex(1, argc, argv);
    outlet_float(x->x_outlet2, count);
    outlet_float(x->x_outlet1, value);
}

static void midiclkin_free(t_midiclkin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midiclkin);
}

static void midiclkin_setup(void)
{
    midiclkin_class = class_new(sym_midiclkin, 
        (t_newmethod)midiclkin_new, (t_method)midiclkin_free, 
            sizeof(t_midiclkin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midiclkin_class, midiclkin_list);
        class_setHelpName(midiclkin_class, sym_midiout);
}

void inmidi_clk(double timing)
{

    static t_float prev = 0;
    static t_float count = 0;
    t_float cur,diff;

    if (sym__midiclkin->s_thing)
    {
        t_atom at[2];
        diff =timing - prev;
        count++;
   
        if (count == 3)
        {  /* 24 count per quoter note */
             SET_FLOAT(at, 1 );
             count = 0;
        }
        else SET_FLOAT(at, 0);

        SET_FLOAT(at+1, diff);
        pd_list(sym__midiclkin->s_thing, 2, at);
        prev = timing;
    }
}

/*----------midirealtimein (midi FA,FB,FC,FF message )-----------------*/

static t_class *midirealtimein_class;

typedef struct _midirealtimein
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midirealtimein;

static void *midirealtimein_new( void)
{
    t_midirealtimein *x = (t_midirealtimein *)pd_new(midirealtimein_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midirealtimein);
    return (x);
}

static void midirealtimein_list(t_midirealtimein *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_float portno = atom_getFloatAtIndex(0, argc, argv);
    t_float byte = atom_getFloatAtIndex(1, argc, argv);

    outlet_float(x->x_outlet2, portno);
    outlet_float(x->x_outlet1, byte);
}

static void midirealtimein_free(t_midirealtimein *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midirealtimein);
}

static void midirealtimein_setup(void)
{
    midirealtimein_class = class_new(sym_midirealtimein, 
        (t_newmethod)midirealtimein_new, (t_method)midirealtimein_free, 
            sizeof(t_midirealtimein), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midirealtimein_class, midirealtimein_list);
        class_setHelpName(midirealtimein_class, sym_midiout);
}

void inmidi_realTimeIn(int portno, int SysMsg)
{
    if (sym__midirealtimein->s_thing)
    {
        t_atom at[2];
        SET_FLOAT(at, portno);
        SET_FLOAT(at+1, SysMsg);
        pd_list(sym__midirealtimein->s_thing, 2, at);
    }
}

/* -------------------------- midiout -------------------------- */

static t_class *midiout_class;

typedef struct _midiout
{
    t_object x_obj;
    t_float x_portno;
} t_midiout;

static void *midiout_new(t_float portno)
{
    t_midiout *x = (t_midiout *)pd_new(midiout_class);
    if (portno <= 0) portno = 1;
    x->x_portno = portno;
    inlet_newFloat(&x->x_obj, &x->x_portno);
    return (x);
}

static void midiout_float(t_midiout *x, t_float f)
{
    midi_broadcast (x->x_portno - 1, 1, f, 0, 0);
}

static void midiout_setup(void)
{
    midiout_class = class_new(sym_midiout, (t_newmethod)midiout_new, 0,
        sizeof(t_midiout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(midiout_class, midiout_float);
    class_setHelpName(midiout_class, sym_midiout);
}

/* -------------------------- noteout -------------------------- */

static t_class *noteout_class;

typedef struct _noteout
{
    t_object x_obj;
    t_float x_velo;
    t_float x_channel;
} t_noteout;

static void *noteout_new(t_float channel)
{
    t_noteout *x = (t_noteout *)pd_new(noteout_class);
    x->x_velo = 0;
    if (channel < 1) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_velo);
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void noteout_float(t_noteout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_noteOn((binchan >> 4),
        (binchan & 15), (int)f, (int)x->x_velo);
}

static void noteout_setup(void)
{
    noteout_class = class_new(sym_noteout, (t_newmethod)noteout_new, 0,
        sizeof(t_noteout), 0, A_DEFFLOAT, 0);
    class_addFloat(noteout_class, noteout_float);
    class_setHelpName(noteout_class, sym_midiout);
}


/* -------------------------- ctlout -------------------------- */

static t_class *ctlout_class;

typedef struct _ctlout
{
    t_object x_obj;
    t_float x_ctl;
    t_float x_channel;
} t_ctlout;

static void *ctlout_new(t_float ctl, t_float channel)
{
    t_ctlout *x = (t_ctlout *)pd_new(ctlout_class);
    x->x_ctl = ctl;
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_ctl);
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void ctlout_float(t_ctlout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_controlChange((binchan >> 4),
        (binchan & 15), (int)(x->x_ctl), (int)f);
}

static void ctlout_setup(void)
{
    ctlout_class = class_new(sym_ctlout, (t_newmethod)ctlout_new, 0,
        sizeof(t_ctlout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(ctlout_class, ctlout_float);
    class_setHelpName(ctlout_class, sym_midiout);
}


/* -------------------------- pgmout -------------------------- */

static t_class *pgmout_class;

typedef struct _pgmout
{
    t_object x_obj;
    t_float x_channel;
} t_pgmout;

static void *pgmout_new(t_float channel)
{
    t_pgmout *x = (t_pgmout *)pd_new(pgmout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void pgmout_float(t_pgmout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    int n = f - 1;
    if (binchan < 0)
        binchan = 0;
    if (n < 0) n = 0;
    else if (n > 127) n = 127;
    outmidi_programChange((binchan >> 4),
        (binchan & 15), n);
}

static void pgmout_setup(void)
{
    pgmout_class = class_new(sym_pgmout, (t_newmethod)pgmout_new, 0,
        sizeof(t_pgmout), 0, A_DEFFLOAT, 0);
    class_addFloat(pgmout_class, pgmout_float);
    class_setHelpName(pgmout_class, sym_midiout);
}


/* -------------------------- bendout -------------------------- */

static t_class *bendout_class;

typedef struct _bendout
{
    t_object x_obj;
    t_float x_channel;
} t_bendout;

static void *bendout_new(t_float channel)
{
    t_bendout *x = (t_bendout *)pd_new(bendout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void bendout_float(t_bendout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    int n = (int)f +  8192;
    if (binchan < 0)
        binchan = 0;
    outmidi_pitchBend((binchan >> 4), (binchan & 15), n);
}

static void bendout_setup(void)
{
    bendout_class = class_new(sym_bendout, (t_newmethod)bendout_new, 0,
        sizeof(t_bendout), 0, A_DEFFLOAT, 0);
    class_addFloat(bendout_class, bendout_float);
    class_setHelpName(bendout_class, sym_midiout);
}

/* -------------------------- touch -------------------------- */

static t_class *touchout_class;

typedef struct _touchout
{
    t_object x_obj;
    t_float x_channel;
} t_touchout;

static void *touchout_new(t_float channel)
{
    t_touchout *x = (t_touchout *)pd_new(touchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void touchout_float(t_touchout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_afterTouch((binchan >> 4), (binchan & 15), (int)f);
}

static void touchout_setup(void)
{
    touchout_class = class_new(sym_touchout, (t_newmethod)touchout_new, 0,
        sizeof(t_touchout), 0, A_DEFFLOAT, 0);
    class_addFloat(touchout_class, touchout_float);
    class_setHelpName(touchout_class, sym_midiout);
}

/* -------------------------- polytouch -------------------------- */

static t_class *polytouchout_class;

typedef struct _polytouchout
{
    t_object x_obj;
    t_float x_channel;
    t_float x_pitch;
} t_polytouchout;

static void *polytouchout_new(t_float channel)
{
    t_polytouchout *x = (t_polytouchout *)pd_new(polytouchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    x->x_pitch = 0;
    inlet_newFloat(&x->x_obj, &x->x_pitch);
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void polytouchout_float(t_polytouchout *x, t_float n)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_polyPressure((binchan >> 4), (binchan & 15), x->x_pitch, n);
}

static void polytouchout_setup(void)
{
    polytouchout_class = class_new(sym_polytouchout, 
        (t_newmethod)polytouchout_new, 0,
        sizeof(t_polytouchout), 0, A_DEFFLOAT, 0);
    class_addFloat(polytouchout_class, polytouchout_float);
    class_setHelpName(polytouchout_class, sym_midiout);
}

/* -------------------------- makenote -------------------------- */

static t_class *makenote_class;

typedef struct _hang
{
    t_clock *h_clock;
    struct _hang *h_next;
    t_float h_pitch;
    struct _makenote *h_owner;
} t_hang;

typedef struct _makenote
{
    t_object x_obj;
    t_float x_velo;
    t_float x_dur;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
    t_hang *x_hang;
} t_makenote;

static void *makenote_new(t_float velo, t_float dur)
{
    t_makenote *x = (t_makenote *)pd_new(makenote_class);
    x->x_velo = velo;
    x->x_dur = dur;
    inlet_newFloat(&x->x_obj, &x->x_velo);
    inlet_newFloat(&x->x_obj, &x->x_dur);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    x->x_hang = 0;
    return (x);
}

static void makenote_tick(t_hang *hang)
{
    t_makenote *x = hang->h_owner;
    t_hang *h2, *h3;
    outlet_float(x->x_velout, 0);
    outlet_float(x->x_pitchout, hang->h_pitch);
    if (x->x_hang == hang) x->x_hang = hang->h_next;
    else for (h2 = x->x_hang; h3 = h2->h_next; h2 = h3)
    {
        if (h3 == hang)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    clock_free(hang->h_clock);
    PD_MEMORY_FREE(hang);
}

static void makenote_float(t_makenote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
    hang = (t_hang *)PD_MEMORY_GET(sizeof *hang);
    hang->h_next = x->x_hang;
    x->x_hang = hang;
    hang->h_pitch = f;
    hang->h_owner = x;
    hang->h_clock = clock_new(hang, (t_method)makenote_tick);
    clock_delay(hang->h_clock, (x->x_dur >= 0 ? x->x_dur : 0));
}

static void makenote_stop(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        outlet_float(x->x_velout, 0);
        outlet_float(x->x_pitchout, hang->h_pitch);
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

static void makenote_clear(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

static void makenote_setup(void)
{
    makenote_class = class_new(sym_makenote, 
        (t_newmethod)makenote_new, (t_method)makenote_clear,
        sizeof(t_makenote), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(makenote_class, makenote_float);
    class_addMethod(makenote_class, (t_method)makenote_stop, sym_stop,
        0);
    class_addMethod(makenote_class, (t_method)makenote_clear, sym_clear,
        0);
}

/* -------------------------- stripnote -------------------------- */

static t_class *stripnote_class;

typedef struct _stripnote
{
    t_object x_obj;
    t_float x_velo;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
} t_stripnote;

static void *stripnote_new(void )
{
    t_stripnote *x = (t_stripnote *)pd_new(stripnote_class);
    inlet_newFloat(&x->x_obj, &x->x_velo);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    return (x);
}
    
static void stripnote_float(t_stripnote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
}

static void stripnote_setup(void)
{
    stripnote_class = class_new(sym_stripnote,
        (t_newmethod)stripnote_new, 0, sizeof(t_stripnote), 0, 0);
    class_addFloat(stripnote_class, stripnote_float);
}

/* -------------------------- poly -------------------------- */

static t_class *poly_class;

typedef struct voice
{
    t_float v_pitch;
    int v_used;
    unsigned long v_serial;
} t_voice;

typedef struct poly
{
    t_object x_obj;
    int x_n;
    t_voice *x_vec;
    t_float x_vel;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
    unsigned long x_serial;
    int x_steal;
} t_poly;

static void *poly_new(t_float fnvoice, t_float fsteal)
{
    int i, n = fnvoice;
    t_poly *x = (t_poly *)pd_new(poly_class);
    t_voice *v;
    if (n < 1) n = 1;
    x->x_n = n;
    x->x_vec = (t_voice *)PD_MEMORY_GET(n * sizeof(*x->x_vec));
    for (v = x->x_vec, i = n; i--; v++)
        v->v_pitch = v->v_used = v->v_serial = 0;
    x->x_vel = 0;
    x->x_steal = (fsteal != 0);
    inlet_newFloat(&x->x_obj, &x->x_vel);
    outlet_new(&x->x_obj, &s_float);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    x->x_serial = 0;
    return (x);
}

static void poly_float(t_poly *x, t_float f)
{
    int i;
    t_voice *v;
    t_voice *firston, *firstoff;
    unsigned int serialon, serialoff, onindex = 0, offindex = 0;
    if (x->x_vel > 0)
    {
            /* note on.  Look for a vacant voice */
        for (v = x->x_vec, i = 0, firston = firstoff = 0,
            serialon = serialoff = 0xffffffff; i < x->x_n; v++, i++)
        {
            if (v->v_used && v->v_serial < serialon)
                    firston = v, serialon = v->v_serial, onindex = i;
            else if (!v->v_used && v->v_serial < serialoff)
                    firstoff = v, serialoff = v->v_serial, offindex = i;
        }
        if (firstoff)
        {
            outlet_float(x->x_velout, x->x_vel);
            outlet_float(x->x_pitchout, firstoff->v_pitch = f);
            outlet_float(x->x_obj.te_outlet, offindex+1);
            firstoff->v_used = 1;
            firstoff->v_serial = x->x_serial++;
        }
            /* if none, steal one */
        else if (firston && x->x_steal)
        {
            outlet_float(x->x_velout, 0);
            outlet_float(x->x_pitchout, firston->v_pitch);
            outlet_float(x->x_obj.te_outlet, onindex+1);
            outlet_float(x->x_velout, x->x_vel);
            outlet_float(x->x_pitchout, firston->v_pitch = f);
            outlet_float(x->x_obj.te_outlet, onindex+1);
            firston->v_serial = x->x_serial++;
        }
    }
    else    /* note off. Turn off oldest match */
    {
        for (v = x->x_vec, i = 0, firston = 0, serialon = 0xffffffff;
            i < x->x_n; v++, i++)
                if (v->v_used && v->v_pitch == f && v->v_serial < serialon)
                    firston = v, serialon = v->v_serial, onindex = i;
        if (firston)
        {
            firston->v_used = 0;
            firston->v_serial = x->x_serial++;
            outlet_float(x->x_velout, 0);
            outlet_float(x->x_pitchout, firston->v_pitch);
            outlet_float(x->x_obj.te_outlet, onindex+1);
        }
    }
}

static void poly_stop(t_poly *x)
{
    int i;
    t_voice *v;
    for (i = 0, v = x->x_vec; i < x->x_n; i++, v++)
        if (v->v_used)
    {
        outlet_float(x->x_velout, 0L);
        outlet_float(x->x_pitchout, v->v_pitch);
        outlet_float(x->x_obj.te_outlet, i+1);
        v->v_used = 0;
        v->v_serial = x->x_serial++;
    }
}

static void poly_clear(t_poly *x)
{
    int i;
    t_voice *v;
    for (v = x->x_vec, i = x->x_n; i--; v++) v->v_used = v->v_serial = 0;
}

static void poly_free(t_poly *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

static void poly_setup(void)
{
    poly_class = class_new(sym_poly, 
        (t_newmethod)poly_new, (t_method)poly_free,
        sizeof(t_poly), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(poly_class, poly_float);
    class_addMethod(poly_class, (t_method)poly_stop, sym_stop, 0);
    class_addMethod(poly_class, (t_method)poly_clear, sym_clear, 0);
}

/* -------------------------- bag -------------------------- */

static t_class *bag_class;

typedef struct _bagelem
{
    struct _bagelem *e_next;
    t_float e_value;
} t_bagelem;

typedef struct _bag
{
    t_object x_obj;
    t_float x_velo;
    t_bagelem *x_first;
} t_bag;

static void *bag_new(void )
{
    t_bag *x = (t_bag *)pd_new(bag_class);
    x->x_velo = 0;
    inlet_newFloat(&x->x_obj, &x->x_velo);
    outlet_new(&x->x_obj, &s_float);
    x->x_first = 0;
    return (x);
}

static void bag_float(t_bag *x, t_float f)
{
    t_bagelem *bagelem, *e2, *e3;
    if (x->x_velo != 0)
    {
        bagelem = (t_bagelem *)PD_MEMORY_GET(sizeof *bagelem);
        bagelem->e_next = 0;
        bagelem->e_value = f;
        if (!x->x_first) x->x_first = bagelem;
        else    /* LATER replace with a faster algorithm */
        {
            for (e2 = x->x_first; e3 = e2->e_next; e2 = e3)
                ;
            e2->e_next = bagelem;
        }
    }
    else
    {
        if (!x->x_first) return;
        if (x->x_first->e_value == f)
        {
            bagelem = x->x_first;
            x->x_first = x->x_first->e_next;
            PD_MEMORY_FREE(bagelem);
            return;
        }
        for (e2 = x->x_first; e3 = e2->e_next; e2 = e3)
            if (e3->e_value == f)
        {
            e2->e_next = e3->e_next;
            PD_MEMORY_FREE(e3);
            return;
        }
    }
}

static void bag_flush(t_bag *x)
{
    t_bagelem *bagelem;
    while (bagelem = x->x_first)
    {
        outlet_float(x->x_obj.te_outlet, bagelem->e_value);
        x->x_first = bagelem->e_next;
        PD_MEMORY_FREE(bagelem);
    }
}

static void bag_clear(t_bag *x)
{
    t_bagelem *bagelem;
    while (bagelem = x->x_first)
    {
        x->x_first = bagelem->e_next;
        PD_MEMORY_FREE(bagelem);
    }
}

static void bag_setup(void)
{
    bag_class = class_new(sym_bag, 
        (t_newmethod)bag_new, (t_method)bag_clear,
        sizeof(t_bag), 0, 0);
    class_addFloat(bag_class, bag_float);
    class_addMethod(bag_class, (t_method)bag_flush, sym_flush, 0);
    class_addMethod(bag_class, (t_method)bag_clear, sym_clear, 0);
}

void sys_setmidiprefix(const char *prefix)
{
}

void x_midi_setup(void)
{
    midiin_setup();
    midirealtimein_setup();
    notein_setup();
    ctlin_setup();
    pgmin_setup();
    bendin_setup();
    touchin_setup();
    polytouchin_setup();
    midiclkin_setup();
    midiout_setup();
    noteout_setup();
    ctlout_setup();
    pgmout_setup();
    bendout_setup();
    touchout_setup();
    polytouchout_setup();
    makenote_setup();
    stripnote_setup();
    poly_setup();
    bag_setup();
}
