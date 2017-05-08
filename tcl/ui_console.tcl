
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2017 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# The application console.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_console 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} { ::ui_console::_create }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc post {message} { 

    .console.text.internal insert end $message basicLog
    .console.text.internal insert end "\n"
    
    after idle ::ui_console::_update
}

proc warning {message} { 

    .console.text.internal insert end $message warningLog
    .console.text.internal insert end "\n"
    
    after idle ::ui_console::_update
}

proc error {message} { 

    .console.text.internal insert end $message errorLog
    .console.text.internal insert end "\n"
    
    after idle ::ui_console::_update
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {} {

    toplevel .console -class PdConsole
    wm title .console [_ $::var(appName)]
    wm group .console .
    
    wm minsize  .console {*}[::styleMinimumSize]
    wm geometry .console "=400x300+30+60"
    
    .console configure -menu .menubar

    ttk::scrollbar  .console.scroll     -command ".console.text yview"
    text            .console.text       -font [::styleFontConsole] \
                                        -borderwidth 0 \
                                        -insertwidth 0 \
                                        -highlightthickness 0 \
                                        -undo 0 \
                                        -yscrollcommand ".console.scroll set"
        
    pack .console.text                  -side right -fill both -expand 1
        
    bind .console <<SelectAll>> ".console.text tag add sel 1.0 end"
    
    wm protocol .console WM_DELETE_WINDOW   { ::ui_console::closed }
    
    # Set the color layout. 
    
    .console.text tag configure errorLog    -foreground red
    .console.text tag configure warningLog  -foreground red
    .console.text tag configure basicLog    -foreground black
    
    # Read-only text widget ( http://wiki.tcl.tk/1152 ).
  
    rename ::.console.text ::.console.text.internal

    proc ::.console.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert"  {}
            "delete"  {}
            "default" { return [eval ::.console.text.internal $args] }
        }
    }
}

proc closed {} {

    ::ui_interface::pdsend "pd _quit"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _update {} {

    set view [.console.text yview]
    
    if {[lindex $view 0] == 0.0 && [lindex $view 1] == 1.0} {
        pack forget .console.scroll
    } else {
        pack .console.scroll -side right -fill y -before .console.text
    }
        
    .console.text yview end
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
