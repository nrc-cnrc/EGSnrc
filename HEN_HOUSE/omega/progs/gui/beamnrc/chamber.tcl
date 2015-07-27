
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: CHAMBER
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


proc init_CHAMBER { id } {
    global cmval top_identical chm_identical bot_identical

    for {set i 0} {$i<3} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,3,0) 0
    set cmval($id,3,1) 0
    set cmval($id,3,2) 0

    for {set j 0} {$j<=100} {incr j} {
	for {set k 0} {$k<=2} {incr k} {
	    set cmval($id,4,0,$k,$j) {}
	}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,4,1,$k,$j) {}
	}
	set cmval($id,4,2,$j) {}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,4,3,$k,$j) {}
	}
	set cmval($id,4,4,$j) {}
    }

    for {set j 0} {$j<=2} {incr j} {
	set cmval($id,5,0,$j) {}
    }
    for {set j 0} {$j<=20} {incr j} {
	for {set k 0} {$k<=1} {incr k} {
	    set cmval($id,5,1,$k,$j) {}
	}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,5,2,$k,$j) {}
	}
	set cmval($id,5,3,$j) {}
    }
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,5,4,$k) {}
    }
    set cmval($id,5,5) {}
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,5,6,$k) {}
    }
    set cmval($id,5,7) {}
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,5,8,$k) {}
    }
    set cmval($id,5,9) {}

    for {set j 1} {$j<=20} {incr j} {
	for {set k 0} {$k<=2} {incr k} {
	    set cmval($id,6,0,$k,$j) {}
	}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,6,1,$k,$j) {}
	}
	set cmval($id,6,2,$j) {}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,6,3,$k,$j) {}
	}
	set cmval($id,6,4,$j) {}
    }

    set cmval($id,7) 0

    set top_identical($id) 0
    set bot_identical($id) 0
    set chm_identical($id) 0
}

proc read_CHAMBER { fileid id } {
    global cmval GUI_DIR top_identical chm_identical bot_identical cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read CHAMBER $cm_ident($id).  The inputs don't begin where\
		they should.  There is either an error in the CM preceding\
		this, or the accelerator module file is not the correct one\
		for use with this input file." error 0 OK
	return -code break
	# breaks out of the loops this procedure was called from.
    }

    # read rmax 0
    gets $fileid data
    set data [get_val $data cmval $id,0]

    # read title 1
    gets $fileid data
    set cmval($id,1) $data

    # read zmin 2
    gets $fileid data
    set data [get_val $data cmval $id,2]

    # read ntop, nchm, nbot
    gets $fileid data
    set data [get_val $data cmval $id,3,0]
    set ntop $cmval($id,3,0)

    set data [get_val $data cmval $id,3,1]
    set nchm $cmval($id,3,1)

    set data [get_val $data cmval $id,3,2]
    set nbot $cmval($id,3,2)

    # TOP OF CHAMBER
    # read if $ntop>0
    if $ntop>0 {
	# read the first set - if nflag=ntop all are identical
	set i 1
	gets $fileid data
	for {set j 0} {$j<=2} {incr j} {
	    set data [get_val $data cmval $id,4,0,$j,1]
	}
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,4,1,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,4,2,1]
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,4,3,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,4,4,1]

	set top_identical($id) 1
	if [expr $cmval($id,4,0,2,1)]!=$ntop {
	    set top_identical($id) 0
	    for {set i 2} {$i<=$ntop} {incr i} {
		gets $fileid data
		for {set j 0} {$j<=1} {incr j} {
		    set data [get_val $data cmval $id,4,0,$j,$i]
		}
		gets $fileid data
		for {set j 0} {$j<=3} {incr j} {
		    set data [get_val $data cmval $id,4,1,$j,$i]
		}
		gets $fileid data
		set data [get_str $data cmval $id,4,2,$i]
		gets $fileid data
		for {set j 0} {$j<=3} {incr j} {
		    set data [get_val $data cmval $id,4,3,$j,$i]
		}
		gets $fileid data
		set data [get_str $data cmval $id,4,4,$i]
	    }
	}
    }

    # CHAMBER
    # radii of chamber wall, (inner , outer) and inner of container wall
    gets $fileid data
    for {set j 0} {$j<=2} {incr j} {
	set data [get_val $data cmval $id,5,0,$j]
    }
    # read the first set - if nflag=nchm all are identical
    # only if nchm>0 (if negative it's groups which is the next case)
    if $nchm>0 {
	gets $fileid data
	for {set j 0} {$j<=1} {incr j} {
	    set data [get_val $data cmval $id,5,1,$j,1]
	}
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,5,2,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,5,3,1]

	# REPEAT IF NECESSARY
	if [string compare $cmval($id,5,1,1,1) ""]==0 {set cmval($id,5,1,1,1) 0}
	if [expr $cmval($id,5,1,1,1)]==$nchm {
	    # already read what's required.
	    set chm_identical($id) 1
	} elseif [expr $cmval($id,5,1,1,1)]!=$nchm {
	    set chm_identical($id) 0
	    for {set i 2} {$i<=$nchm} {incr i} {
		gets $fileid data
		set data [get_val $data cmval $id,5,1,0,$i]
		gets $fileid data
		for {set j 0} {$j<=3} {incr j} {
		    set data [get_val $data cmval $id,5,2,$j,$i]
		}
		gets $fileid data
		set data [get_str $data cmval $id,5,3,$i]
	    }
	}
    } elseif $nchm<0 {
	# don't want this to show up in the text box as negative, so change.
	set cmval($id,3,1) [expr -$nchm]
	set chm_identical($id) 2
	# nchm<0: read in thicknesses, #layers/group first -nchm of them.
	for {set i 1} {$i<=[expr -$nchm]} {incr i} {
	    gets $fileid data
	    set data [get_val $data cmval $id,5,1,0,$i]
	    set data [get_val $data cmval $id,5,1,1,$i]
	}
	# then read the ecut, pcut etc for all the groups (one set)
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,5,2,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,5,3,1]
    }

    # chamber wall inputs
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,5,4,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,5,5]

    # gap between chamber and container
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,5,6,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,5,7]

    # container wall
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,5,8,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,5,9]

    # BOTTOM
    if $nbot>0 {
	# read the first set - if nflag=nbot all are identical
	gets $fileid data
	for {set j 0} {$j<=2} {incr j} {
	    set data [get_val $data cmval $id,6,0,$j,1]
	}
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,6,1,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,6,2,1]
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,6,3,$j,1]
	}
	gets $fileid data
	set data [get_str $data cmval $id,6,4,1]

	set bot_identical($id) 1
	if [string compare "" $cmval($id,6,0,2,1)]==0 {set cmval($id,6,0,2,1) 0}
	if { [expr $cmval($id,6,0,2,1)]!=$nbot } {
	    set bot_identical($id) 0

	    for {set i 2} {$i<=$nbot} {incr i} {
		gets $fileid data
		for {set j 0} {$j<=1} {incr j} {
		    set data [get_val $data cmval $id,6,0,$j,$i]
		}
		gets $fileid data
		for {set j 0} {$j<=3} {incr j} {
		    set data [get_val $data cmval $id,6,1,$j,$i]
		}
		gets $fileid data
		set data [get_str $data cmval $id,6,2,$i]
		gets $fileid data
		for {set j 0} {$j<=3} {incr j} {
		    set data [get_val $data cmval $id,6,3,$j,$i]
		}
		gets $fileid data
		set data [get_str $data cmval $id,6,4,$i]
	    }
	}
    }

    # range rejection, mrnge
    gets $fileid data
    set indx [string first , $data]
    if { $indx==0 } {
        set cmval($id,7) 0
    } elseif { $indx>0 } {
        set data [string range $data 0 [incr indx -1] ]
        if { [string first 1 $data]>=0 } {
           set cmval($id,7) 1
        } else {
           set cmval($id,7) 0
        }
    } else {
        set indx [string length $data]
        set cmval($id,7) [expr [string range $data 0 [incr indx -1] ] ]
    }
}


