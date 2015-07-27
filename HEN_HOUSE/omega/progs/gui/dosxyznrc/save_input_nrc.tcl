
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: save input
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


proc save_input {} {

    # query_filename with DoesInpFileExist as the next procedure.

    global inp_file_dir

    query_filename DoesInpFileExist $inp_file_dir egsinp
}


proc DoesInpFileExist { tree filename } {

    # see if the input file exists already and prompt for overwrite.

    global globext
    if { [string compare [file extension $filename] ""]==0 } {
	# THE FILENAME HAS BEEN INPUT WITHOUT AN EXTENSION.  ADD
	# THE FILTER BOX EXTENSION TO IT.
	set filename "$filename.$globext"
    }
    if { [file exists $tree/$filename]==1 } {
	if { [file isdirectory $tree/$filename]!=1 } {
	    overwrite_prompt $tree $filename
	} else {
	    tk_dialog .save_inp.nope "Nuh-uh" "Please specify a file." \
		    warning 0 OK
	}
    } else {
	save_inp_file $tree $filename
    }
}

proc save_inp_file {tree filen} {

    # set the new input filename and call create_file to write it.

    global inp_file inp_base inp_file_dir debug

    set oldfile $inp_file

    set inp_file [file join $tree $filen]
    set inp_base [file rootname $filen]

    .inploaded configure -text "Input parameters $inp_base loaded"

    create_file $inp_file

    # Enable the Edit Parameters option on the main menubar by destroying and
    # re-creating it.
    destroy .mbar.file.menu
    menu .mbar.file.menu
    .mbar.file.menu add command -label "Start a new input file" \
	    -command "reset_parameters; edit_parameters"
    .mbar.file.menu add command -label "Load a previous input file" \
	    -command "load_input"
    .mbar.file.menu add command -label "Edit parameters"\
	    -command "edit_parameters"
    .mbar.file.menu add command -label "Save input parameters" \
            -command "overwrite_input"
    .mbar.file.menu add command -label "Save input parameters as..." \
	    -command "save_input"
    .mbar.file.menu add separator
    .mbar.file.menu add command -label "Change PEGS4 file" \
	    -command "get_pegs4file"
    .mbar.file.menu add separator
    .mbar.file.menu add command -label "Exit" -command "exit_prompt"

    if $debug==1 { exec tkdiff $oldfile $inp_file }
}

proc overwrite_prompt {tree filename} {

    # ask the user if they want to overwrite the file they've selected.

    set result [tk_dialog .overwrite "Overwrite?" \
	    "The file already exists.  \
	    Overwrite?" question 0 Yes No  ]

    if $result==0 {
	# YES, overwrite
	save_inp_file $tree $filename
    }
}

proc overwrite_input {} {

    # This is the "Save" command as opposed to the "Save as.." command
    # for the input parameters, so we don't ask if the user wants to
    # overwrite or not, of course they do, right?

    global inp_file

    if { $inp_file=="" } {
      tk_dialog .need_file "Warning" \
                "You need to load a previous input file first."\
                warning 0 OK
    } else {
      create_file $inp_file
    }
}

