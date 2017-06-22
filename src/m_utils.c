
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define UTILS_BIND              "pd-"
#define UTILS_BIND_TEMPLATE     "_TEMPLATE_"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void utils_anythingToList (t_pd *x, t_listmethod fn, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    PD_ATOMS_ALLOCA (t, argc + 1);
    atom_copyAtoms (argv, argc, t + 1, argc);
    SET_SYMBOL (t, s);
    (*fn) (x, &s_anything, argc + 1, t);
    PD_ATOMS_FREEA (t, argc + 1);
}

t_symbol *symbol_withAtoms (int argc, t_atom *argv)
{
    t_symbol *s = &s_;
        
    if (argc == 1 && IS_SYMBOL (argv)) { s = GET_SYMBOL (argv); }
    else if (argc) {
        char *t = atom_atomsToString (argc, argv);
        s = gensym (t);
        PD_MEMORY_FREE (t);
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *utils_getFirstAtomOfObject (t_object *x)
{
    return utils_getFirstAtomOfBuffer (object_getBuffer (x));
}

t_symbol *utils_getFirstAtomOfBuffer (t_buffer *x)
{
    if (x != NULL) {
    //
    int argc = buffer_getSize (x);
    t_atom *argv = buffer_getAtoms (x);
    if (argc && IS_SYMBOL (argv)) { return GET_SYMBOL (argv); }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that usage of "empty" or "-" as nil tokens is a bad idea. */
/* But it must be kept to ensure compatibility with legacy files. */

t_symbol *symbol_nil (void)
{
    return sym_empty;
}

t_symbol *symbol_dash (void)
{
    return sym___dash__;
}

t_symbol *symbol_emptyAsNil (t_symbol *s)
{
    if (s == &s_) { return symbol_nil(); }
    else { 
        return s;
    }
}

t_symbol *symbol_emptyAsDash (t_symbol *s)
{
    if (s == &s_) { return symbol_dash(); }
    else { 
        return s;
    }
}

int symbol_isNil (t_symbol *s)
{
    return (s == symbol_nil());
}

int symbol_isNilOrDash (t_symbol *s)
{
    return (s == symbol_nil() || s == symbol_dash());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_dollarToHash (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '$', '#');
        return gensym (t);
    }
}

t_symbol *symbol_hashToDollar (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '#', '$');
        return gensym (t);
    }
}

/* A format to avoid slicing by the string parser. */

t_symbol *symbol_decode (t_symbol *s)
{
    if (!s) { PD_BUG; }
    else {
    //
    char *p = s->s_name;
    
    PD_ASSERT (strlen (s->s_name) < PD_STRING);
    
    if (*p != '@') { return s; }
    else {
    //
    int i;
    char t[PD_STRING] = { 0 };
    
    p++;
    
    for (i = 0; i < PD_STRING - 1; i++, p++) {
    //
    if (*p == 0)   { break; }
    if (*p == '@') {
        if (p[1] == '_')        { t[i] = ' '; p++; }
        else if (p[1] == '@')   { t[i] = '@'; p++; }
        else if (p[1] == 'c')   { t[i] = ','; p++; }
        else if (p[1] == 's')   { t[i] = ';'; p++; }
        else if (p[1] == 'd')   { t[i] = '$'; p++; }
        else {
            t[i] = *p;
        }
            
    } else { 
        t[i] = *p;
    }
    //
    }
    
    t[i] = 0;
    
    return gensym (t);
    //
    }
    //
    }
    
    return &s_;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *utils_getDefaultBindName (t_class *class, t_symbol *prefix)
{
    int i = 1;
    char t[PD_STRING] = { 0 };
    
    PD_ASSERT (prefix);
    
    while (1) {
        t_error err = string_sprintf (t, PD_STRING, "%s%d", prefix->s_name, i);
        PD_ABORT (err != PD_ERROR_NONE);
        t_symbol *name = gensym (t);
        if (!pd_getThingByClass (name, class)) { return name; }
        i++;
        PD_ABORT (i < 0);
    }
}

t_symbol *symbol_removeExtension (t_symbol *s)
{
    PD_ASSERT (s);
    
    if (s != &s_) {
    //
    char t[PD_STRING] = { 0 };
    int n = string_indexOfFirstOccurrenceFromEnd (s->s_name, ".");
    t_error err = string_copy (t, PD_STRING, s->s_name);
    PD_ASSERT (!err);
    if (!err && n >= 0) { t[n] = 0; return gensym (t); } 
    //
    }
    
    return s;
}

int utils_isNameAllowedForWindow (t_symbol *s)
{
    if (s == sym_Array)         { return 0; }
    else if (s == sym_Patch)    { return 0; }
    else if (s == sym_Scalar)   { return 0; }
    else if (s == sym_Text)     { return 0; }
    
    return 1;
}

t_unique utils_unique (void)
{
    static t_unique unique = 10000;     /* Static. */
    
    unique++;
    
    return (unique == 0 ? (++unique) : unique);        
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_makeBindSymbol (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, UTILS_BIND "%s", s->s_name);
    PD_UNUSED (err); PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *symbol_makeTemplateIdentifier (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, UTILS_BIND_TEMPLATE "%s", s->s_name);
    PD_UNUSED (err); PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *symbol_stripBindSymbol (t_symbol *s)
{
    if (string_startWith (s->s_name, UTILS_BIND)) { 
        return gensym (s->s_name + strlen (UTILS_BIND));
    }
    
    return s;
}

t_symbol *symbol_stripTemplateIdentifier (t_symbol *s)
{
    if (string_startWith (s->s_name, UTILS_BIND_TEMPLATE)) {
        return gensym (s->s_name + strlen (UTILS_BIND_TEMPLATE)); 
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error utils_version (char *dest, size_t size)
{
    t_error err = string_sprintf (dest, size, "%s %s / %s / %s", 
                    PD_NAME, 
                    PD_VERSION, 
                    midi_nameNative(), 
                    audio_nameNative());
    
    #if PD_32BIT
        err |= string_add (dest, size, " / 32-bit");
    #endif
    
    #if PD_64BIT
        err |= string_add (dest, size, " / 64-bit");
    #endif
    
    #if PD_WITH_DEBUG
        err |= string_add (dest, size, " / DEBUG");
    #endif

    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