proc edit_CHAMBER { id zmax } {
    global cmval GUI_DIR top_identical bot_identical chm_identical chm_option
    global omega helvfont help_chamber_text

    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NCHAMBER.
    catch { destroy .chamb$id }
    toplevel .chamb$id
    wm title .chamb$id "Edit CHAMBER, CM\#$id"
    frame .chamb$id.frm -bd 4
    set top .chamb$id.frm

    if [catch {expr $top_identical($id)}]==1 {
	set top_identical($id) 0
    }
    if [catch {expr $chm_identical($id)}]==1 {
	set chm_identical($id) 0
    }
    if [catch {expr $bot_identical($id)}]==1 {
	set bot_identical($id) 0
    }

    label $top.mainlabel -text "Chamber" -font $helvfont
    pack $top.mainlabel -pady 10

    #REPLACE {$MAX_N_$CHAMBER} WITH {101}
    get_1_default CHAMBER $top "maximum number of chamber layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_rad $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2

    frame $top.sep1 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep1 -side top -fill x -pady 10

    frame $top.inp3 -bd 10
    frame $top.inp3.frm
    set w $top.inp3.frm.f1
    frame $w
    label $w.label -text {Number of layers in top part}
    entry $w.inp -textvariable cmval($id,3,0) -width 8
    pack $w.label -anchor w -side left -padx 5
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -anchor w
    checkbutton $top.inp3.frm.check -text "identical layers"\
	    -variable top_identical($id) -onvalue 1 -offvalue 0\
	    -anchor w
    pack $top.inp3.frm.check -side top -anchor w
    button $top.inp3.edit -text " Edit >> " -width 10 \
	    -command "define_chamber_top_props $id"
    pack $top.inp3.frm -anchor w -side left -fill x
    pack $top.inp3.edit -padx 20 -side right -anchor e
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 10
    frame $top.inp4.frm

    # AN OPTION MENU TO ALLOW THE USER TO CHOOSE CHAMBER OR PHANTOM
    set w $top.inp4.frm.f0
    frame $w
    button $w.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help chm_option"
    pack $w.help -side left -anchor w
    label $w.lab -text "Treat central region as"
    pack $w.lab -side left -padx 5
    set chm_option chamber
    tk_optionMenu $w.option chm_option\
	    chamber phantom
    pack $w.option -side left -padx 5
    pack $w -anchor w

    set w $top.inp4.frm.f1
    frame $w
    label $w.label -text {Number of layers OR groups in chamber}
    entry $w.inp -textvariable cmval($id,3,1) -width 8
    pack $w.label -anchor w -side left -padx 5
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -anchor w
    radiobutton $top.inp4.frm.check0 -text "define layers individually"\
	    -variable chm_identical($id) -value 0 -anchor w
    pack $top.inp4.frm.check0 -side top -anchor w
    radiobutton $top.inp4.frm.check2 -text "define layers as groups"\
	    -variable chm_identical($id) -value 2 -anchor w
    pack $top.inp4.frm.check2 -side top -anchor w
    radiobutton $top.inp4.frm.check1 -text "define layers as identical"\
	    -variable chm_identical($id) -value 1 -anchor w
    pack $top.inp4.frm.check1 -side top -anchor w
    button $top.inp4.edit -text " Edit >> " -width 10 \
	    -command "define_chamber_props $id"
    pack $top.inp4.frm -anchor w -side left -fill x
    pack $top.inp4.edit -padx 20 -side right -anchor e
    pack $top.inp4 -side top -fill x

    # CONFIGURE THE RADIOBUTTONS TO DISPLAY THE RIGHT VALUE
#    for {set k 0} {$k<=2} {incr k} {
#	if $chm_identical($id)==$k {
#	    $top.inp4.frm.check$k invoke
#	}
#    }

    frame $top.inp5 -bd 10
    frame $top.inp5.frm
    set w $top.inp5.frm.f1
    frame $w
    label $w.label -text {Number of layers in bottom part}
    entry $w.inp -textvariable cmval($id,3,2) -width 8
    pack $w.label -anchor w -side left -padx 5
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -anchor w
    checkbutton $top.inp5.frm.check -text "identical layers"\
	    -variable bot_identical($id) -onvalue 1 -offvalue 0\
	    -anchor w
    pack $top.inp5.frm.check -side top -anchor w
    button $top.inp5.edit -text " Edit >> " -width 10 \
	    -command "define_chamber_bot_props $id"
    pack $top.inp5.frm -anchor w -side left -fill x
    pack $top.inp5.edit -padx 20 -side right -anchor e
    pack $top.inp5 -side top -fill x

    frame $top.sep2 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep2 -side top -anchor w -fill both

    # range rejection radiobuttons
    frame $top.inp6 -bd 10
    radiobutton $top.inp6.r1 -text "No ECUTRR calculation, range rejection\
	    will still be done on a region-by-region basis"\
	    -variable cmval($id,7) -value 0
    radiobutton $top.inp6.r2 -text "Estimate thickness of the chamber for\
	    ECUTRR calculations in automated range rejection"\
	    -variable cmval($id,7) -value 1
    pack $top.inp6.r1 $top.inp6.r2 -side top -pady 5 -anchor w
    pack $top.inp6 -side top -fill x

    frame $top.sep3 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep3 -side top -anchor w -fill both

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .chamb$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command \
	    "help_gif .chamb$id.help {$help_chamber_text} help_chamber"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_CHAMBER $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts -pady 10
    pack $top
}



