
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: save module
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
#  Portions of this code were derived from the Quality Assurance tool written
#  by Blake Walters
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


proc save_module {} {
    # call query_filename with DoesModFileExist as the next procedure.
    global egs_home
    set tree [file join $egs_home beamnrc spec_modules]
    query_filename DoesModFileExist $tree module
}

proc DoesModFileExist { tree filename } {
    # check whether the module file exists and if so prompt for overwrite, else
    # save_mod_file
    global globext
    if { [string compare [file extension $filename] ""]==0 } {
	# THE FILENAME HAS BEEN INPUT WITHOUT AN EXTENSION.  ADD
	# THE FILTER BOX EXTENSION TO IT.
	set filename "$filename.$globext"
    }
    if [file exists $tree/$filename]==1 {
	overwrite_prompt $tree $filename save_mod_file mod
    } else {
	save_mod_file $tree $filename
    }
}

proc save_mod_file {tree filen} {

    # set the module filename and inp_file_dir which it should correspond to.
    # Write the file, then activate the edit buttons on .cm_selected.

    global mod_file mod_base cm_names cm_ident home num_cm inp_file_dir
    global cm_type pegs4filename egs_home

    set mod_file [file join $tree $filen]
    set mod_base [file rootname $filen]
    set inp_file_dir [file join $egs_home BEAM_$mod_base]
    .modloaded configure -text "Accelerator $mod_base loaded"

    set str1 " CM names:  "
    set str2 " Identifiers:  "
    for {set i 1} {$i<=$num_cm} {incr i} {
	set index $cm_type($i)
	set str1 "$str1$cm_names($index) "
	set str2 "$str2$cm_ident($i) "
    }
    set id [open $mod_file w]
    puts $id $str1
    puts $id $str2
    close $id

    if [string compare $pegs4filename {}]==0 {
	# before activating the edit buttons, get a peg4filename
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

    for {set i 1} {$i<=$num_cm} {incr i} {
	set index $cm_type($i)
        init_$cm_names($index) $i
    }
    destroy .select
}

proc overwrite_prompt {tree filename save_proc type} {

    # ask the user if they want to overwrite the file they've selected.

    global ${type}_file GUI_DIR
    set result [tk_dialog .overwrite "Overwrite?" \
	    "The file already exists.  \
	    Overwrite?" question 0 Yes No  ]

    if $result==0 {
	# YES, overwrite
	$save_proc $tree $filename
    }
}



