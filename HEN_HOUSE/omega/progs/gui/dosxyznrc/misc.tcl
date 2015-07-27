
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: miscellaneous functions
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
#  Contributors:    Iwan Kawrakow
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


proc help_dialog {w title text bitmap default args} {

    # help dialog box to put text in a scrolling window.

    global helvfont
    toplevel $w
    wm title $w "Help"

    frame $w.msg -bd 5

    # find the height required for the text:
    # 60 chars/line, max of 30 lines, min of 5 lines
    set len [string length $text]
    set height [expr $len/60 + 5]
    if $height>30 {set height 30}
    if $height<5 {set height 5}


    # text box for help text (white background):
    text $w.msg.text -font $helvfont -width 60 -height $height -wrap word\
	-relief sunken -bd 5 -yscrollcommand "$w.msg.v_scroll set" -bg white
    $w.msg.text insert 1.0 $text
    # no editing allowed:
    $w.msg.text configure -state disabled

    # vertical scrollbar:
    scrollbar $w.msg.v_scroll -command "$w.msg.text yview"

    pack $w.msg.text -side left
    pack $w.msg.v_scroll -side right -fill y
    pack $w.msg -side top

    frame $w.buts -bd 4
    button $w.buts.okb -text "OK" -command "destroy $w" \
	    -relief groove -bd 8
    pack $w.buts.okb -side left -padx 10
    pack $w.buts -pady 10
}

proc help_gif { w text iconfile } {

    # THIS DIALOG IS FOR HELP WHEN A GIF FILE IS USED AS AN ICON
    # THE ICON IS LEFT, TEXT IS RIGHT.

    global helvfont XYZ_DIR
    toplevel $w -bd 8
    wm title $w "Help"
    wm geometry $w +100+100

    frame $w.f1 -bd 0 -relief sunken -bg white
    # Create the image from a gif file:
    if {$iconfile != ""} {
	image create photo graphic -file \
		[file join $XYZ_DIR graphics $iconfile.gif]
	label $w.f1.lbl -image graphic -bg white
    } else {
	label $w.f1.lbl -text "No graphic for this source"
    }
    #    message $w.f1.m -width 5i -text $text -font $helvfont -bg white

    # put the text in a yscroll window frame
    frame $w.f1.m -bg white

    set height 30

    # text box for help text (white background):
    text $w.f1.m.text -font $helvfont -width 60 -height $height -wrap word\
	 -bd 5 -yscrollcommand "$w.f1.m.v_scroll set" -bg white
    $w.f1.m.text insert 1.0 $text
    # no editing allowed:
    $w.f1.m.text configure -state disabled

    # vertical scrollbar:
    scrollbar $w.f1.m.v_scroll -command "$w.f1.m.text yview" -bg white

    pack $w.f1.m.text -side left
    pack $w.f1.m.v_scroll -side right -fill y

    pack $w.f1.lbl $w.f1.m -side left -padx 10 -pady 5 -fill both

    pack $w.f1
    frame $w.sep -bd 4 -width 100 -height 2 -relief groove
    pack $w.sep -pady 10 -fill x
    button $w.okb -text "OK" -command "destroy $w" \
	    -relief groove -bd 8
    pack $w.okb -pady 10
}

proc tk_dialog {w title text bitmap default args} {

    # All so that I can make the windows wider so the text doesn't get
    # squished and so I can make the text helvetica instead of times.
    # Taken directly from dialog.tcl in the /usr/lib/tk4.1 directory.
    # I have removed comments to conserve space.  See the original file.

    global tkPriv helvfont

    catch {destroy $w}
    toplevel $w -class Dialog
    wm title $w $title
    wm iconname $w Dialog
    wm protocol $w WM_DELETE_WINDOW { }
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both -expand 1
    # THIS IS THE PLACE TO CHANGE THE WIDTH:
    option add *Dialog.msg.wrapLength 5i widgetDefault
    label $w.msg -justify left -text $text
    # THIS IS THE PLACE TO CHANGE THE FONT:
    catch { $w.msg configure -font $helvfont }
    pack $w.msg -in $w.top -side right -expand 1 -fill both -padx 3m -pady 3m
    if {$bitmap != ""} {
	label $w.bitmap -bitmap $bitmap
	pack $w.bitmap -in $w.top -side left -padx 3m -pady 3m
    }
    set i 0
    foreach but $args {
	button $w.button$i -text $but -command "set tkPriv(button) $i"
	if {$i == $default} {
	    frame $w.default -relief sunken -bd 1
	    raise $w.button$i $w.default
	    pack $w.default -in $w.bot -side left -expand 1 -padx 3m -pady 2m
	    pack $w.button$i -in $w.default -padx 2m -pady 2m
	} else {
	    pack $w.button$i -in $w.bot -side left -expand 1 \
		    -padx 3m -pady 2m
	}
	incr i
    }
    if {$default >= 0} {
	bind $w <Return> "
	    $w.button$default configure -state active -relief sunken
	    update idletasks
	    after 100
	    set tkPriv(button) $default
	"
    }
    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w
    set oldFocus [focus]
    set oldGrab [grab current $w]
    if {$oldGrab != ""} {
	set grabStatus [grab status $oldGrab]
    }
    grab $w
    if {$default >= 0} {
	focus $w.button$default
    } else {
	focus $w
    }
    tkwait variable tkPriv(button)
    catch {focus $oldFocus}
    destroy $w
    if {$oldGrab != ""} {
	if {$grabStatus == "global"} {
	    grab -global $oldGrab
	} else {
	    grab $oldGrab
	}
    }
    return $tkPriv(button)
}

proc max_nrc { x1 x2 } {
    if { $x1 < $x2 } {
      set y $x2
    } else {
      set y $x1
    }
    return $y
}

proc min_nrc { x1 x2 } {
    if { $x1 > $x2 } {
      set y $x2
    } else {
      set y $x1
    }
    return $y
}