proc define_chamber_top_props { id } {
    global cmval helvfont values
    global top_identical chm_identical bot_identical

    if $cmval($id,3,0)==0 {
	tk_dialog .chamb$id.nope "Nuh-uh" "There are no top layers." \
		warning 0 OK
	return
    }

    # allow a maximum of 2/window, the proceed to the next window.

    if $top_identical($id)==1 {
	set ntop 1
    } else {
	set ntop $cmval($id,3,0)
    }
    set nwindow [expr $ntop/2]
    if [expr $ntop%2]>0 {
	incr nwindow
    }
    set x 100
    set y 100
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .chamb$id.baby$i }
	toplevel .chamb$id.baby$i
	set top .chamb$id.baby$i
	wm title $top "Define top part of chamber, window $i"
	wm geometry $top +$x+$y
	incr x 40
	incr y 40
	frame $top.frm -bd 4
	for {set ii 1} {$ii<=2} {incr ii} {
	    set index [expr 2*($i-1)+$ii]
	    if $index>$ntop {break}

	    frame $top.frm.f$i-$ii -bd 4 -relief groove
	    set w $top.frm.f$i-$ii
	    label $w.mainlab -text "Layer $index" -font $helvfont
	    pack $w.mainlab -side top

	    frame $w.thick
	    label $w.thick.lab -text "Thickness of layer $index"
	    entry $w.thick.inp -textvariable cmval($id,4,0,0,$index) -width 10
	    pack $w.thick.lab -anchor w -side left
	    pack $w.thick.inp -anchor e -side right -fill x -expand true
	    pack $w.thick -side top -anchor w -fill x
	    frame $w.rad
	    label $w.rad.lab -text "Radius of inner cylinders"
	    entry $w.rad.inp -textvariable cmval($id,4,0,1,$index) -width 10
	    pack $w.rad.lab -anchor w -side left
	    pack $w.rad.inp -anchor e -side right -fill x -expand true
	    pack $w.rad -anchor w -side top -fill x

	    label $w.innerlab -text "Inner cylinders:" -font $helvfont
	    pack $w.innerlab -side top -anchor w -pady 10

	    add_ecut $w.f0 $id,4,1,0,$index
	    add_pcut $w.f1 $id,4,1,1,$index
	    add_dose $w.f2 $id,4,1,2,$index
	    add_latch $w.f3 $id,4,1,3,$index
	    add_material $w.f4 $id,4,2,$index

	    label $w.outerlab -text "Outer annuli:" -font $helvfont
	    pack $w.outerlab -side top -anchor w -pady 10

	    add_ecut $w.g0 $id,4,3,0,$index
	    add_pcut $w.g1 $id,4,3,1,$index
	    add_dose $w.g2 $id,4,3,2,$index
	    add_latch $w.g3 $id,4,3,3,$index
	    add_material $w.g4 $id,4,4,$index

	    pack $w -side left
	}
	pack $top.frm -side top
	button $top.okb -text "OK" -command "destroy .chamb$id.baby$i"\
	    -relief groove -bd 8
	pack $top.okb -pady 10 -side top
	if [expr 2*($i-1)+$ii]>$ntop {break}
    }
}

proc define_chamber_bot_props { id } {
    global cmval helvfont values
    global top_identical chm_identical bot_identical

    if $cmval($id,3,2)==0 {
	tk_dialog .chamb$id.nope "Nuh-uh" "There are no bottom layers." \
		warning 0 OK
	return
    }

    # allow a maximum of 2/window, the proceed to the next window.

    if $bot_identical($id)==1 {
	set nbot 1
    } else {
	set nbot $cmval($id,3,2)
    }
    set nwindow [expr $nbot/2]
    if [expr $nbot%2]>0 {
	incr nwindow
    }
    set x 100
    set y 100
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .chamb$id.baby$i }
	toplevel .chamb$id.baby$i
	set top .chamb$id.baby$i
	wm title $top "Define bottom part of chamber, window $i"
	wm geometry $top +$x+$y
	incr x 40
	incr y 40
	frame $top.frm -bd 4
	for {set ii 1} {$ii<=2} {incr ii} {
	    set index [expr 2*($i-1)+$ii]
	    if $index>$nbot {break}

	    frame $top.frm.f$i-$ii -bd 4 -relief groove
	    set w $top.frm.f$i-$ii
	    label $w.mainlab -text "Layer $index" -font $helvfont
	    pack $w.mainlab -side top

	    frame $w.thick
	    label $w.thick.lab -text "Thickness of layer $index"
	    entry $w.thick.inp -textvariable cmval($id,6,0,0,$index) -width 10
	    pack $w.thick.lab -anchor w -side left
	    pack $w.thick.inp -anchor e -side right -fill x -expand true
	    pack $w.thick -side top -anchor w -fill x
	    frame $w.rad
	    label $w.rad.lab -text "Radius of inner cylinders"
	    entry $w.rad.inp -textvariable cmval($id,6,0,1,$index) -width 10
	    pack $w.rad.lab -anchor w -side left
	    pack $w.rad.inp -anchor e -side right -fill x -expand true
	    pack $w.rad -anchor w -side top -fill x

	    label $w.innerlab -text "Inner cylinders:" -font $helvfont
	    pack $w.innerlab -side top -anchor w -pady 10

	    add_ecut $w.f0 $id,6,1,0,$index
	    add_pcut $w.f1 $id,6,1,1,$index
	    add_dose $w.f2 $id,6,1,2,$index
	    add_latch $w.f3 $id,6,1,3,$index
	    add_material $w.f4 $id,6,2,$index

	    label $w.outerlab -text "Outer annuli:" -font $helvfont
	    pack $w.outerlab -side top -anchor w -pady 10

	    add_ecut $w.g0 $id,6,3,0,$index
	    add_pcut $w.g1 $id,6,3,1,$index
	    add_dose $w.g2 $id,6,3,2,$index
	    add_latch $w.g3 $id,6,3,3,$index
	    add_material $w.g4 $id,6,4,$index

	    pack $w -side left
	}
	pack $top.frm -side top
	button $top.okb -text "OK" -command "destroy .chamb$id.baby$i"\
		-relief groove -bd 8
	pack $top.okb -pady 10 -side top
	if [expr 2*($i-1)+$ii]>$nbot {break}
    }
}


