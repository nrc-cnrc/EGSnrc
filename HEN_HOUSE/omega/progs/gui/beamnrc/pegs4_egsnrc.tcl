
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: pegs4 functions
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


proc get_pegs4file { } {
    global pegs4filename helvfont env pegs_from_home egs_home hen_house
    global is_pegsless

    catch {destroy .pegs4}

    toplevel .pegs4 -bd 5
    wm title .pegs4 "Enter PEGS4 filename"

    set width [winfo screenwidth .]
    set height [winfo screenheight .]
    set x [expr ($width/2) - 450]
    set y [expr ($height/2) -150]
    wm geometry .pegs4 +$x+$y

    set text "Enter the name of the PEGS4 cross-section data file to use by\
	    browsing your home directory tree or the HEN_HOUSE.\
	    This is essential for defining the materials in the simulation.\
            If you choose pegsless mode, then your media definitions must appear\
            in the .egsinp file."
    message .pegs4.mess -text $text -font $helvfont -width 600
    pack .pegs4.mess -pady 15

    frame .pegs4.frm

    frame .pegs4.frm.file
    label .pegs4.frm.file.lab -text {} -font $helvfont
    entry .pegs4.frm.file.inp -textvariable pegs4filename -width 40 -bg white
    pack .pegs4.frm.file.lab -anchor w -padx 5 -pady 5
    pack .pegs4.frm.file.inp  -side left -anchor w -fill x -padx 5 -expand true

    frame .pegs4.frm.bb
    set local_pegs_area [file join $egs_home pegs4 data]
    set main_pegs_area [file join $hen_house pegs4 data]
    button .pegs4.frm.bb.home -text "Browse local area" -command\
	    "set is_pegsless 0; set pegs_from_home 1;\
	    query_filename set_pegs4filename $local_pegs_area pegs4dat"
    button .pegs4.frm.bb.hen -text "Browse HEN_HOUSE" -command\
	    "set is_pegsless 0; set pegs_from_home 0; query_filename set_pegs4filename\
	    $main_pegs_area pegs4dat"
    button .pegs4.frm.bb.pegsless -text "Go PEGSless" -command\
            {set is_pegsless 1; set medium(0) "VACUUM"; read_matfile; add_inp_media; destroy .pegs4}
    pack .pegs4.frm.bb.home .pegs4.frm.bb.hen .pegs4.frm.bb.pegsless -fill x

    pack .pegs4.frm.file -side left -anchor w -fill x -padx 5 -expand true
    pack .pegs4.frm.bb -side right -padx 5 -anchor e
    pack .pegs4.frm -fill x

    frame .pegs4.but
    button .pegs4.but.okb -text "OK" -command\
	    "check_pegs4file" -relief groove -bd 8
    button .pegs4.but.cb -text "Cancel" -command\
	    {set result [tk_dialog .warning "Warning" "If no PEGS4 data is\
	    loaded, the GUI will not be able to assign materials in your\
	    simulation.  Are you sure?" warning 1 Yes No]; if $result==0 {\
	    destroy .pegs4 } } -relief groove -bd 8
    pack .pegs4.but.okb .pegs4.but.cb -padx 10 -side left
    pack .pegs4.but -pady 10

    update
    focus .pegs4.frm.file.inp
}

proc set_pegs4filename { tree filename } {
    global pegs4filename pegs_from_home env egs_home hen_house
    set pegs4filename [file join $tree $filename]
    if [file exists $pegs4filename]==0 {
	tk_dialog .error "Error" "$pegs4filename doesn't exist!  Please try\
		again." error 0 OK
	return
    }
    if $pegs_from_home==0 {
	set text "Directory: [file join $hen_house pegs4 data]"
    } else {
	set text "Directory: [file join $egs_home pegs4 data]"
    }
    .pegs4.frm.file.lab configure -text $text
}

