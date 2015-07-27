
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: file browser
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


proc browse { next_proc_name dir } {

    # pops up a file browser with no filter, calling next_proc_name
    # when OK is clicked.

    global home filename tree rootx rooty helvfont
    set top .query
    toplevel $top

    set filename {}
    set tree $dir
    wm title .query "Select file"
    set x [expr $rootx + 50]
    set y [expr $rooty + 50]
    wm geometry .query +$x+$y

    frame $top.f1
    # add a scrollbar
    scrollbar $top.f1.scrollit -command "$top.f1.list yview"

    # create a listbox
    listbox $top.f1.list -yscroll "$top.f1.scrollit set" -relief groove\
	    -bg white -font $helvfont
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
	$top.f1.list insert end $i
    }

    bind .query.f1.list <Double-Button-1> {
	# IF a directory is double-clicked, CD TO IT.
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
		.query.f1.list insert end $i
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

proc set_spec_file { } {
    global filename spec_file
    set spec_file [file join [pwd] $filename]
}

proc set_beam_code { tree filename } {
    # for source 9, called from query_filename
    global the_beam_code env tcl_platform
    if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
    } else {
                  set libext so
    }
    set the_beam_code $filename
    set len [string length $the_beam_code]
    set BEAMindex [string first BEAM $the_beam_code]
    if $BEAMindex<0 {set BEAMindex 0}
    set extindex [expr [string last .$libext $the_beam_code]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_beam_code [string range $the_beam_code $BEAMindex $extindex]
    set beamdir [file join $env(EGS_HOME) $the_beam_code]
    if {[file exists $beamdir]} {
       .main_w.srcopt.files.inpfilebrowse configure \
         -text "Browse EGS_HOME/$the_beam_code" \
         -command "query_filename set_input_file $beamdir egsinp" \
         -state normal
    } else {
       set egs_home $env(EGS_HOME)
       tk_dialog .acceldirexist "Warning" "Directory\
                    EGS_HOME/$the_beam_code does not exist.  Input files\
                    must be in this directory.  Will browse from\
                    EGS_HOME." error 0 OK
       .main_w.srcopt.files.inpfilebrowse configure \
         -text "Browse EGS_HOME" \
         -command "query_filename set_input_file $egs_home egsinp"
    }
}

proc set_vcu_code { tree filename } {
    # for source 20 and 21 called from query_filename
    global the_vcu_code env tcl_platform
    if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
    } else {
                  set libext so
    }
    set egs_home $env(EGS_HOME)
    set the_vcu_code $filename
    set BEAMflag 0
    set len [string length $the_vcu_code]
    set VCUindex [string first particleDmlc $the_vcu_code]
    if { $VCUindex<=0 } {
	set VCUindex [string first BEAM $the_vcu_code]
	if { $VCUindex<=0 } {
	    set VCUindex 0
	} else {
	    set BEAMflag 1
	}
    }
    set extindex [expr [string last .$libext $the_vcu_code]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_vcu_code [string range $the_vcu_code $VCUindex $extindex]

    if { $BEAMflag == 0 } {
    .main_w.srcopt.files.vcusimlab configure \
        -text "External library:"
	.main_w.srcopt.files.inpfilevculab configure \
	    -text "External library input file (full path):"
	.main_w.srcopt.files.inpfilevcubrowse configure \
	    -text "Browse EGS_HOME" \
	    -command "browse set_vcu_input_file $egs_home"
    } else {
	set beamdir [file join $egs_home $the_vcu_code]
	if {[file exists $beamdir]} {
	    .main_w.srcopt.files.vcusimlab configure \
        -text "BEAMnrc library:"
	    .main_w.srcopt.files.inpfilevculab configure \
		-text "BEAMnrc library input file:"
	    .main_w.srcopt.files.inpfilevcubrowse configure \
		-text "Browse EGS_HOME/$the_vcu_code" \
		-command "query_filename set_beam_input_file $beamdir egsinp"
	} else {
	    tk_dialog .acceldirexist "Warning" "Directory\
                    EGS_HOME/$the_beam_code does not exist.  Input files\
                    must be in this directory.  Will browse from\
                    EGS_HOME." error 0 OK
	    .main_w.srcopt.files.inpfilevcubrowse configure \
		-text "Browse EGS_HOME" \
		-command "query_filename set_input_file $egs_home egsinp"
	}
    }

}

proc set_input_file { tree filename } {
    #for source 9, called from query_filename
    global the_input_file
    set the_input_file $filename
    set len [string length $the_input_file]
    set extindex [expr [string last .egsinp $the_input_file]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_input_file [string range $the_input_file 0 $extindex]
}

proc set_vcu_input_file { } {
    # for source 20,21 called from query_filename
    global filename the_vcu_input_file
    set the_vcu_input_file [file join [pwd] $filename]
}

proc set_beam_input_file { tree filename } {
    # for source 20,21 called from query_filename for beam not vcu lib input file
    global the_vcu_input_file
    set the_vcu_input_file $filename
    set len [string length $the_vcu_input_file]
    set extindex [expr [string last .egsinp $the_vcu_input_file]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_vcu_input_file [string range $the_vcu_input_file 0 $extindex]
}

proc set_pegs_file { tree filename } {
    #for source 9, called from query_filename
    global the_pegs_file
    set the_pegs_file $filename
    set len [string length $the_pegs_file]
    set extindex [expr [string last .pegs4dat $the_pegs_file]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_pegs_file [string range $the_pegs_file 0 $extindex]
}
proc set_phsp_file { } {
    # for source 20 called from query_filename
    global the_phsp_file filename
    set the_phsp_file [file join [pwd] $filename]
}