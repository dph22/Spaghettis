
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directorySupport;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SEARCHPATH_LEVELS   10
#define SEARCHPATH_FDOPEN   15
#define SEARCHPATH_MAXIMUM  256

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_pathlist *searchpath_roots;            /* Static. */
static t_pathlist *searchpath_extended;         /* Static. */
static t_pathlist *searchpath_external;         /* Static. */
static t_pathlist *searchpath_patch;            /* Static. */
static t_pathlist *searchpath_help;             /* Static. */
static t_pathlist *searchpath_duplicates;       /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int searchpath_countDirectories;         /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int searchpath_scanProceed (const char *path, const struct stat *b, int flag, struct FTW *f)
{
    int abort = 0;
    
    if (flag == FTW_D) {
    //
    if (path_isValid (path)) {
    //
    abort |= (searchpath_countDirectories >= SEARCHPATH_MAXIMUM);
    abort |= (f->level > SEARCHPATH_LEVELS);
    
    if (!abort) {
        searchpath_countDirectories++;
        searchpath_extended = pathlist_newAppend (searchpath_extended, NULL, path);
    }
    //
    }
    //
    }
    
    if (flag == FTW_F) {
    //
    if (path_isValid (path)) {
    //
    const char *filename = path + f->base;
    
    if (string_endWith (path, PD_PLUGIN)) {
        searchpath_external = pathlist_newAppend (searchpath_external, &searchpath_duplicates, filename);
        
    } else if (string_endWith (path, PD_PATCH)) {
        searchpath_patch = pathlist_newAppend (searchpath_patch,       &searchpath_duplicates, filename);
        
    } else if (string_endWith (path, PD_HELP)) {
        searchpath_help = pathlist_newAppend (searchpath_help,         &searchpath_duplicates, filename);
    }
    //
    }
    //
    }
    
    if (abort) { return 1; } else { return scheduler_isExiting(); }
}

t_error searchpath_scan (void)
{
    t_error err = PD_ERROR_NONE;
    t_pathlist *l = searchpath_roots;
    
    pathlist_free (searchpath_extended);   searchpath_extended   = NULL;
    pathlist_free (searchpath_external);   searchpath_external   = NULL;
    pathlist_free (searchpath_patch);      searchpath_patch      = NULL;
    pathlist_free (searchpath_help);       searchpath_help       = NULL;
    pathlist_free (searchpath_duplicates); searchpath_duplicates = NULL;
    searchpath_countDirectories = 0;
    
    while (!err && l) {
        char *path = pathlist_getPath (l); l = pathlist_getNext (l);
        err |= (nftw (path, searchpath_scanProceed, SEARCHPATH_FDOPEN, FTW_MOUNT | FTW_PHYS) != 0);
    }
    
    PD_ASSERT (!pathlist_check (searchpath_extended));
    PD_ASSERT (!pathlist_check (searchpath_external));
    PD_ASSERT (!pathlist_check (searchpath_patch));
    PD_ASSERT (!pathlist_check (searchpath_help));
    PD_ASSERT (!pathlist_check (searchpath_duplicates));
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void searchpath_reportWrite (int f, const char *prefix, const char *s)
{
    ssize_t w = 0;      /* Avoid unused return warning. */
    
    w += write (f, prefix, strlen (prefix));
    w += write (f, " ", 1);
    w += write (f, s, strlen (s));
    w += write (f, "\n", 1);

    (void)w;
}

void searchpath_report (void)
{
    char t[PD_STRING] = { 0 };
    t_error err = string_sprintf (t, PD_STRING, "%s/scan-XXXXXX", main_directorySupport->s_name);
    int f;
    
    if (!err && ((f = mkstemp (t)) != -1)) {
    //
    t_pathlist *l = searchpath_extended;

    post ("scan: dump %s", t);  // --
    
    while (l) {
        const char *path = pathlist_getPath (l);
        l = pathlist_getNext (l);
        searchpath_reportWrite (f, "Path:", path);
    }
    
    l = searchpath_external;
    
    while (l) {
        const char *path = pathlist_getPath (l);
        l = pathlist_getNext (l);
        searchpath_reportWrite (f, "External:", path);
    }
    
    l = searchpath_patch;
    
    while (l) {
        const char *path = pathlist_getPath (l);
        l = pathlist_getNext (l);
        searchpath_reportWrite (f, "Patch:", path);
    }
    
    l = searchpath_help;
    
    while (l) {
        const char *path = pathlist_getPath (l);
        l = pathlist_getNext (l);
        searchpath_reportWrite (f, "Help:", path);
    }
    
    l = searchpath_duplicates;
    
    while (l) {
        const char *path = pathlist_getPath (l);
        l = pathlist_getNext (l);
        searchpath_reportWrite (f, "Duplicates:", path);
    }
    
    close (f);
    //
    }
}

int searchpath_isExternalAvailable (t_symbol *s)
{
    int available = 0; char t[PD_STRING] = { 0 };
    
    if (!string_sprintf (t, PD_STRING, "%s%s", s->s_name, PD_PLUGIN)) {
        available = pathlist_contains (searchpath_external, t);
    }
    
    return available;
}

int searchpath_isAbstractionAvailable (t_symbol *s)
{
    int available = 0; char t[PD_STRING] = { 0 };
    
    if (!string_sprintf (t, PD_STRING, "%s%s", s->s_name, PD_PATCH)) {
        available = pathlist_contains (searchpath_patch, t);
    }
    
    return available;
}

int searchpath_hasDuplicates (void)
{
    return (pathlist_getSize (searchpath_duplicates) != 0);
}

void searchpath_extendedMatchedAtIndex (int n)
{
    PD_ASSERT (searchpath_extended);
    
    /* Matching folders are moved to front. */
    
    if (n > 0) { searchpath_extended = pathlist_moveFront (searchpath_extended, n); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void searchpath_appendRoot (const char *filepath)
{
    searchpath_roots = pathlist_newAppend (searchpath_roots, NULL, filepath);
}

void searchpath_setRootsEncoded (int argc, t_atom *argv)
{
    int i;
    
    pathlist_free (searchpath_roots); searchpath_roots = NULL;
    
    for (i = 0; i < argc; i++) {
    //
    t_symbol *path = symbol_decode (atom_getSymbolAtIndex (i, argc, argv));
        
    searchpath_roots = pathlist_newAppend (searchpath_roots, NULL, path->s_name);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist *searchpath_getRoots (void)
{
    return searchpath_roots;
}

t_pathlist *searchpath_getExtended (void)
{
    return searchpath_extended;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void searchpath_release (void)
{
    PD_ASSERT (!pathlist_check (searchpath_roots));
    PD_ASSERT (!pathlist_check (searchpath_extended));
    PD_ASSERT (!pathlist_check (searchpath_external));
    PD_ASSERT (!pathlist_check (searchpath_patch));
    PD_ASSERT (!pathlist_check (searchpath_help));
    PD_ASSERT (!pathlist_check (searchpath_duplicates));
    
    pathlist_free (searchpath_roots);
    pathlist_free (searchpath_extended);
    pathlist_free (searchpath_external);
    pathlist_free (searchpath_patch);
    pathlist_free (searchpath_help);
    pathlist_free (searchpath_duplicates);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