proc define_chamber_props { id } {
    global cmval helvfont values
    global chm_identical chm_option buttoncolor

    if {$cmval($id,3,1)==0 || [string compare $cmval($id,3,1) {}]==0} {
	tk_dialog .chamb$id.nope "Nuh-uh" "There are no chamber layers, \
		which means there is no chamber!  This is VERY BAD!  Put\
		in some layers!" warning 0 OK
	return
    }

    catch { destroy .chamb$id.child }
    toplevel .chamb$id.child
    set top .chamb$id.child
    wm title $top "Define chamber"

    frame $top.frm -bd 4
    if [string compare $chm_option "chamber"]==0 {
	set mainlab(0) "Chamber wall"
	set mainlab(1) "Gap between chamber and container"
	set mainlab(2) "Container wall"
	set thicklab(0) "Inner radius of chamber wall"
	set thicklab(1) "Outer radius of chamber wall"
	set thicklab(2) "Inner radius of container wall"
    } else {
	set mainlab(0) "Large region of phantom"
	set mainlab(1) "First region outside large region of phantom"
	set mainlab(2) "Second region outside large region of phantom"
	set thicklab(0) "Radius of central-axis region (cm)"
	set thicklab(1) "Outer radius (cm)"
	set thicklab(2) "Outer radius (cm)"
    }

    set w $top.frm
    if [string compare $chm_option "chamber"]==0 {
	for {set i 0} {$i<3} {incr i} {
	    set index [expr 4+2*$i]
	    label $w.mainlab$i -text $mainlab($i) -font $helvfont
	    pack $w.mainlab$i -side top -anchor w -pady 10

	    frame $w.thick$i
	    label $w.thick$i.lab -text $thicklab($i)
	    entry $w.thick$i.inp -textvariable cmval($id,5,0,$i) -width 10
	    pack $w.thick$i.lab -anchor w -side left
	    pack $w.thick$i.inp -anchor e -side right -fill x -expand true
	    pack $w.thick$i -side top -anchor w -fill x

	    add_ecut $w.f0-$i $id,5,$index,0
	    add_pcut $w.f1-$i $id,5,$index,1
	    add_dose $w.f2-$i $id,5,$index,2
	    add_latch $w.f3-$i $id,5,$index,3
	    add_material $w.f4-$i $id,5,[expr $index+1]

	    pack $w -side left
	}
    } else {
	set i 0
	set index [expr 4+2*$i]
	label $w.mainlab$i -text $mainlab($i) -font $helvfont
	pack $w.mainlab$i -side top -anchor w -pady 10

	frame $w.thick$i
	label $w.thick$i.lab -text $thicklab(1)
	entry $w.thick$i.inp -textvariable cmval($id,5,0,1) -width 10
	pack $w.thick$i.lab -anchor w -side left
	pack $w.thick$i.inp -anchor e -side right -fill x -expand true
	pack $w.thick$i -side top -anchor w -fill x

	add_ecut $w.f0-$i $id,5,$index,0
	add_pcut $w.f1-$i $id,5,$index,1
	add_dose $w.f2-$i $id,5,$index,2
	add_latch $w.f3-$i $id,5,$index,3
	add_material $w.f4-$i $id,5,[expr $index+1]

	set i 1
	set index [expr 4+2*$i]
	label $w.mainlab$i -text $mainlab($i) -font $helvfont
	pack $w.mainlab$i -side top -anchor w -pady 10

	frame $w.thick$i
	label $w.thick$i.lab -text $thicklab(2)
	entry $w.thick$i.inp -textvariable cmval($id,5,0,2) -width 10
	pack $w.thick$i.lab -anchor w -side left
	pack $w.thick$i.inp -anchor e -side right -fill x -expand true
	pack $w.thick$i -side top -anchor w -fill x

	add_ecut $w.f0-$i $id,5,$index,0
	add_pcut $w.f1-$i $id,5,$index,1
	add_dose $w.f2-$i $id,5,$index,2
	add_latch $w.f3-$i $id,5,$index,3
	add_material $w.f4-$i $id,5,[expr $index+1]

	set i 2
	set index [expr 4+2*$i]
	label $w.mainlab$i -text $mainlab($i) -font $helvfont
	pack $w.mainlab$i -side top -anchor w -pady 10

	add_ecut $w.f0-$i $id,5,$index,0
	add_pcut $w.f1-$i $id,5,$index,1
	add_dose $w.f2-$i $id,5,$index,2
	add_latch $w.f3-$i $id,5,$index,3
	add_material $w.f4-$i $id,5,[expr $index+1]

	pack $w -side left
    }

    frame $top.frm2
    set w $top.frm2
    if { $chm_identical($id)==1 } {

	if [string compare $chm_option "chamber"]==0 {
	    label $w.label -text "For all layers:" -font $helvfont
	} else {
	    label $w.label -text "For layers in central-axis region:" \
		    -font $helvfont
	}
	pack $w.label -pady 10 -anchor w

	if [string compare $chm_option "phantom"]==0 {
	    frame $w.thick0
	    label $w.thick0.lab -text $thicklab(0)
	    entry $w.thick0.inp -textvariable cmval($id,5,0,0) -width 10
	    pack $w.thick0.lab -anchor w -side left
	    pack $w.thick0.inp -anchor e -side right -fill x -expand true
	    pack $w.thick0 -side top -anchor w -fill x -pady 5
	}

	frame $w.thick
	label $w.thick.lab -text "Thickness of each layer (cm)"
	entry $w.thick.inp -textvariable cmval($id,5,1,0,1) -width 10
	pack $w.thick.lab -anchor w -side left
	pack $w.thick.inp -anchor e -side right -fill x -expand true
	pack $w.thick -side top -anchor w -fill x -pady 5


	add_ecut $w.f0 $id,5,2,0,1
	add_pcut $w.f1 $id,5,2,1,1
	if [string compare $chm_option "chamber"]==0 {
	    add_dose $w.f2 $id,5,2,2,1
	} else {
	    frame $w.f2
	    label $w.f2.lab -text "Dose zone for this volume element\
		    (0 for no scoring)"
	    pack $w.f2.lab -anchor w -side left
	    entry $w.f2.ent -textvariable cmval($id,5,2,2,1) -width 10
	    pack $w.f2.ent -anchor e -side right -fill x -expand true
	    pack $w.f2 -side top -fill x
	}
	add_latch $w.f3 $id,5,2,3,1
	add_material $w.f4 $id,5,3,1

    } elseif $chm_identical($id)==0 {
	set nchm $cmval($id,3,1)
	if [string compare $chm_option "chamber"]==0 {
	    label $w.label -text "For each layer:" -font $helvfont
	} else {
	    label $w.label -text "For each layer in central-axis region:" \
		    -font $helvfont
	}
	pack $w.label -pady 10 -anchor w

	if [string compare $chm_option "phantom"]==0 {
	    frame $w.thick0
	    label $w.thick0.lab -text $thicklab(0)
	    entry $w.thick0.inp -textvariable cmval($id,5,0,0) -width 10
	    pack $w.thick0.lab -anchor w -side left
	    pack $w.thick0.inp -anchor e -side right -fill x -expand true
	    pack $w.thick0 -side top -anchor w -fill x -pady 5
	}
	frame $w.gridfrm
	set row 0; set col 0
	for {set i 1} {$i<=$nchm} {incr i} {
	    button $w.gridfrm.b$i -text "Edit chamber layer $i" \
		    -command "edit_layer $id $i"
	    set_button_color $id $i
	    if { $row >= 10 } {
		incr col
		set row 0
	    }
	    grid configure $w.gridfrm.b$i -row $row -column $col -padx 5 -pady 5
	    incr row
	}
	pack $w.gridfrm -side top
	message $top.frm2.text -font $helvfont -width 3i -text \
		"If a button's text is red, that layer has not been\
		completely defined."
	pack $top.frm2.text -fill x

    } else {
	# groups
	button $w.thick -text "Define layer thickness in each group >>"\
		-command "edit_group_thick $id"
	pack $w.thick -side top -anchor w -fill x
	if [string compare $chm_option "chamber"]==0 {
	    label $w.label -text "For all groups:" -font $helvfont
	} else {
	    label $w.label -text "For all groups in central-axis region:" \
		    -font $helvfont
	}
	pack $w.label -pady 10 -anchor w
	if [string compare $chm_option "phantom"]==0 {
	    frame $w.thick0
	    label $w.thick0.lab -text $thicklab(0)
	    entry $w.thick0.inp -textvariable cmval($id,5,0,0) -width 10
	    pack $w.thick0.lab -anchor w -side left
	    pack $w.thick0.inp -anchor e -side right -fill x -expand true
	    pack $w.thick0 -side top -anchor w -fill x -pady 5
	}

	add_ecut $w.f0 $id,5,2,0,1
	add_pcut $w.f1 $id,5,2,1,1
	if [string compare $chm_option "chamber"]==0 {
	    add_dose $w.f2 $id,5,2,2,1
	} else {
	    frame $w.f2
	    label $w.f2.lab -text "Dose zones start at (0 for no scoring)"
	    pack $w.f2.lab -anchor w -side left
	    entry $w.f2.ent -textvariable cmval($id,5,2,2,1) -width 10
	    pack $w.f2.ent -anchor e -side right -fill x -expand true
	    pack $w.f2 -side top -fill x
	}
	add_latch $w.f3 $id,5,2,3,1
	add_material $w.f4 $id,5,3,1

    }
    button $top.frm2.okb -text "OK" -command "destroy .chamb$id.child"\
	    -bd 8 -relief groove
    pack $top.frm2.okb -pady 20 -side top
    pack $top.frm $top.frm2 -side left
}

