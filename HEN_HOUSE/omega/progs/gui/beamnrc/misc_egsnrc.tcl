
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: miscellaneous functions
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

proc help_dialog {w title text bitmap default args} {

    # help dialog box to put text in a scrolling window.

    global helvfont
    toplevel $w
    wm title $w "Help"

    frame $w.msg -bd 5

    # find the height required for the text:
    # 80 chars/line, max of 30 lines, min of 5 lines
    set len [string length $text]
    set height [expr $len/80 + 5]
    if $height>30 {set height 30}
    if $height<5 {set height 5}


    # text box for help text (white background):
    text $w.msg.text -font $helvfont -width 80 -height $height -wrap word\
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

    global helvfont GUI_DIR
    toplevel $w -bd 8
    wm title $w "Help"
    frame $w.f1 -bd 0 -relief sunken -bg white
    # Create the image from a gif file:
    if {$iconfile != ""} {
	image create photo graphic -file \
		[file join $GUI_DIR graphics $iconfile.gif]
	label $w.f1.lbl -image graphic -bg white
        message $w.f1.m -width 5i -text $text -font $helvfont -bg white
    } else {
	label $w.f1.lbl -text "No graphic for this source"
        message $w.f1.m -width 7i -text $text -font $helvfont -bg white
    }
    pack $w.f1.lbl $w.f1.m -side left -padx 10 -pady 5 -fill both
    pack $w.f1
    frame $w.sep -bd 4 -width 100 -height 2 -relief groove
    pack $w.sep -pady 10 -fill x
    button $w.okb -text "OK" -command "destroy $w" \
	    -relief groove -bd 8
    pack $w.okb -pady 10
}


proc get_1_default {name top text} {

    # read the macro file for CM $name to get defaults and put a label on the
    # main edit window.

    global omega helvfont
    set filename $omega/beamnrc/CMs/${name}_macros.mortran
    set default1 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	set i 0
	while {$i<100 & [string compare $default1 {}]==0} {
	    incr i
	    gets $fileid data
	    set data [string trimright $data]
	    if [string first "REPLACE" $data]>=0 {
		if [string first $name $data]>0 {
		    set start [string last "\{" $data]
		    incr start
		    set end [string last "\}" $data]
		    incr end -1
		    set default1 [string range $data $start $end]
		}
	    }
	}
	close $fileid
    } else {
	tk_dialog .nope "Too bad" "Couldn't get the defaults" info 0 OK
    }

    label $top.default1 -text "The default $text is $default1." -font $helvfont
    pack $top.default1 -side top
}

proc get_2_defaults {name top str1 text1 str2 text2} {

    # get two defaults for CM $name from the macro file and put 2 labels
    # on the main edit window.

    global omega helvfont
    set filename $omega/beamnrc/CMs/${name}_macros.mortran
    set default1 {}
    set default2 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	set i 0
	while { [string compare $default2 {}]==0 || \
		[string compare $default1 {}]==0 } {
	    gets $fileid data
	    incr i
	    set data [string trimright $data]
	    if [string first "REPLACE" $data]>=0 {
		if [string first $str1 $data]>0 {
		    set start [string last "\{" $data]
		    incr start
		    set end [string last "\}" $data]
		    incr end -1
		    set default1 [string range $data $start $end]
		} elseif [string first $str2 $data]>0 {
		    set start [string last "\{" $data]
		    incr start
		    set end [string last "\}" $data]
		    incr end -1
		    set default2 [string range $data $start $end]
		}
	    }
	    if $i>200 {break}
	}
	close $fileid
    } else {
	tk_dialog .nope "Too bad" "Couldn't get the defaults" info 0 OK
    }

    label $top.default1 -text "The default $text1 is $default1," -font $helvfont
    label $top.default2 -text "and the default $text2 is $default2." \
	    -font $helvfont
    pack $top.default1 $top.default2 -side top
}

