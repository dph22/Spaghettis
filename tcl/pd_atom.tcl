
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Atom property.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_atom 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_atom:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  atomWidth
variable  atomLow
variable  atomHigh
variable  atomSend
variable  atomReceive
variable  atomName
variable  atomPosition

array set atomWidth     {}
array set atomLow       {}
array set atomHigh      {}
array set atomSend      {}
array set atomReceive   {}
array set atomName      {}
array set atomPosition  {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top width low high send receive name position} {

    ::pd_atom::_create $top $width $low $high $send $receive $name $position
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top width low high send receive name position} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
    
    toplevel $top -class PdDialog
    wm title $top [_ "Atom"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]

    set atomWidth($top)         $width
    set atomLow($top)           $low
    set atomHigh($top)          $high
    set atomSend($top)          [::parseDash $send]
    set atomReceive($top)       [::parseDash $receive]
    set atomName($top)          [::parseDash $name]
    set atomPosition($top)      $position
    
    set atomWidth(${top}.old)   $width
    set atomLow(${top}.old)     $low
    set atomHigh(${top}.old)    $high

    set values {"Left" "Right" "Top" "Bottom"}
     
    ttk::frame      $top.f                          {*}[::styleFrame]
    ttk::labelframe $top.f.properties               {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.label                    {*}[::styleLabelFrame]  -text [_ "Label"]
    
    pack $top.f                                     {*}[::packMain]
    pack $top.f.properties                          {*}[::packCategory]
    pack $top.f.label                               {*}[::packCategoryNext]
    
    ttk::label $top.f.properties.widthLabel         {*}[::styleLabel] \
                                                        -text [_ "Digits"]
    ttk::entry $top.f.properties.width              {*}[::styleEntryNumber] \
                                                        -textvariable ::pd_atom::atomWidth($top) \
                                                        -width $::width(small)
    
    ttk::label $top.f.properties.lowLabel           {*}[::styleLabel] \
                                                        -text [_ "Value Low"]
    ttk::entry $top.f.properties.low                {*}[::styleEntryNumber] \
                                                        -textvariable ::pd_atom::atomLow($top) \
                                                        -width $::width(small)
    
    ttk::label $top.f.properties.highLabel          {*}[::styleLabel] \
                                                        -text [_ "Value High"]
    ttk::entry $top.f.properties.high               {*}[::styleEntryNumber] \
                                                        -textvariable ::pd_atom::atomHigh($top) \
                                                        -width $::width(small)
    
    ttk::label $top.f.label.nameLabel               {*}[::styleLabel] \
                                                        -text [_ "Name"]
    ttk::entry $top.f.label.name                    {*}[::styleEntry] \
                                                        -textvariable ::pd_atom::atomName($top) \
                                                        -width $::width(large)
                                                        
    ttk::label $top.f.label.positionLabel           {*}[::styleLabel] \
                                                        -text [_ "Position"]
                                                        
    ::createMenuByIndex $top.f.label.position       $values ::pd_atom::atomPosition($top) \
                                                        -width [::measure $values]
    
    ttk::label $top.f.label.sendLabel               {*}[::styleLabel] \
                                                        -text [_ "Send"]
    ttk::entry $top.f.label.send                    {*}[::styleEntry] \
                                                        -textvariable ::pd_atom::atomSend($top) \
                                                        -width $::width(large)
    
    ttk::label $top.f.label.receiveLabel            {*}[::styleLabel] \
                                                        -text [_ "Receive"]
    ttk::entry $top.f.label.receive                 {*}[::styleEntry] \
                                                        -textvariable ::pd_atom::atomReceive($top) \
                                                        -width $::width(large)

    grid $top.f.properties.widthLabel               -row 0 -column 0 -sticky nsew
    grid $top.f.properties.width                    -row 0 -column 1 -sticky nsew
    grid $top.f.properties.lowLabel                 -row 1 -column 0 -sticky nsew
    grid $top.f.properties.low                      -row 1 -column 1 -sticky nsew
    grid $top.f.properties.highLabel                -row 2 -column 0 -sticky nsew
    grid $top.f.properties.high                     -row 2 -column 1 -sticky nsew
    
    grid $top.f.label.nameLabel                     -row 0 -column 0 -sticky nsew
    grid $top.f.label.name                          -row 0 -column 1 -sticky nsew
    grid $top.f.label.positionLabel                 -row 1 -column 0 -sticky nsew
    grid $top.f.label.position                      -row 1 -column 1 -sticky nsew
    grid $top.f.label.sendLabel                     -row 2 -column 0 -sticky nsew
    grid $top.f.label.send                          -row 2 -column 1 -sticky nsew
    grid $top.f.label.receiveLabel                  -row 3 -column 0 -sticky nsew
    grid $top.f.label.receive                       -row 3 -column 1 -sticky nsew
    
    grid columnconfigure $top.f.properties  0 -weight 1
    grid columnconfigure $top.f.label       0 -weight 1
    
    bind  $top.f.properties.width   <Return> { ::nextEntry %W }
    bind  $top.f.properties.low     <Return> { ::nextEntry %W }
    bind  $top.f.properties.high    <Return> { ::nextEntry %W }
    bind  $top.f.label.name         <Return> { ::nextEntry %W }
    bind  $top.f.label.send         <Return> { ::nextEntry %W }
    bind  $top.f.label.receive      <Return> { ::nextEntry %W }

    focus $top.f.properties.width
    
    after idle "$top.f.properties.width selection range 0 end"
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_atom::closed $top"
}

proc closed {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
    
    ::pd_atom::_apply $top
    
    unset atomWidth($top)
    unset atomLow($top)
    unset atomHigh($top)
    unset atomSend($top)
    unset atomReceive($top)
    unset atomName($top)
    unset atomPosition($top)
    
    unset atomWidth(${top}.old)
    unset atomLow(${top}.old)
    unset atomHigh(${top}.old)

    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
        
    ::pd_atom::_forceValues $top
    
    ::pd_connect::pdsend "$top param \
            $atomWidth($top) \
            $atomLow($top) \
            $atomHigh($top) \
            [::sanitized [::withDash $atomSend($top)]] \
            [::sanitized [::withDash $atomReceive($top)]] \
            [::sanitized [::withDash $atomName($top)]] \
            $atomPosition($top)"
    
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    
    set atomWidth($top) [::ifInteger $atomWidth($top) $atomWidth(${top}.old)]
    set atomLow($top)   [::ifInteger $atomLow($top) $atomLow(${top}.old)]
    set atomHigh($top)  [::ifInteger $atomHigh($top) $atomHigh(${top}.old)]
    
    set min [::tcl::mathfunc::min $atomLow($top) $atomHigh($top)]
    set max [::tcl::mathfunc::max $atomLow($top) $atomHigh($top)]
    
    set atomLow($top)  $min
    set atomHigh($top) $max
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