proc set_button_color { id ilayer } {
    global buttoncolor cmval
    # ilayer is the layer to be checked.
    set buttoncolor($ilayer) black
    # if thickness is not done, set button to red
    if [catch {expr $cmval($id,5,1,0,$ilayer)}]==1 {
	set buttoncolor($ilayer) red
    }
    # if ecut,pcut dose_zone or latch bit are not filled, set to red
#    for {set j 0} {$j<=3} {incr j} {
#	if [catch {expr $cmval($id,5,2,$j,$ilayer)}]==1 {
#	    set buttoncolor($ilayer) red
#	}
#    }
    #if material is not set, set to red
    if [string compare $cmval($id,5,3,$ilayer) {}]==0 {
	set buttoncolor($ilayer) red
    }
    .chamb$id.child.frm2.gridfrm.b$ilayer configure -fg $buttoncolor($ilayer)
}


proc edit_group_thick { id } {
    global cmval

    catch { destroy .chamb$id.child.thick }
    toplevel .chamb$id.child.thick
    set w .chamb$id.child.thick
    wm title $w "Group layer thickness"
    for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
	frame $w.thick$i

	label $w.thick$i.lab1 -text "Thickness of each of the"
	entry $w.thick$i.inp1 -textvariable cmval($id,5,1,1,$i) -width 10
	pack $w.thick$i.lab1 -anchor w -side left
	pack $w.thick$i.inp1 -side left
	label $w.thick$i.lab2 -text "layers in group $i (cm)"
	entry $w.thick$i.inp2 -textvariable cmval($id,5,1,0,$i) -width 10
	pack $w.thick$i.lab2 -side left
	pack $w.thick$i.inp2 -anchor e -side right -fill x -expand true
	pack $w.thick$i -side top -anchor w -fill x -pady 5
    }
    button $w.okb -text "OK" -command "destroy .chamb$id.child.thick"\
	    -relief groove -bd 8
    pack $w.okb -pady 20 -side top
}

proc edit_layer {id layer} {
    global cmval chm_identical helvfont GUI_DIR sameaslayer chm_option
    set w .chamb$id.child.layer$layer
    catch { destroy $w }
    toplevel $w -bd 4

    label $w.label -text "Layer $layer" -font $helvfont
    pack $w.label -pady 10

    set sameaslayer {}

    button $w.calculate -text "Calculate distance to front of this layer" \
	    -command "calculate_distance $id $layer"
    pack $w.calculate -fill x -expand true -pady 10

    frame $w.sameas
    frame $w.sameas.frm
    button $w.sameas.frm.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_sameaslayer $w"
    pack $w.sameas.frm.help -anchor w -side left
    label $w.sameas.frm.label -text {Same as layer}
    pack $w.sameas.frm.label -side left -padx 5
    entry $w.sameas.frm.inp -textvariable sameaslayer -width 5
    pack $w.sameas.frm.inp -side left -padx 5
    button $w.sameas.frm.apply -text "Apply" \
	    -command "set_same_as_layer $id $layer"
    pack $w.sameas.frm.apply -anchor e -side right -fill x -expand true -padx 5
    label $w.sameas.caution \
	    -text {Caution!  Once filled, the old entries are lost.}
    pack $w.sameas.frm $w.sameas.caution -side top
    pack $w.sameas -side top -fill x -pady 15

    frame $w.thick0
    label $w.thick0.lab -text "Thickness of this layer (cm)"
    entry $w.thick0.inp -textvariable cmval($id,5,1,0,$layer) -width 10
    pack $w.thick0.lab -anchor w -side left
    pack $w.thick0.inp -anchor e -side right -fill x -expand true
    pack $w.thick0 -side top -anchor w -fill x

    add_ecut $w.f0 $id,5,2,0,$layer
    add_pcut $w.f1 $id,5,2,1,$layer
    if [string compare $chm_option "chamber"]==0 {
	add_dose $w.f2 $id,5,2,2,$layer
    } else {
	frame $w.f2
	label $w.f2.lab -text "Dose zones start at (0 for no scoring)"
	pack $w.f2.lab -anchor w -side left
	entry $w.f2.ent -textvariable cmval($id,5,2,2,$layer) -width 10
	pack $w.f2.ent -anchor e -side right -fill x -expand true
	pack $w.f2 -side top -fill x
    }
    add_latch $w.f3 $id,5,2,3,$layer
    add_material $w.f4 $id,5,3,$layer

    button $w.okb -text "OK" -command "destroy .chamb$id.child.layer$layer;\
	    set_button_color $id $layer" -relief groove -bd 8
    pack $w.okb -pady 10 -side top
}

