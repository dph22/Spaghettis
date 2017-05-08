
/* Copyright (c) 1999-2017 Guenter Geiger and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void path_slashToBackslashIfNecessary (char *s)
{
    #if PD_WINDOWS
    
    string_replaceCharacter (s, '/', '\\');
    
    #endif
}

void path_backslashToSlashIfNecessary (char *s)
{
    #if PD_WINDOWS
    
    string_replaceCharacter (s, '\\', '/');
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if !PD_WINDOWS

int path_isFileExist (const char *filepath)
{
    struct stat t; return (stat (filepath, &t) == 0);
}

int path_isFileExistAsRegularFile (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISREG (t.st_mode));
}

#else   
    /* < https://msdn.microsoft.com/en-us/library/aa364944%28v=vs.85%29.aspx > */
    #error
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error path_withDirectoryAndName (char *dest, size_t size, const char *directory, const char *name)
{
    t_error err = PD_ERROR;
    
    PD_ASSERT (directory);
    PD_ASSERT (name);
    
    if (*name) {
    //
    #if PD_WINDOWS 
        int absolute = (!(*directory) || (name[0] && name[1] == ':'));
    #else 
        int absolute = (!(*directory) || (name[0] == '/'));
    #endif
    
    err = PD_ERROR_NONE;
    
    if (!absolute) {
        err |= string_add (dest, size, directory);
        err |= string_add (dest, size, "/");
    }
    
    err |= string_add (dest, size, name);
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
