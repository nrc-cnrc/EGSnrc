
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: query filename
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Joanne Treurniet, 1998
#
#  Contributors:    Blake Walters
#                   Iwan Kawrakow
#
###############################################################################
#
#  The contributors named above are only those who could be identified from
#  this file's revision history.
#
#  This code is part of the BEAMnrc code system for Monte Carlo simulation of
#  radiotherapy treatments units. BEAM was originally developed at the
#  National Research Council of Canada as part of the OMEGA collaborative
#  research project with the University of Wisconsin, and was originally
#  described in:
#
#  BEAM: A Monte Carlo code to simulate radiotherapy treatment units,
#  DWO Rogers, BA Faddegon, GX Ding, C-M Ma, J Wei and TR Mackie,
#  Medical Physics 22, 503-524 (1995).
#
#  BEAM User Manual
#  DWO Rogers, C-M Ma, B Walters, GX Ding, D Sheikh-Bagheri and G Zhang,
#  NRC Report PIRS-509A (rev D)
#
#  As well as the authors of this paper and report, Joanne Treurniet of NRC
#  made significant contributions to the code system, in particular the GUIs
#  and EGS_Windows. Mark Holmes, Brian Geiser and Paul Reckwerdt of Wisconsin
#  played important roles in the overall OMEGA project within which the BEAM
#  code system was developed.
#
#  There have been major upgrades in the BEAM code starting in 2000 which
#  have been heavily supported by Iwan Kawrakow, most notably: the port to
#  EGSnrc, the inclusion of history-by-history statistics and the development
#  of the directional bremsstrahlung splitting variance reduction technique.
#
###############################################################################


