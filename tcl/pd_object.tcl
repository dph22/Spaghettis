
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_object 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_object:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newText {c tags x y text fontSize color} {
    
    $c create text $x $y    -tags $tags \
                            -text $text \
                            -fill $color \
                            -anchor nw \
                            -font [getFont $fontSize]
}

proc setText {c tag text} {

    $c itemconfig $tag -text $text
}

proc setEditing {top tag isEditing} {

    if {$isEditing == 0} {selection clear $top.c}
    
    $top.c focus $tag
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
