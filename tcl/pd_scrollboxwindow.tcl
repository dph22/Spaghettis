
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

####### pd_scrollboxwindow -- pd_scrollbox window with default bindings #########
## This is the base dialog behind the Path and Startup dialogs
## This namespace specifies everything the two dialogs have in common,
## with arguments specifying the differences
##
## By default, this creates a dialog centered on the viewing area of the screen
## with cancel, apply, and OK buttons
## which contains a pd_scrollbox widget populated with the given data

package provide pd_scrollboxwindow 0.1

package require pd_scrollbox

namespace eval pd_scrollboxwindow {
}


proc ::pd_scrollboxwindow::get_listdata {mytoplevel} {
    return [$mytoplevel.listbox.box get 0 end]
}

proc ::pd_scrollboxwindow::do_apply {mytoplevel commit_method listdata} {
    $commit_method [pdtk_encode $listdata]
    ::pd_connect::pdsend "pd save-preferences"
}

# Cancel button action
proc ::pd_scrollboxwindow::cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

# Apply button action
proc ::pd_scrollboxwindow::apply {mytoplevel commit_method } {
    do_apply $mytoplevel $commit_method [get_listdata $mytoplevel]
}

# OK button action
# The "commit" action can take a second or more,
# long enough to be noticeable, so we only write
# the changes after closing the dialog
proc ::pd_scrollboxwindow::ok {mytoplevel commit_method } {
    set listdata [get_listdata $mytoplevel]
    cancel $mytoplevel
    do_apply $mytoplevel $commit_method $listdata
}

# "Constructor" function for building the window
# id -- the window id to use
# listdata -- the data used to populate the pd_scrollbox
# add_method -- a reference to a proc to be called when the user adds a new item
# edit_method -- same as above, for editing and existing item
# commit_method -- same as above, to commit during the "apply" action
# title -- top-level title for the dialog
# width, height -- initial width and height dimensions for the window, also minimum size
proc ::pd_scrollboxwindow::make {mytoplevel listdata add_method edit_method commit_method title width height } {
    wm deiconify .console
    raise .console
    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel $title
    wm group $mytoplevel .
    wm transient $mytoplevel .console
    wm protocol $mytoplevel WM_DELETE_WINDOW "::pd_scrollboxwindow::cancel $mytoplevel"

    # Enforce a minimum size for the window
    wm minsize $mytoplevel $width $height

    # Set the current dimensions of the window
    wm geometry $mytoplevel "${width}x${height}"

    # Add the pd_scrollbox widget
    ::pd_scrollbox::make $mytoplevel $listdata $add_method $edit_method

    # Use two frames for the buttons, since we want them both
    # bottom and right
    frame $mytoplevel.nb
    pack $mytoplevel.nb -side bottom -fill x -pady 2m

    frame $mytoplevel.nb.buttonframe
    pack $mytoplevel.nb.buttonframe -side right -padx 2m

    button $mytoplevel.nb.buttonframe.cancel -text [_ "Cancel"]\
        -command "::pd_scrollboxwindow::cancel $mytoplevel"
    button $mytoplevel.nb.buttonframe.apply -text [_ "Apply"]\
        -command "::pd_scrollboxwindow::apply $mytoplevel $commit_method"
    button $mytoplevel.nb.buttonframe.ok -text [_ "OK"]\
        -command "::pd_scrollboxwindow::ok $mytoplevel $commit_method"

    pack $mytoplevel.nb.buttonframe.cancel -side left -expand 1 -padx 2m
    pack $mytoplevel.nb.buttonframe.apply -side left -expand 1 -padx 2m
    pack $mytoplevel.nb.buttonframe.ok -side left -expand 1 -padx 2m
}

proc pdtk_encode { listdata } {
    set outlist {}
    foreach this_path $listdata {
        if {0==[string match "" $this_path]} {
            lappend outlist [::encode $this_path]
        }
    }
    return $outlist
}

