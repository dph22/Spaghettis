
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2016 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# File I/O management. 

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_file 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_file:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable untitledName       "Untitled"
variable untitledNumber     "1"
variable directoryNew       [pwd]
variable directoryOpen      [pwd]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    variable directoryNew
    variable directoryOpen
    
    if {[tk windowingsystem] eq "aqua"} { set directoryNew $::env(HOME); set directoryOpen $::env(HOME) }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Handling menu actions.

proc newPatch {} {

    variable untitledName
    variable untitledNumber
    variable directoryNew
    
    ::ui_interface::pdsend "pd new $untitledName-$untitledNumber [::escaped $directoryNew]"
    
    incr untitledNumber 
}

proc openPatch {} {

    variable directoryOpen
    
    set f [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir $directoryOpen]

    if {$f ne ""} {
        foreach filename $f { ::ui_file::openFile $filename }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Open a file (currently only patches).

proc openFile {filename} {

    variable directoryOpen
    
    set basename  [file tail $filename]
    set extension [file extension $filename]
    set directory [file normalize [file dirname $filename]]
    
    if {[file exists $filename]} {
    if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
        ::ui_interface::pdsend "pd open [::escaped $basename] [::escaped $directory]"
        set directoryOpen $directory
        return
    }
    }
    
    ::ui_console::post [format [_ "Opening %s failed."] $filename]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the executable.

proc saveAs {target filename directory destroy} {

    variable directoryNew
    
    if {![file isdirectory $directory]} { set directory $directoryNew }
    
    set filename [tk_getSaveFile    -initialfile $filename \
                                    -initialdir $directory \
                                    -filetypes $::var(filesTypes) \
                                    -defaultextension [lindex $::var(filesExtensions) 0]]
                      
    if {$filename ne ""} {
        set basename  [file tail $filename]
        set directory [file normalize [file dirname $filename]]
        ::ui_interface::pdsend "$target savetofile [::escaped $basename] [::escaped $directory] $destroy"
        set directoryNew $directory
    }
}

# Function called by the openpanel object.

proc openPanel {target directory} {

    variable directoryOpen
    
    if {![file isdirectory $directory]} { set directory $directoryOpen }
    
    set filename [tk_getOpenFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::ui_interface::pdsend "$target callback [::escaped $filename]"
    }
}

# Function called by the savepanel object.

proc savePanel {target directory} {

    variable directoryNew
    
    if {![file isdirectory $directory]} { set directory $directoryNew }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::ui_interface::pdsend "$target callback [::escaped $filename]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
