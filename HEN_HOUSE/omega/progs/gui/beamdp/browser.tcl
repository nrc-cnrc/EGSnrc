
###############################################################################
#
#  EGSnrc BEAMnrc beamdp graphical user interface: file browsing
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
#  Author:          Joanne Treurniet, 1999
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


proc browse { dir next_proc_name flag labeltext } {

    # pops up a file browser with no filter, calling next_proc_name
    # when OK is clicked.
    # If a phase space file is needed, filter for *egs4phsp* and
    # *egsphsp* (flag=0)
    # Otherwise no filter (flag=1).

    global filename tree rootx rooty bflag
    set top .query
    toplevel $top

    set bflag $flag
    set filename {}
    set tree $dir
    wm title .query "Select file"
    set x [expr $rootx + 50]
    set y [expr $rooty + 50]
    wm geometry .query +$x+$y

    label $top.label -text $labeltext
    pack $top.label -pady 10 -padx 5 -anchor w

    frame $top.f1
    # add a scrollbar
    scrollbar $top.f1.scrollit -command "$top.f1.list yview"

    # create a listbox
    listbox $top.f1.list -yscroll "$top.f1.scrollit set" -relief groove\
	    -bg white
    pack $top.f1.scrollit -side right -fill y
    pack $top.f1.list -side left -expand 1 -fill both

    # pack listbox above OK, cancel buttons
    label $top.lab -text "Current directory: " -anchor w
    label $top.file -text "$tree" -anchor w
    entry $top.text -width 40 -relief sunken -textvariable filename -bg white
    frame $top.b
    button $top.b.button1 -text "OK" \
	    -command "destroy .query; $next_proc_name"
    button $top.b.button2 -text "Cancel" -command "destroy .query"
    pack $top.b.button1 $top.b.button2 -side left -fill x -expand 1
    pack $top.f1 $top.lab $top.file $top.text $top.b -fill x

    # insert files, directories in $dir into listbox
    cd $dir
    set files [glob -nocomplain .. *]
    # SORT LIST ALPHABETICALLY
    set files [lsort $files]
    foreach i $files {
	if [file isdirectory $i] {
	    $top.f1.list insert end $i
	} else {
	    if $flag==1 {
		$top.f1.list insert end $i
	    } else {
                if [string match "*egsphsp*" $i]==1 {
                    $top.f1.list insert end $i
                }
		if [string match "*egs4phsp*" $i]==1 {
		    $top.f1.list insert end $i
		}
                if [string match "*IAEAphsp*" $i]==1 {
                    $top.f1.list insert end $i
                }
	    }
	}
    }

    bind .query.f1.list <Double-Button-1> {
	# IF a directory is double-clicked, CD TO IT.
	global filename bflag
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
		    if $bflag==1 {
			.query.f1.list insert end $i
		    } else {
                        if [string match "*egsphsp*" $i]==1 {
                            .query.f1.list insert end $i
                        }
			if [string match "*egs4phsp*" $i]==1 {
			    .query.f1.list insert end $i
			}
		    }
		}
	    }
	}
    }

    bind .query.f1.list <Button-1> {
	# IF a FILE is single-clicked, PUT it in the TEXT BOX.
	global filename
	set dir [.query.f1.list get @%x,%y]
	# $dir is the filename
	if { [file isdirectory $dir] == 0 } {
	    set tree [pwd]
	    set filename $dir
	}
    }
}

proc set_spcnam { } {
    global filename spcnam
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set spcnam [file join [pwd] $filename]
}

proc set_phspfile { } {
    global filename phspfile start_dir
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set phspfile [file join [pwd] $filename]
    set start_dir [pwd]
}

proc set_phspfile2 { } {
    global filename phspfile2
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set phspfile2 [file join [pwd] $filename]
}

proc set_srcfilename { } {
    global filename srcfilename browservar
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set srcfilename [file join [pwd] $filename]
    set browservar 1
}

proc set_newsrcfilename { } {
    global filename newsrcfilename
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set newsrcfilename [file join [pwd] $filename]
}

proc set_xmgrfile { } {
    global filename xmgrfile
    # if nothing was entered, return
    if { [string compare $filename ""]==0 } {
	tk_dialog .query.empty "Empty string" "No filename was entered.  \
		Please try again." error 0 OK
	return
    }
    set xmgrfile [file join [pwd] $filename]
}

# The following were taken from Blake Walters' QAtool.

proc startjob { } {
    global pid omega hen_house
    set pid [exec beamdp < beamdpguitemp.script > temp &]
}

proc displaydialg {args} {
    set tailfile [ open [lindex "$args" 0]]
    .run.frm.dialg insert end "[read $tailfile]"
    updatetail $tailfile
    return
}

proc updatetail {filnam} {
    global waitvar errvar
    set lines [list [read $filnam] ]
    foreach line $lines {
        catch {
	    .run.frm.dialg insert end $line
	    .run.frm.dialg see end
	}
        if [regexp "BYE!" $line] {
            set waitvar 1
	    set errvar 0
            close $filnam
            return
        }
        if [regexp "Abort" $line] {
            set waitvar 1
	    set errvar 1
            close $filnam
            return
        }
        if [regexp "!!!!!!" $line] {
            set waitvar 1
	    set errvar 1
            close $filnam
            return
        }
    }
    after 1000 [list updatetail $filnam]
}