proc add_rmax_square { w index } {
    global GUI_DIR
    frame $w -bd 4
    button $w.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_rmax [winfo parent $w]"
    label $w.label -text {Half-width of outer square boundary (cm)}
    entry $w.inp -textvariable cmval($index) -width 8
    pack $w.help -anchor w -side left
    pack $w.label -side left -padx 10
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_rmax_rad { w index } {
    global GUI_DIR
    frame $w -bd 4
    button $w.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_rmax [winfo parent $w]"
    label $w.label -text {Outer radial boundary for CM (cm)}
    entry $w.inp -textvariable cmval($index) -width 8
    pack $w.help -anchor w -side left
    pack $w.label -side left -padx 10
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_title { w index } {
    global GUI_DIR
    frame $w -bd 4
    button $w.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_title [winfo parent $w]"
    label $w.label -text {Title}
    entry $w.inp -textvariable cmval($index) -width 8
    pack $w.help -anchor w -side left
    pack $w.label -side left -padx 10
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_zmin { w index } {
    global GUI_DIR
    frame $w -bd 4
    button $w.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_zmin [winfo parent $w]"
    label $w.label -text {Distance of front of material in CM\
	    to reference plane (cm)}
    entry $w.inp -textvariable cmval($index) -width 8
    pack $w.help -anchor w -side left
    pack $w.label -side left -padx 10
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_ecut { w index } {
    global cmval values
    frame $w
    button $w.b -text "?" -command "help_ecut $w"
    label $w.lab -text "Electron cutoff energy (default ECUTIN) (MeV)"
    pack $w.b -anchor w -side left
    pack $w.lab -side left -padx 5
    if [string compare $cmval($index) ""]==0 {
	set cmval($index) $values(ecut)
    }
    entry $w.ent -textvariable cmval($index) -width 10
    pack $w.ent -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_pcut { w index } {
    global cmval values
    frame $w
    button $w.b -text "?" -command "help_pcut $w"
    label $w.lab -text "Photon cutoff energy (default PCUTIN) (MeV)"
    pack $w.b -anchor w -side left
    pack $w.lab -side left -padx 5
    if [string compare $cmval($index) ""]==0 {
	set cmval($index) $values(pcut)
    }
    entry $w.ent -textvariable cmval($index) -width 10
    pack $w.ent -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_dose { w index } {
    global cmval
    frame $w
    button $w.b -text "?" -command "help_dose $w"
    label $w.lab -text "Dose zone (0 for no scoring)"
    pack $w.b -anchor w -side left
    pack $w.lab -side left -padx 5
    entry $w.ent -textvariable cmval($index) -width 10
    pack $w.ent -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_latch { w index } {
    global cmval
    frame $w
    button $w.b -text "?" -command "help_latch $w"
    label $w.lab -text "Associate with LATCH bit"
    pack $w.b -anchor w -side left
    pack $w.lab -padx 5 -side left
    entry $w.ent -textvariable cmval($index) -width 10
    pack $w.ent -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}

proc add_material { w index } {
    global cmval nmed medium med_per_column

    # Medium: an option menu with medium($i)
    frame $w
    button $w.b -text "?" -command "help_material $w"
    label $w.lab -text "Material"
    pack $w.b -anchor w -side left
    pack $w.lab -padx 5 -side left
    set width 20
    if [catch {string compare $cmval($index) {} }]==1 {
	set cmval($index) {}
    }
    menubutton $w.ent -text $cmval($index) -menu \
	    $w.ent.m -bd 1 -relief raised -indicatoron 1\
	    -width $width
    menu $w.ent.m -tearoff no
    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
           $w.ent.m add command -label $medium($iopt) -columnbreak [expr $iopt % $med_per_column == 0]\
                -command "set_material $w.ent $iopt $index"
    }
    pack $w.ent -anchor e -side right -fill x -expand true
    pack $w -side top -fill x
}
proc set_material { w iopt index } {
    global cmval medium
    set cmval($index) $medium($iopt)
    $w configure -text $cmval($index)
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
