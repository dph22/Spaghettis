
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2019 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# IEM ( http://iem.kug.ac.at/ ) objects properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_iem 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_iem:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  iemType
variable  iemWidth
variable  iemWidthMinimum
variable  iemWidthLabel
variable  iemHeight
variable  iemHeightMinimum
variable  iemHeightLabel
variable  iemOption1
variable  iemOption2
variable  iemCheck
variable  iemLoadbang
variable  iemExtra
variable  iemExtraMaximum
variable  iemExtraLabel
variable  iemSend
variable  iemReceive
variable  iemBackgroundColor
variable  iemFrontColor
variable  iemSteady
variable  iemSave
variable  iemFocused

array set iemType               {}
array set iemWidth              {}
array set iemWidthMinimum       {}
array set iemWidthLabel         {}
array set iemHeight             {}
array set iemHeightMinimum      {}
array set iemHeightLabel        {}
array set iemOption1            {}
array set iemOption2            {}
array set iemCheck              {}
array set iemLoadbang           {}
array set iemExtra              {}
array set iemExtraMaximum       {}
array set iemExtraLabel         {}
array set iemSend               {}
array set iemReceive            {}
array set iemBackgroundColor    {}
array set iemFrontColor         {}
array set iemSteady             {}
array set iemSave               {}
array set iemFocused            {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _focus {top item} {
    
    variable iemFocused
    
    if {$iemFocused($top) == 0} { focus $item; after idle "$item selection range 0 end"; }
    
    set iemFocused($top) 1
}

proc create {top type
             width widthMinimum widthLabel height heightMinimum heightLabel
             option1 option1Label option2 option2Label 
             check check1 check2 
             loadbang
             extra extraMaximum extraLabel
             send receive
             backgroundColor frontColor
             steady
             save} {
    
    variable iemType
    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel
    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel
    variable iemSend
    variable iemReceive
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemSteady
    variable iemSave
    variable iemFocused

    set iemType($top)               $type
    set iemWidth($top)              $width
    set iemWidthMinimum($top)       $widthMinimum
    set iemWidthLabel($top)         $widthLabel
    set iemHeight($top)             $height
    set iemHeightMinimum($top)      $heightMinimum
    set iemHeightLabel($top)        $heightLabel
    set iemOption1($top)            $option1
    set iemOption2($top)            $option2
    set iemCheck($top)              $check
    set iemLoadbang($top)           $loadbang
    set iemExtra($top)              $extra
    set iemExtraMaximum($top)       $extraMaximum
    set iemExtraLabel($top)         $extraLabel
    set iemSend($top)               [::hashToDollar [::parseNil $send]]
    set iemReceive($top)            [::hashToDollar [::parseNil $receive]]
    set iemBackgroundColor($top)    $backgroundColor
    set iemFrontColor($top)         $frontColor
    set iemSteady($top)             $steady
    set iemSave($top)               $save
    set iemFocused($top)            0

    set iemWidth(${top}.old)        $width
    set iemHeight(${top}.old)       $height
    set iemOption1(${top}.old)      $option1
    set iemOption2(${top}.old)      $option2
    set iemExtra(${top}.old)        $extra
    set iemSend(${top}.old)         [::hashToDollar [::parseNil $send]]
    set iemReceive(${top}.old)      [::hashToDollar [::parseNil $receive]]

    toplevel $top -class PdDialog
    wm title $top [_ $type]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    ttk::frame      $top.f                              {*}[::styleFrame]
    ttk::labelframe $top.f.properties                   {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.colors                       {*}[::styleLabelFrame]  -text [_ "Colors"]
    ttk::labelframe $top.f.binding                      {*}[::styleLabelFrame]  -text [_ "Binding"]
        
    pack $top.f                                         {*}[::packMain]
    pack $top.f.properties                              {*}[::packCategory]
    pack $top.f.colors                                  {*}[::packCategoryNext]
    pack $top.f.binding                                 {*}[::packCategoryNext]
        
    set row -1
    
    if {$widthLabel ne $::var(nil)}     {
    
        ttk::label $top.f.properties.widthLabel         {*}[::styleLabel] \
                                                            -text [_ $widthLabel]
        ttk::entry $top.f.properties.width              {*}[::styleEntryNumber] \
                                                            -textvariable ::ui_iem::iemWidth($top) \
                                                            -width $::width(small)
        
        grid $top.f.properties.widthLabel               -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.width                    -row $row       -column 2 -sticky ew

        bind $top.f.properties.width <Return>           { ::nextEntry %W }
        
        ::ui_iem::_focus $top $top.f.properties.width
    }
    
    if {$heightLabel ne $::var(nil)}    {
    
        ttk::label $top.f.properties.heightLabel        {*}[::styleLabel] \
                                                            -text [_ $heightLabel]
        ttk::entry $top.f.properties.height             {*}[::styleEntryNumber] \
                                                            -textvariable ::ui_iem::iemHeight($top) \
                                                            -width $::width(small)
                                                        
        grid $top.f.properties.heightLabel              -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.height                   -row $row       -column 2 -sticky ew
        
        bind $top.f.properties.height <Return>          { ::nextEntry %W }
    }
    
    if {$option1Label ne $::var(nil)}   {
    
        ttk::label $top.f.properties.option1Label       {*}[::styleLabel] \
                                                            -text [_ $option1Label]
        ttk::entry $top.f.properties.option1            {*}[::styleEntryNumber] \
                                                            -textvariable ::ui_iem::iemOption1($top) \
                                                            -width $::width(small)

        grid $top.f.properties.option1Label             -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.option1                  -row $row       -column 2 -sticky ew
        
        bind  $top.f.properties.option1 <Return>        { ::nextEntry %W }
        
        ::ui_iem::_focus $top $top.f.properties.option1
    }
    
    if {$option2Label ne $::var(nil)}   {
    
        ttk::label $top.f.properties.option2Label       {*}[::styleLabel] \
                                                            -text [_ $option2Label]
        ttk::entry $top.f.properties.option2            {*}[::styleEntryNumber] \
                                                            -textvariable ::ui_iem::iemOption2($top) \
                                                            -width $::width(small)
                                                        
        grid $top.f.properties.option2Label             -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.option2                  -row $row       -column 2 -sticky ew
        
        bind $top.f.properties.option2 <Return>         { ::nextEntry %W }
    }
    
    if {$extraLabel ne $::var(nil)}     {
    
        ttk::label $top.f.properties.extraLabel         {*}[::styleLabel] \
                                                            -text [_ $extraLabel]
        ttk::entry $top.f.properties.extra              {*}[::styleEntryNumber] \
                                                            -textvariable ::ui_iem::iemExtra($top) \
                                                            -width $::width(small)

        grid $top.f.properties.extraLabel               -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.extra                    -row $row       -column 2 -sticky ew
        
        bind $top.f.properties.extra <Return>           { ::nextEntry %W }
    }
    
    if {$save != -1}    {
    
        ttk::label $top.f.properties.saveLabel          {*}[::styleLabel] \
                                                            -text [_ "Save Contents"]
        ttk::checkbutton $top.f.properties.save         {*}[::styleCheckButton] \
                                                            -variable ::ui_iem::iemSave($top) \
                                                            -takefocus 0
        
        grid $top.f.properties.saveLabel                -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.save                     -row $row       -column 2 -sticky ew
    }
    
    if {$loadbang != -1}    {
    
        ttk::label $top.f.properties.loadbangLabel      {*}[::styleLabel] \
                                                            -text [_ "Loadbang"]
        ttk::checkbutton $top.f.properties.loadbang     {*}[::styleCheckButton] \
                                                            -variable ::ui_iem::iemLoadbang($top) \
                                                            -takefocus 0
        
        grid $top.f.properties.loadbangLabel            -row [incr row] -column 0 -sticky ew
        grid $top.f.properties.loadbang                 -row $row       -column 2 -sticky ew
    }
    
    if {$check != -1}                   {
    if {$check1 ne $::var(nil)}         {
    if {$check2 ne $::var(nil)}         {
    
        set values [list [_ $check1] [_ $check2]]
        
        ::createMenuByIndex $top.f.properties.check     $values ::ui_iem::iemCheck($top) \
                                                            -width [::measure $values]
        
        grid $top.f.properties.check                    -row [incr row] -column 1 -sticky ew -columnspan 2
    }
    }
    }
    
    if {$steady != -1}              {
    
        set values [list [_ "Skip"] [_ "Steady"]]
        
        ::createMenuByIndex $top.f.properties.steady    $values ::ui_iem::iemSteady($top) \
                                                            -width [::measure $values]
        
        grid $top.f.properties.steady                   -row [incr row] -column 1 -sticky ew -columnspan 2
    }
    
    ttk::label $top.f.colors.backgroundLabel            {*}[::styleLabel]   -text [_ "Background"]
    
    label $top.f.colors.background                      -background [::integerToColor $backgroundColor] \
                                                            -width $::width(small)

    ttk::label $top.f.colors.frontLabel                 {*}[::styleLabel]   -text [_ "Foreground"]
    
    label $top.f.colors.front                           -background [::integerToColor $frontColor] \
                                                            -width $::width(small)
                                                        
    ttk::label $top.f.binding.sendLabel                 {*}[::styleLabel] \
                                                            -text [_ "Send"]
    ttk::entry $top.f.binding.send                      {*}[::styleEntry] \
                                                            -textvariable ::ui_iem::iemSend($top) \
                                                            -width $::width(medium)
    ttk::label $top.f.binding.receiveLabel              {*}[::styleLabel] \
                                                            -text [_ "Receive"]
    ttk::entry $top.f.binding.receive                   {*}[::styleEntry] \
                                                            -textvariable ::ui_iem::iemReceive($top) \
                                                            -width $::width(medium)
                                              
    grid $top.f.colors.backgroundLabel                  -row 0 -column 0 -sticky ew
    grid $top.f.colors.background                       -row 0 -column 1 -sticky ew -pady 2
    grid $top.f.colors.frontLabel                       -row 1 -column 0 -sticky ew
    grid $top.f.colors.front                            -row 1 -column 1 -sticky ew -pady 2

    grid $top.f.binding.sendLabel                       -row 0 -column 0 -sticky ew
    grid $top.f.binding.send                            -row 0 -column 1 -sticky ew -columnspan 2
    grid $top.f.binding.receiveLabel                    -row 1 -column 0 -sticky ew
    grid $top.f.binding.receive                         -row 1 -column 1 -sticky ew -columnspan 2
    
    bind $top.f.colors.background   <Button>            "::ui_iem::_chooseBackgroundColor $top %W"
    bind $top.f.colors.front        <Button>            "::ui_iem::_chooseFrontColor $top %W"
    
    bind $top.f.binding.send        <Return>            { ::nextEntry %W }
    bind $top.f.binding.receive     <Return>            { ::nextEntry %W }
    
    grid columnconfigure $top.f.properties              0 -weight 3
    grid columnconfigure $top.f.properties              1 -weight 1
    grid columnconfigure $top.f.properties              2 -weight 0
    grid columnconfigure $top.f.colors                  0 -weight 1
    grid columnconfigure $top.f.colors                  1 -weight 0
    grid columnconfigure $top.f.binding                 0 -weight 3
    grid columnconfigure $top.f.binding                 1 -weight 1
    grid columnconfigure $top.f.binding                 2 -weight 0
    
    wm protocol $top WM_DELETE_WINDOW   "::ui_iem::closed $top"
}

proc closed {top} {
    
    ::ui_iem::_apply $top
    ::cancel $top
}

proc released {top} {

    variable iemType
    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel
    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel
    variable iemSend
    variable iemReceive
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemSteady
    variable iemSave
    variable iemFocused
    
    unset iemType($top)
    unset iemWidth($top)
    unset iemWidthMinimum($top)
    unset iemWidthLabel($top)
    unset iemHeight($top)
    unset iemHeightMinimum($top)
    unset iemHeightLabel($top)
    unset iemOption1($top)
    unset iemOption2($top)
    unset iemCheck($top)
    unset iemLoadbang($top)
    unset iemExtra($top)
    unset iemExtraMaximum($top)
    unset iemExtraLabel($top)
    unset iemSend($top)
    unset iemReceive($top)
    unset iemBackgroundColor($top)
    unset iemFrontColor($top)
    unset iemSteady($top)
    unset iemSave($top)
    unset iemFocused($top)
    
    unset iemWidth(${top}.old)
    unset iemHeight(${top}.old)
    unset iemOption1(${top}.old)
    unset iemOption2(${top}.old)
    unset iemExtra(${top}.old)
    unset iemSend(${top}.old)
    unset iemReceive(${top}.old)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable iemWidth
    variable iemHeight
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemSend
    variable iemReceive
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemSteady
    variable iemSave
    
    _forceWidth     $top
    _forceHeight    $top
    _forceExtra     $top
    _forceOptions   $top
    _forceNames     $top
    
    ::ui_interface::pdsend "$top _iemdialog \
            $iemWidth($top) \
            $iemHeight($top) \
            $iemOption1($top) \
            $iemOption2($top) \
            $iemCheck($top) \
            $iemLoadbang($top) \
            $iemExtra($top) \
            [::sanitized [::dollarToHash [::withNil $iemSend($top)]]] \
            [::sanitized [::dollarToHash [::withNil $iemReceive($top)]]] \
            $iemBackgroundColor($top) \
            $iemFrontColor($top) \
            $iemSteady($top) \
            $iemSave($top)"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceWidth {top} {

    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel

    if {$iemWidthLabel($top) ne $::var(nil)} {
        set iemWidth($top) [::ifInteger $iemWidth($top) $iemWidth(${top}.old)]
        set iemWidth($top) [::tcl::mathfunc::max $iemWidth($top) $iemWidthMinimum($top)]
    }
}

proc _forceHeight {top} {

    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel

    if {$::ui_iem::iemHeightLabel($top) ne $::var(nil)} {
        set iemHeight($top) [::ifInteger $iemHeight($top) $iemHeight(${top}.old)]
        set iemHeight($top) [::tcl::mathfunc::max $iemHeight($top) $iemHeightMinimum($top)]
    }
}

proc _forceExtra {top} {

    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel

    if {$::ui_iem::iemExtraLabel($top) ne $::var(nil)} {
        set iemExtra($top) [::ifInteger $iemExtra($top) $iemExtra(${top}.old)]
        set iemExtra($top) [::tcl::mathfunc::max $iemExtra($top) 1]
        set iemExtra($top) [::tcl::mathfunc::min $iemExtra($top) $iemExtraMaximum($top)]
    }
}

proc _forceOptions {top} {
    
    variable iemType
    variable iemOption1
    variable iemOption2

    switch -regexp -- $iemType($top) {
        "Bang"          {
            set iemOption1($top) [::ifInteger $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::tcl::mathfunc::max $iemOption1($top) 10]
        }
        "Toggle"        {
            set iemOption1($top) [::ifNumber  $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::ifNotZero $iemOption1($top) $iemOption1(${top}.old)]
        }
        "Panel"         {
            set iemOption1($top) [::ifInteger $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::tcl::mathfunc::max $iemOption1($top) 1]
            set iemOption2($top) [::ifInteger $iemOption2($top) $iemOption2(${top}.old)]
            set iemOption2($top) [::tcl::mathfunc::max $iemOption2($top) 1]
        }
        "Slider|Dial" {
            set iemOption1($top) [::ifNumber $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption2($top) [::ifNumber $iemOption2($top) $iemOption2(${top}.old)]
        } 
    }
}

proc _forceNames {top} {

    variable iemSend
    variable iemReceive

    set iemSend($top)    [::ifNotNumber $iemSend($top) $iemSend(${top}.old)]
    set iemReceive($top) [::ifNotNumber $iemReceive($top) $iemReceive(${top}.old)]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _chooseBackgroundColor {top label} {
    
    variable iemBackgroundColor
    
    set iemBackgroundColor($top) [::chooseColor $label $iemBackgroundColor($top) [_ "Background"]]
}

proc _chooseFrontColor {top label} {
    
    variable iemFrontColor
    
    set iemFrontColor($top) [::chooseColor $label $iemFrontColor($top) [_ "Foreground"]]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