proc calculate_distance {id layer} {
    global cmval
    set sum 0
    for {set i 1} {$i<$layer} {incr i} {
	if [catch {set x [expr $cmval($id,5,1,0,$i)]}]==1 {
	    tk_dialog .chamb$id.child.layer$layer.nocalc "No no no"\
		    "The layer thicknesses up to here haven't been defined yet."\
		    warning 0 OK
	    return
	}
	set sum [expr $sum+$x]
    }
    tk_dialog .chamb$id.child.layer$layer.calc "Distance in chamber"\
	    "The distance from the front of this chamber to the current\
	    layer is $sum cm" info 0 OK
}
proc set_same_as_layer {id layer} {
    global sameaslayer cmval
    if [string compare $sameaslayer {}]==0 {
	tk_dialog .chamb$id.child.layer$layer.nope "No no no"\
		"There's no input in the entry box.  Try again."\
		warning 0 OK
	return
    }
    if {$sameaslayer<1 || $sameaslayer>$cmval($id,3,1)} {
	tk_dialog .chamb$id.child.layer$layer.nope "No no no"\
		"Valid layers are from 1 to $cmval($id,3,1).  Try again."\
		warning 0 OK
	return
    }
    set cmval($id,5,1,0,$layer) $cmval($id,5,1,0,$sameaslayer)
    set cmval($id,5,2,0,$layer) $cmval($id,5,2,0,$sameaslayer)
    set cmval($id,5,2,1,$layer) $cmval($id,5,2,1,$sameaslayer)
    set cmval($id,5,2,2,$layer) $cmval($id,5,2,2,$sameaslayer)
    set cmval($id,5,2,3,$layer) $cmval($id,5,2,3,$sameaslayer)
    set cmval($id,5,3,$layer) $cmval($id,5,3,$sameaslayer)
}

proc write_CHAMBER {fileid id} {
    global cmval cm_names cm_ident cm_type
    global top_identical bot_identical chm_identical

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZMIN"
    if $chm_identical($id)==2 {
	puts $fileid "$cmval($id,3,0), -$cmval($id,3,1), $cmval($id,3,2), N_TOP, N_CHM, N_BOT"
    } else {
	puts $fileid "$cmval($id,3,0), $cmval($id,3,1), $cmval($id,3,2), N_TOP, N_CHM, N_BOT"
    }
    # Top part - if top_identical($id)==1 just do once, otherwise repeat for all

    if $cmval($id,3,0)>0 {
	if $top_identical($id)==1 {
	    set ntop 1
	    set cmval($id,4,0,2,1) $cmval($id,3,0)
	} else {
	    set ntop [expr $cmval($id,3,0)]
	    for {set j 1} {$j<=$ntop} {incr j} {
		set cmval($id,4,0,2,$j) 0
	    }
	}
    } else {
	set ntop 0
    }
    for {set i 1} {$i<=$ntop} {incr i} {
	set str {}
	set str "$str$cmval($id,4,0,0,$i), "
	set str "$str$cmval($id,4,0,1,$i), "
	set str "$str$cmval($id,4,0,2,$i), ZTHICK, RCYS, FLAG FOR LAYER $i IN TOP"
	puts $fileid $str
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,4,1,$j,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,4,2,$i)
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,4,3,$j,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,4,4,$i)
    }

    # CHAMBER part
    # radii
    set str {}
    set str "$str$cmval($id,5,0,0), "
    set str "$str$cmval($id,5,0,1), "
    set str "$str$cmval($id,5,0,2), RADII FOR CENTRAL PART"
    puts $fileid $str

    # layers 0=>individual 1=>identical 2=>groups
    if $chm_identical($id)==1 {
	# identical layers - zthick, then ecut,pcut etc.
	set nchm 1
	set flag $cmval($id,3,1)
	for {set i 2} {$i<=$nchm} {incr i} {
	    # automate the dose_zone to dose_zone+nchm-1
	    set cmval($id,5,2,2,$i) [expr $cmval($id,5,2,2,1)+$i-1]
	}
	# zthick, flag
	set str {}
	set str "$str$cmval($id,5,1,0,1), "
	set str "$str$cmval($id,3,1), ZTHICK, FLAG FOR ALL LAYERS IN CENTRAL PART"
	puts $fileid $str
	# ecut etc.
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,5,2,$j,1), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,5,3,1)

    } elseif $chm_identical($id)==2 {
	# groups -
	# flag is the number of layer in each group - different for all
	# still automate dose zone scoring
	set nchm $cmval($id,3,1)
	set flag -$cmval($id,3,1)
	for {set i 2} {$i<=$nchm} {incr i} {
	    # automate the dose_zone to dose_zone+nchm-1
	    set cmval($id,5,2,2,$i) [expr $cmval($id,5,2,2,1)+$i-1]
	}
	for {set i 1} {$i<=$nchm} {incr i} {
	    # zthick, flag
	    set str {}
	    set str "$str$cmval($id,5,1,0,$i), "
	    set str "$str$cmval($id,5,1,1,$i), ZTHICK, FLAG FOR GROUP $i IN CENTRAL PART"
	    puts $fileid $str
	}
	# ecut etc.
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,5,2,$j,1), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,5,3,1)

    } else {
	set nchm [expr $cmval($id,3,1)]
	set flag 0
	for {set i 1} {$i<=$nchm} {incr i} {
	    # zthick, flag
	    set str {}
	    set str "$str$cmval($id,5,1,0,$i), "
	    set str "$str$flag, ZTHICK, FLAG FOR LAYER $i IN CENTRAL PART"
	    puts $fileid $str
	    # ecut etc.
	    set str {}
	    for {set j 0 } {$j<=3} {incr j} {
		set str "$str$cmval($id,5,2,$j,$i), "
	    }
	    puts $fileid $str
	    puts $fileid $cmval($id,5,3,$i)
	}
    }
    # chamber wall ecut etc.
    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,5,4,$j), "
    }
    set str "$str  chamber wall"
    puts $fileid $str
    puts $fileid $cmval($id,5,5)
    # gap ecut etc.
    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,5,6,$j), "
    }
    set str "$str  gap"
    puts $fileid $str
    puts $fileid $cmval($id,5,7)
    # container ecut etc.
    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,5,8,$j), "
    }
    set str "$str  container"
    puts $fileid $str
    puts $fileid $cmval($id,5,9)

    # Bottom part
    if $cmval($id,3,2)>0 {
	if $bot_identical($id)==1 {
	    set nbot 1
	    set cmval($id,6,0,2,1) $cmval($id,3,2)
	} else {
	    set nbot [expr $cmval($id,3,2)]
	    for {set j 1} {$j<=$nbot} {incr j} {
		set cmval($id,6,0,2,$j) 0
	    }
	}
    } else {
	set nbot 0
    }
    for {set i 1} {$i<=$nbot} {incr i} {
	set str {}
	set str "$str$cmval($id,6,0,0,$i), "
	set str "$str$cmval($id,6,0,1,$i), "
	set str "$str$cmval($id,6,0,2,$i), ZTHICK, RCYS, FLAG FOR LAYER $i IN BOTTOM PART"
	puts $fileid $str
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,6,1,$j,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,6,2,$i)
	set str {}
	for {set j 0 } {$j<=3} {incr j} {
	    set str "$str$cmval($id,6,3,$j,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,6,4,$i)
    }
    # mrnge
    puts $fileid "$cmval($id,7), MRNGE"
}