proc query_filename {next_proc_name dir ext {extrainp ""}} {

    # a direcotry browser which includes a filter ($ext) and starting
    # directory ($dir).  $next_proc_name is called when OK is clicked.

    global home filename tree globext rootx rooty helvfont next_proc

    # first make sure that the starting directory exists.
    if [file exists $dir]==0 {
	tk_dialog .nodir "Error" "The directory $dir does not exist.  It\
		must be created before continuing." error 0 OK
	return
    }

    set top .query
    toplevel $top -width 400

    set filename {}
    set tree $dir
    set globext $ext
    set next_proc $next_proc_name
    wm title .query "Select file"
    set x [expr $rootx + 50]
    set y [expr $rooty + 50]
    wm geometry .query +$x+$y

    # a label explaining to the user how to use the browser:
    message $top.mess -text "Double-click a directory name to change to\
	    that directory.  Single-click a filename to select that file.  \
	    To change the file filter, edit the extension and hit enter.  \
	    Click 'OK' to continue." -font $helvfont -width 400
    pack $top.mess -pady 5

    frame $top.f0
    label $top.f0.filterl -text "Filter: *."
    entry $top.f0.filter -textvariable globext -bg white -width 8
    pack $top.f0.filterl $top.f0.filter -side left -padx 0
    pack $top.f0 -side top -anchor w

    # scrollbar and listbox:
    frame $top.f1
    scrollbar $top.f1.scrollit -command "$top.f1.list yview"

    listbox $top.f1.list -yscroll "$top.f1.scrollit set" -relief groove\
	    -bg white -font $helvfont
    pack $top.f1.scrollit -side right -fill y
    pack $top.f1.list -side left -expand 1 -fill both

    # pack listbox above button
    label $top.lab -text "Current directory: " -anchor w
    label $top.file -text "$tree" -anchor w
    entry $top.text -width 40 -relief sunken -textvariable filename -bg white
    frame $top.b
    if {$extrainp == "" } {
        button $top.b.button1 -text "OK" \
	    -command "call_next_proc $next_proc_name"
    } else {
        button $top.b.button1 -text "OK" \
            -command "call_next_proc $next_proc_name $extrainp"
    }
    button $top.b.button2 -text "Cancel" -command "destroy .query"
    pack $top.b.button1 $top.b.button2 -side left -fill x -expand 1
    pack $top.f1 $top.lab $top.file $top.text $top.b -fill x

    # Put files/ directories in listbox, files only if they have
    # extension $globext (the filter)
    cd $dir
    set files [glob -nocomplain .. *]
    # SORT LIST ALPHABETICALLY
    set files [lsort $files]
    foreach i $files {
	if [file isdirectory $i] {
	    $top.f1.list insert end $i
	} else {
	    if { [string compare $globext "*"]==0 } {
		$top.f1.list insert end $i
	    } else {
		if { [string compare [file extension $i] ".$ext"]==0 } {
		    $top.f1.list insert end $i
		}
	    }
	}
    }

    bind .query.f0.filter <Return> {
	# change the filer and hit enter: the files must be re-read for
	# the new extension:
	global globext
	set files [glob -nocomplain .. *]
	.query.f1.list delete 0 end
	foreach i $files {
	    if [file isdirectory $i] {
		.query.f1.list insert end $i
	    } else {
		if { [string compare $globext "*"]==0 } {
		    .query.f1.list insert end $i
		} else {
		    if { [string compare [file extension $i] ".$globext"]==0 } {
			.query.f1.list insert end $i
		    }
		}
	    }
	}
    }
    bind .query.f1.list <Double-Button-1> {
	# IF a direcotry is double-clicked, CD TO IT.
	global filename globext
	set dir [.query.f1.list get @%x,%y]
	if { [file isdirectory $dir] != 0 } {
	    # It's a directory, cd to it and redisplay.
	    cd $dir
	    set tree [pwd]
	    .query.file configure -text [pwd]
	    # redisplay the directory - first delete all entries
	    .query.f1.list delete 0 end
	    set files [glob -nocomplain .. *]
	    # SORT LIST ALPHABETICALLY
	    set files [lsort $files]
	    foreach i $files {
		if [file isdirectory $i] {
		    .query.f1.list insert end $i
		} else {
		    if { [string compare $globext "*"]==0 } {
			.query.f1.list insert end $i
		    } else {
			if { [string compare [file extension $i] \
				".$globext"]==0} {
			    .query.f1.list insert end $i
			}
		    }
		}
	    }
	}
    }

    bind .query.f1.list <Button-1> {
	# IF a file is single-clicked, PUT IT IN TEXT BOX.
	global filename
	set dir [.query.f1.list get @%x,%y]
	# $dir is the filename
	if { [file isdirectory $dir] == 0 } {
	    set tree [pwd]
	    set filename $dir
	}
    }

    bind .query.text <Return> {
	# If return is hit while in the entry box, a filename has been entered.
	global next_proc
        if {$extrainp==""} {
	  call_next_proc $next_proc
        } else {
          call_next_proc $next_proc $extrainp
        }
    }
}


proc call_next_proc { next_proc_name {extrainp ""}} {
    global filename tree globext

    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }

    # if the file has been entered entirely, split it up
    # ONLY FOR UNIX:
    if { [string compare [file dirname $filename] "."]!=0 } {
	# there is a tree in the filename
	if [string compare [string range $filename 0 2] ../]==0 {
	    # there's a ../ in the entered name, so must append pwd.
	    set tree [file join [pwd] [file dirname $filename]]
	} else {
	    set tree [file dirname $filename]
	}
	set filename [file tail $filename]
    }

    # if the file has been entered without an extension, add it on
    if { [string compare [file extension $filename] ""]==0 } {
	set filename "$filename.$globext"
    }

    destroy .query
    if {$extrainp==""} {
      $next_proc_name $tree $filename
    } else {
      $next_proc_name $tree $filename $extrainp
    }
}

proc update_phantfile {tree filename} {
    global PhantFileName medium values

    set PhantFileName [file join [pwd] $filename]
    # now read the top of this file to get the media used.
    set file [open $PhantFileName r]
    gets $file data
    set data [string trimleft $data]
    set values(2) [string trimright $data]
    for {set i 1} {$i<=$values(2)} {incr i} {
	gets $file data
	set medium($i) [string trimright $data]
    }
}
