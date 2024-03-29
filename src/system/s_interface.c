
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryTcl;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  main_portNumber;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         interface_guiSocket;                /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_receiver  *interface_guiReceiver;             /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INTERFACE_PORT                          5400

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LINUX
    #define INTERFACE_LOCALHOST                 "127.0.0.1"
#else
    #define INTERFACE_LOCALHOST                 "localhost"
#endif

#if PD_LINUX
    #define INTERFACE_WISH                      "wish8.6"
#else
    #define INTERFACE_WISH                      "wish"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error interface_fetchGui (struct sockaddr_in *server)
{
    struct hostent *host = gethostbyname (INTERFACE_LOCALHOST);
    t_error err = ((interface_guiSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0);
    err |= (fcntl (interface_guiSocket, F_SETFD, FD_CLOEXEC | O_NONBLOCK) == -1);
    
    PD_ASSERT (!err);
    
    if (host && !err) {
    //
    server->sin_family = AF_INET;
    server->sin_port = htons ((unsigned short)main_portNumber);
    memcpy ((char *)&server->sin_addr, (char *)host->h_addr, host->h_length);
    err |= (connect (interface_guiSocket, (struct sockaddr *)server, sizeof (struct sockaddr_in)) != 0);
    PD_ASSERT (!err);
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error interface_launchGuiSpawnProcess (void) 
{
    char path[PD_STRING]    = { 0 };
    char command[PD_STRING] = { 0 };
    
    t_error err = string_sprintf (path, PD_STRING, "%s/ui_main.tcl", main_directoryTcl->s_name);
    
    err |= string_sprintf (command, PD_STRING,
            INTERFACE_WISH " \"%s\" %d\n",
            path, 
            main_portNumber);
    
    if (!err) {
    //
    if ((err = (path_isFileExist (path) == 0))) { PD_BUG; }
    else {
    //
    pid_t pid = fork();
    
    if (pid < 0)   { err = PD_ERROR; PD_BUG; }
    else if (!pid) {
        if (!privilege_relinquish()) {                  /* Child lose setuid privileges. */
            execl ("/bin/sh", "sh", "-c", command, NULL);
        }
        _exit (1);
    }
    //
    }
    //
    }
       
    return err;
}

static t_error interface_launchGuiSocket (struct sockaddr_in *server, int *fd)
{
    int f = -1;
    t_error err = ((f = socket (AF_INET, SOCK_STREAM, 0)) < 0);
    err |= (fcntl (f, F_SETFD, FD_CLOEXEC | O_NONBLOCK) == -1);
    
    int arg = 1;
    err |= (setsockopt (f, IPPROTO_TCP, TCP_NODELAY, &arg, sizeof (int)) < 0);

    if (err) { PD_BUG; }
    else {
    //
    int n = 0;
    main_portNumber = INTERFACE_PORT;
    
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons ((unsigned short)main_portNumber);

    while (bind (f, (struct sockaddr *)server, sizeof (struct sockaddr_in)) < 0) {
    //
    int e = errno;

    if ((n > 20) || (e != EADDRINUSE)) { err |= PD_ERROR; PD_BUG; break; } 
    else {
        server->sin_port = htons ((unsigned short)++main_portNumber);
    }
    
    n++;
    //
    }
    
    if (!err) { *fd = f; }
    //
    }
    
    PD_ASSERT (!err);
    
    return err;
}

static t_error interface_launchGui (struct sockaddr_in *server, int *fd)
{
    t_error err = PD_ERROR_NONE;
    
    if (!(err |= interface_launchGuiSocket (server, fd))) { err |= interface_launchGuiSpawnProcess(); }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error interface_startGui (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sockaddr_in server;
    int f = -1;
    int launch = (main_portNumber == 0);
    
    if (!launch) { err = interface_fetchGui (&server); }    /* Wish first (macOS). */
    else {
    //
    if (!(err = interface_launchGui (&server, &f))) {       /* Executable first (GNU/Linux). */
        if (!(err = (listen (f, 5) < 0))) {
            socklen_t s = sizeof (struct sockaddr_in);
            err = ((interface_guiSocket = accept (f, (struct sockaddr *)&server, (socklen_t *)&s)) < 0);
        }
        PD_ASSERT (!err);
    }
    //
    }
    
    /* Listen GUI messages. */
    
    if (!err) { interface_guiReceiver = receiver_new (NULL, interface_guiSocket, NULL, NULL, 0, 0); }
    
    /* Initialize GUI. */
    
    if (!err) {
        t_pathlist *l = searchpath_getRoots();
        gui_vAdd ("::initialize\n");
        while (l) {
            gui_vAdd ("lappend ::var(searchPath) {%s}\n", pathlist_getPath (l));    // --
            l = pathlist_getNext (l);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error interface_start (void)
{
    t_error err = interface_startGui();
    
    PD_ASSERT (!err);

    if (!(err |= privilege_restore())) {
        err |= instance_dspCreate();
        err |= privilege_relinquish();
    }
    
    PD_ASSERT (!err);
    
    return err;
}

void interface_quit (void)
{
    scheduler_needToExit();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void interface_initialize (void)
{
}

void interface_release (void)
{
    if (interface_guiReceiver) { receiver_free (interface_guiReceiver); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
