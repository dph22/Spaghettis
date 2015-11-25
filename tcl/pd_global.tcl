
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Global handy functions.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFont {size} { 

    if {[lsearch -exact $::var(fontSizes) $size] > -1} { 
        return [format "::var(font%s)" $size] 
        
    } else {
        set next [lindex $::var(fontSizes) end]
        foreach f $::var(fontSizes) { if {$f > $size} { set next $f; break } }
        return [format "::var(font%s)" $next]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getTitle {top} { 
    
    if {[winfo class $top] eq "PdPatch"} { return [::pd_patch::getTitle $top] }
    return [wm title $top]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc nextEntry {w} {

    set next [tk_focusNext $w]
    focus $next
    if {[string match "*Entry" [winfo class $next]]} { $next selection range 0 end }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc rightNextTo {top} {

    set x [expr {[winfo rootx $top]} + 50]
    set y [expr {[winfo rooty $top]} + 50]
    
    return [format "+%d+%d" $x $y]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Escaping and quoting.

proc encode  {x} { concat +[string map {" " "+_" "$" "+d" ";" "+s" "," "+c" "+" "++"} $x] }
proc enquote {x} { string map {"," "\\," ";" "\\;" " " "\\ "} $x }
proc parse   {x} { if {$x eq "empty"} { return "" } else { return [string map {"#" "$"} $x] } }
proc unspace {x} {
    set y [string map {" " "_" ";" "" "," "" "{" "" "}" "" "\\" ""} $x]
    if {$y eq ""} { set y "empty" }
    concat $y
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ifInteger {new old} {

    if {[string is integer -strict $new]} { return $new } else { return $old }
}

proc ifNumber  {new old} {

    if {[string is double -strict $new]}  { return $new } else { return $old }
}

proc ifNonZero {new old} {

    if {$new != 0.0} { return $new } else { return $old }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ping {} {
    ::pd_connect::pdsend "pd ping"
}

proc watchdog {} {
    ::pd_connect::pdsend "pd watchdog"; after 2000 { ::watchdog }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For future msgcat ( https://www.gnu.org/software/gettext/manual/html_node/Tcl.html ).

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
