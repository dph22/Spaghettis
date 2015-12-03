
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Modal windows.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_confirm 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_confirm:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc checkAction {top message ifYes implicit} {

    if {[winfo viewable $top]} {
        set r [tk_messageBox -message $message -type yesno -default $implicit -icon question -parent $top]
    } else {
        set r [tk_messageBox -message $message -type yesno -default $implicit -icon question]
    }
    
    if {$r eq "yes"} {
        uplevel 0 $ifYes
    }
}

proc checkClose {top ifYes ifNo ifCancel} {

    set message [format [_ "Save \"%s\" before closing?"] [::getTitle $top]]
    
    if {[winfo viewable $top]} {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question -parent $top]
    } else {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question]
    }

    switch -- $r {
        yes     { uplevel 0 $ifYes    }
        no      { uplevel 0 $ifNo     }
        cancel  { uplevel 0 $ifCancel }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
