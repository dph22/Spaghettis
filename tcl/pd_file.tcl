
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_file 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_patch

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_file:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export directoryNew
namespace export directoryOpen
namespace export openPatches
namespace export openFile
namespace export saveAs
namespace export openPanel
namespace export savePanel

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc directoryNew {} {

    if {![file isdirectory $::var(directoryNew)]} { set ::var(directoryNew) $::env(HOME) }
    
    return $::var(directoryNew)
}


proc directoryOpen {} {

    if {![file isdirectory $::var(directoryOpen)]} { set ::var(directoryOpen) $::env(HOME) }
    
    return $::var(directoryOpen)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc openPatches {} {

    set f [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir [::pd_file::directoryOpen]]

    if {$f ne ""} {
        foreach filename $f { ::pd_file::openFile $filename }
    }
}

proc openFile {filename} {

    set basename  [file tail $filename]
    set extension [file extension $filename]
    set directory [file normalize [file dirname $filename]]
    
    if {[file exists $filename]} {
    if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
        ::pd_patch::started_loading_file [format "%s/%s" $basename $filename]
        ::pd_connect::pdsend "pd open [::enquote $basename] [::enquote $directory]"
        set ::var(directoryOpen) $directory
    }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the pd executable.

proc saveAs {target filename directory destroy} {

    if {![file isdirectory $directory]} { set directory [::pd_file::directoryNew] }
    
    set filename [tk_getSaveFile    -initialfile $filename \
                                    -initialdir $directory \
                                    -defaultextension [lindex $::var(filesExtensions) 0]]
                      
    if {$filename ne ""} {
        set basename  [file tail $filename]
        set directory [file normalize [file dirname $filename]]
        ::pd_connect::pdsend "$target savetofile [::enquote $basename] [::enquote $directory] $destroy"
        set ::var(directoryNew) $directory
    }
}

# Function called by the openpanel object.

proc openPanel {target directory} {

    if {![file isdirectory $directory]} { set directory [::pd_file::directoryOpen] }
    
    set filename [tk_getOpenFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# Function called by the savepanel object.

proc savePanel {target directory} {

    if {![file isdirectory $directory]} { set directory [::pd_file::directoryNew] }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
