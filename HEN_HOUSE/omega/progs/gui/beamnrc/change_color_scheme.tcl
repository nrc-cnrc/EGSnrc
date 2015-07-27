
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: change color scheme
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


proc change_color_scheme {} {
    global colorlist medium nmed helvfont smhelvfont colour colornum

    catch {destroy .colour}

    if [catch {expr $nmed}]==1 {
	# nmed hasn't been defined, so no pegs file has been loaded.
	tk_dialog .colourerror "Load media first" "Please load a PEGS4\
		data file to set the names of the media." error 0 OK
	return
    }

    toplevel .colour -bd 5
    set top .colour
    wm title $top "Change colours"
    wm geometry $top +200+300

    label $top.mess -text "Click on the colour swatch to change the colour\
	    of a material" -font $helvfont
    pack $top.mess -pady 10

    radiobutton $top.rcol -text "Colour" -variable colour -value 1 -command\
	    "change_to_colour"
    radiobutton $top.rgrey -text "Greyscale" -variable colour -value 0\
	    -command "change_to_colour"
    pack $top.rcol $top.rgrey -anchor w -pady 0

    frame $top.sep1 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep1 -fill x -pady 5

    frame $top.grid
    set w $top.grid
    set col 0
    set row 0
    for {set i 0} {$i<=$nmed} {incr i} {
	canvas $w.c$i -width 10 -height 10 \
            -bg [lindex $colorlist [min_nrc $i $colornum]]
	bind $w.c$i <Button-1> " change_color $w.c$i $i "
	label $w.l$i -text $medium($i) -font $smhelvfont
	grid config $w.c$i -row $row -column $col
	grid config $w.l$i -row $row -column [expr $col+1] -sticky w
	incr row
	if $row>10 {
	    incr col 2
	    set row 0
	}
    }
    pack $top.grid -pady 5

    frame $top.sep2 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep2 -fill x -pady 5

    button $top.but -text "Close" -command {destroy .colour} -relief groove\
	    -bd 8
    pack $top.but -pady 10
}

proc change_to_colour { } {
    global colorlist nmed colour default_colours colornum

    set col $colour

    if $col==1 {
	# use colorlist
	set colorlist $default_colours
    } else {
	set inc [expr int((65535.0-20000.0)/$nmed)]
	set greylist { #ffffffffffff }
	set glevel [expr 65535-$inc]
	for {set i 1} {$i<=$nmed} {incr i} {
	    set c [format "#%4.4x%4.4x%4.4x" $glevel $glevel $glevel]
	    set greylist [lappend greylist $c]
	    incr glevel -$inc
	}
	set colorlist $greylist
    }
    for {set i 0} {$i<=$nmed} {incr i} {
	.colour.grid.c$i configure \
         -bg [lindex $colorlist [min_nrc $i $colornum]]
    }
}


proc change_color { parent_w i } {
    global colorlist helvfont red green blue tempcolor colour colornum

    # get the rgb value for the colour selected
    set tempcolor [lindex $colorlist [min_nrc $i $colornum]]
    set colist [winfo rgb . $tempcolor]
    set red [lindex $colist 0]
    set green [lindex $colist 1]
    set blue [lindex $colist 2]

    catch {destroy $parent_w.select}

    toplevel $parent_w.select -bd 5
    set top $parent_w.select
    if {$i >= $colornum} {
       wm title $top "Not enough colours."
       wm geometry $top +250+300
       label $top.message -text "This material\
       is black because there aren't enough colours in the default_colours
       list in \$OMEGA_HOME/progs/gui/beamnrc/beamnrc_gui.
       You can't change this colour until you have added enough colours
       to the default_colours list to support this material."
       pack $top.message
       button $top.okb -text "OK" -command \
           "destroy $top" -relief groove -bd 8
       pack $top.okb
    } else {
       wm title $top "Select colour"
       wm geometry $top +250+300

       frame $top.frm
       frame $top.frm.left
       frame $top.frm.right

       set w $top.frm.left

       frame $w.grid
       if $colour==1 {
   	# use rgb colour scales
   	label $w.grid.red -text "Red" -font $helvfont
	label $w.grid.green -text "Green" -font $helvfont
	label $w.grid.blue -text "Blue" -font $helvfont
	grid config $w.grid.red -row 0 -column 0
	grid config $w.grid.green -row 1 -column 0
	grid config $w.grid.blue -row 2 -column 0
	scale $w.grid.sred -from 0 -to 65535 -length 300 -orient horizontal\
		-command "update_palette $parent_w red"
	scale $w.grid.sgreen -from 0 -to 65535 -length 300 -orient horizontal\
		-command "update_palette $parent_w green"
	scale $w.grid.sblue -from 0 -to 65535 -length 300 -orient horizontal\
		-command "update_palette $parent_w blue"
	$w.grid.sred set $red
	$w.grid.sgreen set $green
	$w.grid.sblue set $blue
	grid config $w.grid.sred -row 0 -column 1
	grid config $w.grid.sgreen -row 1 -column 1
	grid config $w.grid.sblue -row 2 -column 1
       } else {
	label $w.grid.grey -text "Grey level" -font $helvfont
	grid config $w.grid.grey -row 0 -column 0 -sticky w
	scale $w.grid.sgrey -from 0 -to 65535 -length 300 -orient horizontal\
		-command "update_palette $parent_w grey"
	$w.grid.sgrey set $red
	grid config $w.grid.sgrey -row 1 -column 0
       }
       pack $w.grid

       pack $w -side left

       set w $top.frm.right
       label $w.lab -text "Selected colour:" -font $helvfont
       pack $w.lab -anchor w
       entry $w.ent -textvariable tempcolor -bg white -state disabled
       pack $w.ent -anchor w
       canvas $w.can -width 100 -height 100
       pack $w.can -fill both -expand true -pady 10

       pack $w

       pack $top.frm -pady 5

       frame $top.buts
       button $top.buts.accept -text "Accept" -relief groove -bd 8 -command\
	    "set_color $parent_w $i; destroy $parent_w.select"
       button $top.buts.cancel -text "Cancel" -relief groove -bd 8 -command\
	    "destroy $parent_w.select"
       pack $top.buts.accept $top.buts.cancel -side left -padx 10
       pack $top.buts -pady 10
   }
}

proc update_palette {parent_w col value} {
    global red green blue tempcolor colour

    switch $col {
	red { set red $value }
	green { set green $value }
	blue { set blue $value }
	grey { set red $value }
    }

    if $colour==1 {
	set tempcolor [format "#%4.4x%4.4x%4.4x" $red $green $blue]
    } else {
	set tempcolor [format "#%4.4x%4.4x%4.4x" $red $red $red]
    }
    $parent_w.select.frm.right.can configure -bg $tempcolor
}

proc set_color { parent_w i } {
    global tempcolor colorlist

    set colorlist [lrm $colorlist $i $tempcolor]
    $parent_w configure -bg $tempcolor
}

proc lrm { list pos item } {
    if {$pos >= 0} {
	return [lreplace $list $pos $pos $item]
    }
    return $list
}



