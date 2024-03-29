
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* LLVM analyzer seems to report false positives. */

#ifndef __clang_analyzer__

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_mica.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micaspell_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micaspell {
    t_object            x_obj;          /* Must be the first. */
    mica::MIR::Spell    x_spell;
    t_symbol            *x_key;
    t_outlet            *x_outlet;
    } t_micaspell;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micaspell_list (t_micaspell *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    PD_ASSERT (x->x_spell.isValid());
    
    prim::Array < int > notes; notes.resize (argc);
    
    for (int i = 0; i < argc; ++i) { int n = (int)atom_getFloat (argv + i); notes[i] = PD_CLAMP (n, 0, 127); }
    
    prim::Array < mica::Concept > result = x->x_spell.getSpelling (notes);
    
    if (result.size() != argc) { PD_BUG; }
    else {
    //
    t_atom *a = NULL;
    
    PD_ATOMS_ALLOCA (a, argc);
    
    for (int i = 0; i < argc; ++i) { t_symbol *t = concept_tag (result[i]); SET_SYMBOL (a + i, t); }
    
    outlet_list (x->x_outlet, argc, a);
    
    PD_ATOMS_FREEA (a, argc);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micaspell_setProceed (t_micaspell *x, t_symbol *key)
{
    if (key != &s_) {
    //
    mica::Concept c = concept_fetch (key);
    
    if (mica::MIR::Spell::keyIsValid (c)) { x->x_spell.setKey (c); x->x_key = key; }
    else {
        error_invalid (sym_mica__space__spell, sym_key);
    }
    //
    }
}

static void micaspell_set (t_micaspell *x, t_symbol *s, int argc, t_atom *argv)
{
    micaspell_setProceed (x, atom_getSymbolAtIndex (0, argc, argv));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *micaspell_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_micaspell *x = (t_micaspell *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, x->x_key);
    
    return b;
    //
    }
    
    return NULL;
}

static void micaspell_restore (t_micaspell *x, t_symbol *s)
{
    t_micaspell *old = (t_micaspell *)instance_pendingFetch (cast_gobj (x));

    micaspell_setProceed (x, old ? old->x_key : s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micaspell_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micaspell *x = (t_micaspell *)pd_new (micaspell_class);
    
    try { new (x) t_micaspell; } catch (...) { PD_BUG; }
    
    x->x_key    = &s_;
    x->x_outlet = outlet_newList (cast_object (x));
    
    if (argc) { micaspell_setProceed (x, concept_tagParsed (argc, argv)); }
    
    return x;
}

static void micaspell_free (t_micaspell *x)
{
    x->~t_micaspell();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micaspell_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__spell,
            (t_newmethod)micaspell_new,
            (t_method)micaspell_free,
            sizeof (t_micaspell),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addList (c, (t_method)micaspell_list);
    
    class_addMethod (c, (t_method)micaspell_set,     sym_set,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)micaspell_restore, sym__restore,  A_DEFSYMBOL, A_NULL);
    
    class_setDataFunction (c, micaspell_functionData);
    class_requirePending (c);
    
    class_setHelpName (c, sym_mica);
    
    micaspell_class = c;
}

void micaspell_destroy (void)
{
    class_free (micaspell_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __clang_analyzer__

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