proc check_pegs4file {} {
    global pegs4filename num_cm env pegs_from_home nmed medium helvfont
    global colorlist smhelvfont colornum egs_home hen_house default_colours

    # if the file selected is in the henhouse, check if the same file is
    # also in egs_home/pegs4/data.  If so, check=0 (bad).

    set check 1
    if $pegs_from_home==0 {
       set local_pegs_area [file join $egs_home pegs4 data]
	if [file exists $local_pegs_area]==1 {
	    cd $local_pegs_area
	    set files [glob -nocomplain *.pegs4dat]
	    foreach i $files {
		if [string compare $i [file tail $pegs4filename]]==0 {
		    set check 0
		}
	    }
	}
    }

    if $check==1 {

	# put on a small window with a message letting the user know it's
	# loading the pegs4data file.
	toplevel .pegsmessage -bd 0

	set width [winfo screenwidth .]
	set height [winfo screenheight .]
	set x [expr ($width/2) - 200]
	set y [expr ($height/2) -100]
	wm geometry .pegsmessage +$x+$y
	wm transient .pegsmessage .

	label .pegsmessage.m -text "Loading PEGS4 data..." -font $helvfont
	pack .pegsmessage.m -padx 20 -pady 20
	update

	# ACTIVATE THE EDIT BUTTONS if necessary
	if $num_cm>0 {
	    .cm_selected.f1.mainb configure -state normal
	    for {set i 1} {$i<=$num_cm} {incr i} {
		.cm_selected.f1.cm($i).editb configure -state normal
	    }
	}
	destroy .pegs4
	if $pegs_from_home==0 {
	    set text "Using PEGS4 file\
	   [file join $hen_house pegs4 data [file tail $pegs4filename]]."
	} else {
	    set text "Using PEGS4 file\
		    [file join $egs_home pegs4 data [file tail $pegs4filename]]."
	}
	.pegs4label configure -text $text
	# Now read in the materials from the file:
	if [file readable $pegs4filename] {
	    set file [open $pegs4filename r]
	    set medium(0) "VACUUM"
	    set i 1
	    while { [eof $file] != 1 } {
		gets $file data
		if [string compare "MEDIUM" [string range $data 1 6]]==0 {
		    # Read the medium name to before the location of the
		    # first comma on the line
		    set medium($i) [string range $data 8 \
			    [expr [string first , $data]-1]]
		    set medium($i) [string trimright $medium($i)]
		    incr i
		}
	    }
	    close $file
	    set nmed [expr $i-1]
            set colorlist $default_colours
            #colorlist might have been redefined
            set colornum [expr [llength $colorlist]-1]
            if {$nmed>=$colornum} {
                 #we do not have enough colours to display all media
                 toplevel .pegs4noofcolors -bd 0 -class Dialog
                 wm title .pegs4noofcolors "WARNING: Not enough colors"
                 wm transient .pegs4noofcolors .
                 label .pegs4noofcolors.topmsg -text "There are not enough colours to display the following pegs4 media:" -font $helvfont -bd 5
                 pack .pegs4noofcolors.topmsg
                 for {set i $colornum} {$i<=$nmed} {incr i} {
                    label .pegs4noofcolors.$i -text $medium($i) \
                          -font $helvfont
                    pack .pegs4noofcolors.$i
                 }
                 label .pegs4noofcolors.botmsg -text "These media will appear \
black.  Add new colours to the default colours
list defined in \$OMEGA_HOME/progs/gui/beamnrc/beamnrc_gui (before `black').
Use `showrgb' for a list of colours available on your system."\
                     -font $helvfont -bd 5
                 pack .pegs4noofcolors.botmsg
                 button .pegs4noofcolors.okb -text "OK" -command \
                        "destroy .pegs4noofcolors" -relief groove -bd 8
                 pack .pegs4noofcolors.okb
                 tkwait window .pegs4noofcolors
            }
	    destroy .pegsmessage
	}
    } else {
	tk_dialog .pegs4.no "Conflict" "This file also exists in your home\
		directory tree, which BEAM will select first.  Please\
		select the file in your home directory." warning 0 OK
    }


}