proc show_CHAMBER { id } {
    global cmval xrange zrange top_identical chm_identical
    global bot_identical cm_ticks

    catch { destroy .chamb$id.show }
    toplevel .chamb$id.show
    wm title .chamb$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # find initial zrange,xrange
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    set topw .chamb$id.show
    frame $topw.toggle
    set toggle$id 0
    radiobutton $topw.toggle.off -text "Lines off" -variable toggle$id \
	    -value 0 -command "set toggle$id 0; $topw.can itemconfigure \
	    chamb_tag -outline {}"
    radiobutton $topw.toggle.on -text "Lines on" -variable toggle$id \
	    -value 1 -command "set toggle$id 1; $topw.can itemconfigure \
	    chamb_tag -outline black"
    pack $topw.toggle.off $topw.toggle.on -anchor w
    pack $topw.toggle -anchor w
    $topw.toggle.off select
    $topw.toggle.on deselect

    if [catch { draw_CHAMBER $id }]==1 {
	destroy .chamb$id.show
	tk_dialog .chamb$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .chamb$id.show.buts
    button .chamb$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .chamb$id.show xz" -relief groove -bd 8
    button .chamb$id.show.buts.print -text "Print..." -command\
	    "print_canvas .chamb$id.show.can 600 600" -relief groove -bd 8
    button .chamb$id.show.buts.done -text "Done" -command\
	    "destroy .chamb$id.show" -relief groove -bd 8
    pack .chamb$id.show.buts.range .chamb$id.show.buts.print\
	    .chamb$id.show.buts.done -side left -padx 10
    pack .chamb$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_CHAMBER { id } {
    global xrange zrange cmval top_identical xscale zscale
    global chm_identical bot_identical helvfont l m cm_ticks

    catch {destroy .chamb$id.show.can}

    # put the canvas up
    canvas .chamb$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 500x500 (allow
    # a x-border of 100 and z-border of 50 to show the scale)

    set width 450.0
    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    add_air $id .chamb$id.show.can $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_CHAMBER $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .chamb$id.show.can

    set curx 0
    set cury 0
    label .chamb$id.show.can.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .chamb$id.show.can create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .chamb$id.show.can.xy
    bind .chamb$id.show.can <Motion> {
	global l m xscale zscale xrange zrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$xscale+$xrange(0)]
	set cury [expr ($cury-$m)/$zscale+$zrange(0)]
        if { $curx<=$xrange(1) & $curx>=$xrange(0) & $cury<=$zrange(1) & \
                $cury>=$zrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    coverup $l $m $width .chamb$id.show.can

    add_axes xz $cm_ticks($id,x) $cm_ticks($id,z) $l $m $width $xrange(0)\
	    $xrange(1) $zrange(0) $zrange(1)\
	    $xscale $zscale .chamb$id.show.can

    pack .chamb$id.show.can -side top -anchor n
}

proc add_CHAMBER {id xscale zscale xmin zmin l m parent_w} {
    global top_identical bot_identical chm_identical cmval medium nmed
    global colorlist helvfont meds_used colornum

    # assign numbers to the media
    # top
    if $cmval($id,3,0)>0 {
	if $top_identical($id)==0 {
	    for {set i 1} {$i<=$cmval($id,3,0)} {incr i} {
		set med(top,in,$i) $colornum
		for {set j 0} {$j<=$nmed} {incr j} {
		    # inner
		    if [string compare $cmval($id,4,2,$i) $medium($j)]==0 {
			set med(top,in,$i) [min_nrc $j $colornum]
			set meds_used($j) 1
			break
		    }
		}
		set med(top,out,$i) $colornum
		for {set j 0} {$j<=$nmed} {incr j} {
		    # outer
		    if [string compare $cmval($id,4,4,$i) $medium($j)]==0 {
			set med(top,out,$i) [min_nrc $j $colornum]
			set meds_used($j) 1
			break
		    }
		}
	    }
	} else {
	    set med(top,in,1) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		# inner
		if [string compare $cmval($id,4,2,1) $medium($j)]==0 {
		    set med(top,in,1) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	    set med(top,out,1) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		# outer
		if [string compare $cmval($id,4,4,1) $medium($j)]==0 {
		    set med(top,out,1) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	}
    }
    # middle
    if $chm_identical($id)!=1 {
	for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
	    set med(chm,$i) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		if [string match $medium($j) $cmval($id,5,3,$i) ]==1 {
		    set med(chm,$i) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	}
    } elseif $chm_identical($id)==1 {
	set med(chm,1) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,5,3,1) $medium($j)]==0 {
		set med(chm,1) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    # chamber wall:
    set med(wall) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,5,5) $medium($j)]==0 {
	    set med(wall) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # gap:
    set med(gap) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,5,7) $medium($j)]==0 {
	    set med(gap) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # container wall:
    set med(cwall) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,5,9) $medium($j)]==0 {
	    set med(cwall) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # bottom
    if $cmval($id,3,2)>0 {
	if $bot_identical($id)==0 {
	    for {set i 1} {$i<=$cmval($id,3,2)} {incr i} {
		set med(bot,in,$i) $colornum
		for {set j 0} {$j<=$nmed} {incr j} {
		    # inner
		    if [string compare $cmval($id,6,2,$i) $medium($j)]==0 {
			set med(bot,in,$i) [min_nrc $j $colornum]
			set meds_used($j) 1
			break
		    }
		}
		set med(bot,out,$i) $colornum
		for {set j 0} {$j<=$nmed} {incr j} {
		    # outer
		    if [string compare $cmval($id,6,4,$i) $medium($j)]==0 {
			set med(bot,out,$i) [min_nrc $j $colornum]
			set meds_used($j) 1
			break
		    }
		}
	    }
	} else {
	    set med(bot,in,1) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		# inner
		if [string compare $cmval($id,6,2,1) $medium($j)]==0 {
		    set med(bot,in,1) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	    set med(bot,out,1) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		# outer
		if [string compare $cmval($id,6,4,1) $medium($j)]==0 {
		    set med(bot,out,1) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	}
    }

    # starting z:
    set tot [expr $cmval($id,2)-$zmin]

    # first top layer - if identical, do as one big block, else do separately.
    if $cmval($id,3,0)>0 {
	if $top_identical($id)==0 {

	    # all rectangular points.

	    for {set i 1} {$i<=$cmval($id,3,0)} {incr i} {
		set rcyl $cmval($id,4,0,1,$i)
		set rmax $cmval($id,0)
		set thick $cmval($id,4,0,0,$i)

		set y(1) [expr $tot*$zscale+$m]
		set y(2) [expr ($tot+$thick)*$zscale+$m]

		# inner bit
		set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
		set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(top,in,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		#outer bit, rhs
		set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
		set x(2) [expr ($rmax-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(top,out,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		#outer bit, lhs
		set x(1) [expr (-$rcyl-$xmin)*$xscale+$l]
		set x(2) [expr (-$rmax-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(top,out,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		set tot [expr $tot+$thick]
	    }
	} else {
	    # identical layers on top

	    set rcyl $cmval($id,4,0,1,1)
	    # all rectangular points.
	    set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
	    set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]
	    set x(3) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	    set x(4) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]

	    set rcyl $cmval($id,4,0,1,1)
	    set rmax $cmval($id,0)

	    set dz $cmval($id,4,0,0,1)
	    for {set i 1} {$i<=$cmval($id,3,0)} {incr i} {
		set y(1) [expr $tot*$zscale+$m]
		set y(2) [expr ($tot+$dz)*$zscale+$m]

		# inner bit
		set color [lindex $colorlist $med(top,in,1)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag
		# outer bits
		set color [lindex $colorlist $med(top,out,1)]
		$parent_w create rectangle $x(4) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag
		set color [lindex $colorlist $med(top,out,1)]
		$parent_w create rectangle $x(3) $y(1) $x(1) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		set tot [expr $tot+$dz]
	    }
	}
    }
    set zchmtop $tot

    # central part
    if $chm_identical($id)==0 {
	# define individually
	set rcyl $cmval($id,5,0,0)
	for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
	    set thick $cmval($id,5,1,0,$i)

	    set y(1) [expr $tot*$zscale+$m]
	    set y(2) [expr ($tot+$thick)*$zscale+$m]

	    # inner bit
	    set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
	    set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]
	    set color [lindex $colorlist $med(chm,$i)]
	    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
		    -fill $color -outline {} -tag chamb_tag

	    set tot [expr $tot+$thick]
	}
    } elseif $chm_identical($id)==1 {
	# identical layers
	set rcyl $cmval($id,5,0,0)

	# all rectangular points, all to same radius
	set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
	set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]

	set dz $cmval($id,5,1,0,1)
	for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
	    set y(1) [expr $tot*$zscale+$m]
	    set y(2) [expr ($tot+$dz)*$zscale+$m]

	    # inner bit
	    set color [lindex $colorlist $med(chm,1)]
	    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
		    -fill $color -outline {} -tag chamb_tag

	    set tot [expr $tot+$dz]
	}
    } elseif $chm_identical($id)==2 {
	# defined in groups, $cmval($id,3,1) is the number of groups
	set rcyl $cmval($id,5,0,0)

	# all rectangular points, all to same radius
	set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
	set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med(chm,1)]
	for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
	    # for each group,
	    set dz $cmval($id,5,1,0,$i)
	    for {set j 1} {$j<=$cmval($id,5,1,1,$i)} {incr j} {
		# for each layer in the group
		set y(1) [expr $tot*$zscale+$m]
		set y(2) [expr ($tot+$dz)*$zscale+$m]

		# inner bit
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		set tot [expr $tot+$dz]
	    }
	}
    }

    # chamber wall:
    set y(1) [expr $zchmtop*$zscale+$m]
    set y(2) [expr $tot*$zscale+$m]
    set x(1) [expr ($cmval($id,5,0,0)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,5,0,1)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(wall)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag
    set x(1) [expr (-$cmval($id,5,0,0)-$xmin)*$xscale+$l]
    set x(2) [expr (-$cmval($id,5,0,1)-$xmin)*$xscale+$l]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag

    # chamber gap:
    set x(1) [expr ($cmval($id,5,0,1)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,5,0,2)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(gap)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag
    set x(1) [expr (-$cmval($id,5,0,1)-$xmin)*$xscale+$l]
    set x(2) [expr (-$cmval($id,5,0,2)-$xmin)*$xscale+$l]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag

    # container wall:
    set x(1) [expr ($cmval($id,5,0,2)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(cwall)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag
    set x(1) [expr (-$cmval($id,5,0,2)-$xmin)*$xscale+$l]
    set x(2) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {} -tag chamb_tag

    # bottom
    if $cmval($id,3,2)>0 {
	if $bot_identical($id)==0 {
	    # individual layers
	    for {set i 1} {$i<=$cmval($id,3,2)} {incr i} {
		set rcyl $cmval($id,6,0,1,$i)
		set rmax $cmval($id,0)
		set thick $cmval($id,6,0,0,$i)

		set y(1) [expr $tot*$zscale+$m]
		set y(2) [expr ($tot+$thick)*$zscale+$m]

		# inner bit
		set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
		set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(bot,in,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		#outer bit, rhs
		set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
		set x(2) [expr ($rmax-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(bot,out,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		#outer bit, lhs
		set x(1) [expr (-$rcyl-$xmin)*$xscale+$l]
		set x(2) [expr (-$rmax-$xmin)*$xscale+$l]
		set color [lindex $colorlist $med(bot,out,$i)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		set tot [expr $tot+$thick]
	    }
	} else {
	    # identical layers on bottom

	    set rcyl $cmval($id,6,0,1,1)

	    # all rectangular points.
	    set x(1) [expr ($rcyl-$xmin)*$xscale+$l]
	    set x(2) [expr (-$rcyl-$xmin)*$xscale+$l]
	    set x(3) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	    set x(4) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]

	    set rcyl $cmval($id,6,0,1,1)
	    set rmax $cmval($id,0)

	    set dz $cmval($id,6,0,0,1)
	    for {set i 1} {$i<=$cmval($id,3,2)} {incr i} {
		set y(1) [expr $tot*$zscale+$m]
		set y(2) [expr ($tot+$dz)*$zscale+$m]

		# inner bit
		set color [lindex $colorlist $med(bot,in,1)]
		$parent_w create rectangle $x(1) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag
		# outer bits
		set color [lindex $colorlist $med(bot,out,1)]
		$parent_w create rectangle $x(4) $y(1) $x(2) $y(2)\
			-fill $color -outline {} -tag chamb_tag
		set color [lindex $colorlist $med(bot,out,1)]
		$parent_w create rectangle $x(3) $y(1) $x(1) $y(2)\
			-fill $color -outline {} -tag chamb_tag

		set tot [expr $tot+$dz]
	    }
	}
    }
}

