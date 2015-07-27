
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: load module
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
#                   Frederic Tessier
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


proc load_old_module {} {
    # if an accel. is loaded, as if it's OK to discard it and if so call the
    # browser.  Else just call the browser.

    global num_cm home mod_file maxvals values GUI_DIR egs_home

    set dir [file join $egs_home beamnrc spec_modules]
    if [file exists $dir]==0 {
	# There was no beamnrc/spec_modules directory
        # Give a warning and start at $EGS_HOME
        tk_dialog .error "Warning" "Could not find directory\
              $egs_home/beamnrc/spec_modules.  Accelerator modules should\
              be in this directory.  Check your BEAM installation.  Browsing\
              will start in $egs_home for now." error 0 OK
	set dir $egs_home
    }

    if {$num_cm>0} {
	set result [tk_dialog .okay "Discard?" "The accelerator\
		currently loaded will be discarded.  Are you sure?" \
		question 0 Yes Cancel ]

	if $result==0 {
	    # YES, discard
            source $GUI_DIR/beamnrc_params.tcl
	    destroy .cm_selected
	    set num_cm 0
	    set mod_file {}
	    .modloaded configure -text {}
	    query_filename set_mod_file $dir module
	}
    } else {
	query_filename set_mod_file $dir module
    }
    # query_filename get a tree and filename to send to set_mod_file...
}

proc set_mod_file {tree filen} {
    # set the module filename and input file directory corresponding to it.
    # then load module.

    global home mod_file mod_base inp_file_dir egs_home

    set mod_file [file join $tree $filen]
    set mod_base [file rootname $filen]
    set inp_file_dir [file join $egs_home BEAM_$mod_base]
    #set inp_file_dir $home/egsnrc/BEAM_$mod_base
    load_module
}

proc load_module {} {
    # read in a module file "mod_file"

    global cm_names cm_type cm_ident num_cm mod_file mod_base pegs4filename
    global maxvals

    # FIRST FIND OUT IF THERE'S ALREADY ONE LOADED, if so destroy it.
    catch { destroy .cm_selected }
    # OPEN THE MODULE FILE
    set data ""
    if { [file readable $mod_file] } {
	set fd [open $mod_file r]
	gets $fd data1
	gets $fd data2
	close $fd
    } else {
	tk_dialog .okay "Sorry, try again..." \
		"The accelerator file $mod_base.module doesn't exist!  \
		You can create this file by selecting 'Specify a new\
		accelerator' on the File menu, or if you would like to\
		reuse an accelerator by a different name, you can copy it\
		to $mod_base.module in your $egs_home/beamnrc/spec_modules directory." \
		warning 0 OK
	return 0
    }

    if [string first CM $data1]==-1 {
	tk_dialog .badmodule "Bad module" "The module file is corrupt.  \
		Please fix it and try again." error 0 OK
	return 0
    }
    if [string first Identifiers $data2]==-1 {
	tk_dialog .badmodule "Bad module" "The module file is corrupt.  \
		Please fix it and try again." error 0 OK
	return 0
    }

    # data1 has the first line of the file.  Split it up into CMs.
    # remove the first bit up to the :
    set indx1 [string first : $data1]
    incr indx1
    set indx2 [string first : $data2]
    incr indx2

    set data1 [string range $data1 $indx1 [string length $data1]]
    set data2 [string range $data2 $indx2 [string length $data2]]
    set data1 [string trimleft $data1]
    set data2 [string trimleft $data2]
    set i 0
    while { [string length $data1] > 0 } {
	set indx [string wordend $data1 1]
	set str [string trimleft [string range $data1 0 $indx]]
	set str [string trimright $str]
	incr indx
	set data1 [string range $data1 $indx [string length $data1]]
	set found 0
	for {set j 1} {$j <= 25} {incr j} {
	    # look for a match in cm_names to get cm_type
	    if [string compare $cm_names($j) $str]==0 {
		set found 1
		incr num_cm
		incr i
		set cm_type($i) $j
	    }
	}
	if $found==0 {
	    tk_dialog .error "Bad Component" "The CM listed in the module file\
		    as $str is not used in BEAMnrc.  This\
		    is a fatal error!" error 0 OK
	    return
	}
    }
    set i 0
    while { [string length $data2] > 0 } {
	# GET IDENTIFIERS
	incr i
        set data2 [string trimleft $data2]
	set indx [string wordend $data2 0]
	set str [string range $data2 0 $indx]
	incr indx
	set data2 [string range $data2 $indx [string length $data2]]
	set cm_ident($i) $str
    }

    # Check that the number of CMs in this file is less than MAX_CMs
    if $num_cm>$maxvals(MAX_CMs) {
	tk_dialog .toobig "Too many CMs" "The number of CMs is this accelerator\
	exceeds the maximum set in the file beamnrc_user_macros.mortran,\
		currently set to $maxvals(MAX_CMs)." info 0 OK
    }

    # put the cm_selected window up:
    popup_cm_window

    # update it with what was just loaded:
    update_popup

    # before enabling the CM buttons, input pegs4filename if necessary
    if [string compare $pegs4filename {}]==0 {
	get_pegs4file
	# wait for the user to finish loading a pegs4file before continuing
	tkwait window .pegs4
    }

    # ACTIVATE THE EDIT BUTTONS if necessary
    if $num_cm>0 {
	.cm_selected.f1.mainb configure -state normal
	for {set i 1} {$i<=$num_cm} {incr i} {
	    .cm_selected.f1.cm($i).editb configure -state normal
	}
    }

    # Initialize the parameters for each of the CMs loaded (in case of overlap
    # from a previous loaded input file).
    for {set i 1} {$i<=$num_cm} {incr i} {
	set index $cm_type($i)
        init_$cm_names($index) $i
    }

    .modloaded configure -text "Accelerator $mod_base loaded"

    return 1

}
