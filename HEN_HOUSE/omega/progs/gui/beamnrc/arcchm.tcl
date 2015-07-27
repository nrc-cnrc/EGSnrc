
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: ARCCHM
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


proc init_ARCCHM { id } {
    global cmval irepeat

    for {set i 0} {$i<11} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,11,0) {}
    set cmval($id,11,1) {}
    set cmval($id,12) {}

    for {set j 0} {$j<=100} {incr j} {
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,13,$j,$k) {}
	}
	set cmval($id,14,$j) {}
    }
    foreach j {back sur xwall} {
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,13,$j,$k) {}
	}
	set cmval($id,14,$j) {}
    }
    set irepeat 1
}

proc read_ARCCHM { fileid id } {
    global cmval GUI_DIR cm_ident irepeat

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read ARCCHM $cm_ident($id).  The inputs don't begin where\
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

    for {set i 2} {$i<=10} {incr i} {
	# read values 2 -> 10
	gets $fileid data
	set data [get_val $data cmval $id,$i]
    }

    # read xmin1,xmax2
    gets $fileid data
    set data [get_val $data cmval $id,11,0]
    set data [get_val $data cmval $id,11,1]

    # read zmax
    gets $fileid data
    set data [get_val $data cmval $id,12]

    # read region 1, before arcchm, props+material
    gets $fileid data
    set data [get_val $data cmval $id,13,1,0]
    set data [get_val $data cmval $id,13,1,1]
    set data [get_val $data cmval $id,13,1,2]
    set data [get_val $data cmval $id,13,1,3]
    gets $fileid data
    set data [get_str $data cmval $id,14,1]

    # read region 2, front face of arcchm, props+material
    gets $fileid data
    set data [get_val $data cmval $id,13,2,0]
    set data [get_val $data cmval $id,13,2,1]
    set data [get_val $data cmval $id,13,2,2]
    set data [get_val $data cmval $id,13,2,3]
    gets $fileid data
    set data [get_str $data cmval $id,14,2]

    # read region 3, ends of arcchm, props+material
    gets $fileid data
    set data [get_val $data cmval $id,13,3,0]
    set data [get_val $data cmval $id,13,3,1]
    set data [get_val $data cmval $id,13,3,2]
    set data [get_val $data cmval $id,13,3,3]
    gets $fileid data
    set data [get_str $data cmval $id,14,3]

    # read regions from end to end, props+material
    set j 4
    gets $fileid data
    set data [get_val $data cmval $id,13,$j,0]
    set data [get_val $data cmval $id,13,$j,1]
    set data [get_val $data cmval $id,13,$j,2]
    set data [get_val $data cmval $id,13,$j,3]
    set data [get_val $data cmval $id,13,$j,4]
    gets $fileid data
    set data [get_str $data cmval $id,14,$j]

    set irepeat 0
    if $cmval($id,13,4,4)==1 {
	set irepeat 1
	# irepeat is 1: get medium of septa and set all regions.
	gets $fileid data
	set data [get_val $data cmval $id,13,5,0]
	set data [get_val $data cmval $id,13,5,1]
	set data [get_val $data cmval $id,13,5,2]
	set data [get_val $data cmval $id,13,5,3]
	gets $fileid data
	set data [get_str $data cmval $id,14,5]
	for {set j 6} {$j<=[expr 2*$cmval($id,4)+2]} {incr j 2} {
	    set cmval($id,13,$j,0) $cmval($id,13,4,0)
	    set cmval($id,13,$j,1) $cmval($id,13,4,1)
	    set cmval($id,13,$j,2) $cmval($id,13,4,2)
	    set cmval($id,13,$j,3) $cmval($id,13,4,3)
	    set cmval($id,14,$j) $cmval($id,14,4)
	}
	for {set j 7} {$j<=[expr 2*$cmval($id,4)+2]} {incr j 2} {
	    set cmval($id,13,$j,0) $cmval($id,13,5,0)
	    set cmval($id,13,$j,1) $cmval($id,13,5,1)
	    set cmval($id,13,$j,2) $cmval($id,13,5,2)
	    set cmval($id,13,$j,3) $cmval($id,13,5,3)
	    set cmval($id,14,$j) $cmval($id,14,5)
	}
    } else {
	for {set j 5} {$j<=[expr 2*$cmval($id,4)+2]} {incr j} {
	    # read regions from end to end, props+material
	    gets $fileid data
	    set data [get_val $data cmval $id,13,$j,0]
	    set data [get_val $data cmval $id,13,$j,1]
	    set data [get_val $data cmval $id,13,$j,2]
	    set data [get_val $data cmval $id,13,$j,3]
	    gets $fileid data
	    set data [get_str $data cmval $id,14,$j]
	}
    }

    # read region back face, surrounding, x-walls props+material
    foreach j {back sur xwall} {
	gets $fileid data
	for {set i 0} {$i<4} {incr i} {
	    set data [get_val $data cmval $id,13,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,14,$j]
    }
}


proc edit_ARCCHM { id zmax } {
    global cmval GUI_DIR helvfont help_arcchm_text irepeat

    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NARCCHM.
    catch { destroy .arcchm$id }
    toplevel .arcchm$id
    wm title .arcchm$id "Edit ARCCHM, CM\#$id"
    frame .arcchm$id.frm -bd 4
    set top .arcchm$id.frm

    label $top.mainlabel -text "ARCCHM" -font $helvfont
    pack $top.mainlabel -pady 10

    #REPLACE {$MAX_N_$ARCCHM} WITH {300}
    get_1_default ARCCHM $top "maximum number of arcchm layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    frame $top.frm

    add_rmax_square $top.frm.inp0 $id,0
    add_title $top.frm.inp1 $id,1

    set w $top.frm.inp2
    frame $w -bd 4
    label $w.label -text {Distance from reference plane to\
	    lowest point on front face (cm)}
    entry $w.inp -textvariable cmval($id,2) -width 8
    pack $w.label -side left -anchor w
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x

    set w $top.frm.inp3
    frame $w -bd 4
    label $w.label -text {Radius of front face of chamber (cm)}
    entry $w.inp -textvariable cmval($id,3) -width 8
    pack $w.label -side left -anchor w
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x

    set lab(5) "Width of each ion chamber (cm)"
    set lab(6) "Width of each septum (cm)"
    set lab(7) "Thickness of chambers and septa (cm)"
    set lab(8) "Thickness of front face of chamber (cm)"
    set lab(9) "Thickness of back face of chamber (cm)"
    set lab(10) "Width of chamber wall along x (cm)"
    for {set i 5} {$i<=10} {incr i} {
	set w $top.frm.inp$i
	frame $w -bd 4
	label $w.label -text $lab($i)
	entry $w.inp -textvariable cmval($id,$i) -width 8
	pack $w.label -side left -anchor w
	pack $w.inp -anchor e -side right -fill x -expand true
	pack $w -side top -fill x
    }

    set w $top.frm.inp11-0
    frame $w -bd 4
    label $w.label -text {Minimum x dimension outside of x wall (cm)}
    entry $w.inp -textvariable cmval($id,11,0) -width 8
    pack $w.label -side left -anchor w
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x

    set w $top.frm.inp11-1
    frame $w -bd 4
    label $w.label -text {Maximum x dimension outside of x wall (cm)}
    entry $w.inp -textvariable cmval($id,11,1) -width 8
    pack $w.label -side left -anchor w
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x

    set w $top.frm.inp12
    frame $w -bd 4
    label $w.label -text {Maximum z of the CM (cm)}
    entry $w.inp -textvariable cmval($id,12) -width 8
    pack $w.label -side left -anchor w
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -side top -fill x

    set w $top.frm.inp4
    frame $w -bd 4
    frame $w.f
    frame $w.f.f
    label $w.f.f.label -text {Number of individual ion chambers}
    entry $w.f.f.inp -textvariable cmval($id,4) -width 8
    pack $w.f.f.label -anchor w -side left
    pack $w.f.f.inp -side left -fill x -expand true -padx 10
    pack $w.f.f
    checkbutton $w.f.check -text "Media same for chambers, same for septa"\
	    -onvalue 1 -offvalue 0 -variable irepeat
    pack $w.f.check
    button $w.edit -text " Define properties >> "\
	    -command "define_arcchm_props $id"
    pack $w.f -anchor w -side left
    pack $w.edit -anchor e -side right
    pack $w -anchor w -fill x -pady 10

    pack $top.frm -pady 5 -side top

    frame $top.sep3 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep3 -side top -anchor w -fill both

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .arcchm$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command \
	    "help_gif .arcchm$id.help {$help_arcchm_text} help_arcchm"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_ARCCHM $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts -pady 10
    pack $top
}

proc define_arcchm_props { id } {
    global cmval helvfont values help_arcchm_text irepeat

    if [string compare $cmval($id,4) {}]==0 {
	tk_dialog .arcchm$id.error "Error" "Please define the number of\
		ion chambers before continuing." error 0 OK
	return
    }

    # There's always at least one window for general areas.
    set x 100
    set y 100

    catch { destroy .arcchm$id.baby0 }
    toplevel .arcchm$id.baby0
    set top .arcchm$id.baby0
    wm title $top "Define ion chamber, general regions"
    wm geometry $top +$x+$y
    incr x 40
    incr y 40
    frame $top.grid -bd 4

    set lab(1) "Region in front of chamber"
    set lab(2) "Front face of chamber"
    set lab(3) "Ends of chamber"
    set lab(4) "Back face of chamber"
    set lab(5) "Region surrounding chamber"
    set lab(6) "X walls of chamber"
    set idx(1) 1
    set idx(2) 2
    set idx(3) 3
    set idx(4) back
    set idx(5) sur
    set idx(6) xwall

    set row 0
    set col 0
    for {set i 1} {$i<=6} {incr i} {
	frame $top.grid.frm$i
	set w $top.grid.frm$i
	label $w.mainlab1 -text $lab($i) -font $helvfont
	pack $w.mainlab1 -side top

	add_ecut $w.f0 $id,13,$idx($i),0
	add_pcut $w.f1 $id,13,$idx($i),1
	add_dose $w.f2 $id,13,$idx($i),2
	add_latch $w.f3 $id,13,$idx($i),3
	add_material $w.f4 $id,14,$idx($i)

	grid config $w -row $row -column $col
	incr row
	if $row>2 {
	    set row 0
	    incr col
	}
    }

    pack $top.grid -side top
    frame $top.buts
    button $top.buts.okb -text "OK" -command "destroy .arcchm$id.baby0"\
	    -relief groove -bd 8
    button $top.buts.help -text "Help" -command "help_gif .arcchm$id.help\
	    {$help_arcchm_text} help_arcchm" -relief groove -bd 8
    pack $top.buts.help $top.buts.okb -padx 10 -side left
    pack $top.buts -pady 10 -side top


    if $irepeat==0 {
	# allow a maximum of 6/window, the proceed to the next window.

	set nwindow [expr $cmval($id,4)/2]
	if [expr $cmval($id,4)%2]>0 {
	    incr nwindow
	}
	set index 3
	for {set i 1} {$i<=$nwindow} {incr i} {
	    catch { destroy .arcchm$id.baby$i }
	    toplevel .arcchm$id.baby$i
	    set top .arcchm$id.baby$i
	    wm title $top "Define ion chamber, window $i"
	    wm geometry $top +$x+$y
	    incr x 40
	    incr y 40
	    frame $top.grid -bd 4
	    set row 0
	    for {set ii 1} {$ii<=3} {incr ii} {
		incr index
		if $index>[expr 2*$cmval($id,4)+2] {break}

		frame $top.grid.f$ii -bd 4
		set w $top.grid.f$ii
		label $w.mainlab -text "Ion chamber [expr 3*($i-1)+$ii]"\
			-font $helvfont
		pack $w.mainlab -side top

		add_ecut $w.f0 $id,13,$index,0
		add_pcut $w.f1 $id,13,$index,1
		add_dose $w.f2 $id,13,$index,2
		add_latch $w.f3 $id,13,$index,3
		add_material $w.f4 $id,14,$index

		grid config $w -row $row -column 0

		incr index
		if $index>[expr 2*$cmval($id,4)+2] {break}

		frame $top.grid.f[expr $ii+3] -bd 4
		set w $top.grid.f[expr $ii+3]
		label $w.mainlab -text "Septum [expr 3*($i-1)+$ii]" -font $helvfont
		pack $w.mainlab -side top

		add_ecut $w.f0 $id,13,$index,0
		add_pcut $w.f1 $id,13,$index,1
		add_dose $w.f2 $id,13,$index,2
		add_latch $w.f3 $id,13,$index,3
		add_material $w.f4 $id,14,$index

		grid config $w -row $row -column 1
		incr row
	    }
	    pack $top.grid -side top
	    button $top.okb -text "OK" -command "destroy .arcchm$id.baby$i"\
		    -relief groove -bd 8
	    pack $top.okb -pady 10 -side top
	    if $index>[expr 2*$cmval($id,4)+2] {break}
	}
    } else {
	# same medium for all chambers, same for all septa
	catch { destroy .arcchm$id.baby}
	toplevel .arcchm$id.baby
	set top .arcchm$id.baby
	wm title $top "Define ion chambers"
	wm geometry $top +$x+$y
	frame $top.grid -bd 4

	frame $top.grid.f1 -bd 4
	set w $top.grid.f1
	label $w.mainlab -text "Ion chambers" -font $helvfont
	pack $w.mainlab -side top

	add_ecut $w.f0 $id,13,4,0
	add_pcut $w.f1 $id,13,4,1
	add_dose $w.f2 $id,13,4,2
	add_latch $w.f3 $id,13,4,3
	add_material $w.f4 $id,14,4

	grid config $w -row 0 -column 0

	frame $top.grid.f2 -bd 4
	set w $top.grid.f2
	label $w.mainlab -text "Septa" -font $helvfont
	pack $w.mainlab -side top

	add_ecut $w.f0 $id,13,5,0
	add_pcut $w.f1 $id,13,5,1
	add_dose $w.f2 $id,13,5,2
	add_latch $w.f3 $id,13,5,3
	add_material $w.f4 $id,14,5

	grid config $w -row 0 -column 1

	pack $top.grid -side top
	button $top.okb -text "OK" -command "destroy .arcchm$id.baby"\
		-relief groove -bd 8
	pack $top.okb -pady 10 -side top
    }
}

proc write_ARCCHM {fileid id} {
    global cmval cm_names cm_ident cm_type irepeat

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZSRC"
    puts $fileid "$cmval($id,3), ZRAD1"
    puts $fileid "$cmval($id,4), NUMBER OF CHAMBERS"
    puts $fileid "$cmval($id,5), WIDTHCHM"
    puts $fileid "$cmval($id,6), WIDTHSEP"
    puts $fileid "$cmval($id,7), ARCTHICK"
    puts $fileid "$cmval($id,8), FRONTHCK"
    puts $fileid "$cmval($id,9), BACKTHCK"
    puts $fileid "$cmval($id,10), WIDTH OF X WALL"
    puts $fileid "$cmval($id,11,0), $cmval($id,11,1), XMIN1 XMAX2"
    puts $fileid "$cmval($id,12), ZMAX"

    # ecut, pcut, etc.
    if $irepeat==0 {
	for {set i 1} {$i<=[expr 2*$cmval($id,4)+2]} {incr i} {
	    set str {}
	    for {set j 0 } {$j<=3} {incr j} {
		set str "$str$cmval($id,13,$i,$j), "
	    }
	    puts $fileid $str
	    puts $fileid $cmval($id,14,$i)
	}
    } else {
	for {set i 1} {$i<=3} {incr i} {
	    set str {}
	    for {set j 0 } {$j<=3} {incr j} {
		set str "$str$cmval($id,13,$i,$j), "
	    }
	    puts $fileid $str
	    puts $fileid $cmval($id,14,$i)
	}
	set str {}
	for {set j 0} {$j<=3} {incr j} {
	    set str "$str$cmval($id,13,4,$j), "
	}
	puts $fileid "$str 1"
	puts $fileid $cmval($id,14,4)
	set str {}
	for {set j 0} {$j<=3} {incr j} {
	    set str "$str$cmval($id,13,5,$j), "
	}
	puts $fileid "$str 1"
	puts $fileid $cmval($id,14,5)
    }

    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,13,back,$j), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,14,back)
    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,13,sur,$j), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,14,sur)
    set str {}
    for {set j 0 } {$j<=3} {incr j} {
	set str "$str$cmval($id,13,xwall,$j), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,14,xwall)
}

proc show_ARCCHM { id } {
    global cmval yrange zrange cm_ticks

    catch { destroy .arcchm$id.show }
    toplevel .arcchm$id.show
    wm title .arcchm$id.show "Preview"

    # find initial zrange,yrange
    set yrange(0) -$cmval($id,0)
    set yrange(1) $cmval($id,0)

    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    # min z depends on zsrc and zrad1: have to catch it or it will
    # bomb if any are undefined.
    catch {
	set zrad2 [expr $cmval($id,3)+$cmval($id,8)]
	set s [expr double($cmval($id,4)*$cmval($id,5)+\
		($cmval($id,4)-1)*$cmval($id,6))]
	set phi [expr double($s)/double($zrad2)]
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_ARCCHM $id }]==1 {
	destroy .arcchm$id.show
	tk_dialog .arcchm$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .arcchm$id.show.buts
    button .arcchm$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .arcchm$id.show yz" -relief groove -bd 8
    button .arcchm$id.show.buts.print -text "Print..." -command\
	    "print_canvas .arcchm$id.show.can 600 600" -relief groove -bd 8
    button .arcchm$id.show.buts.done -text "Done" -command\
	    "destroy .arcchm$id.show" -relief groove -bd 8
    pack .arcchm$id.show.buts.range .arcchm$id.show.buts.print\
	    .arcchm$id.show.buts.done -side left -padx 10
    pack .arcchm$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_ARCCHM { id } {
    global xrange yrange zrange cmval xscale yscale zscale l m
    global helvfont cm_ticks

    catch {destroy .arcchm$id.show.can}

    # put the canvas up
    canvas .arcchm$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 450x450 (allow
    # a x-border of 100 and z-border of 50 to show the scale)

    set width 450.0
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    add_air $id .arcchm$id.show.can $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    add_ARCCHM_yz $id $yscale $zscale $yrange(0) $zrange(0)\
	    $l $m .arcchm$id.show.can

    set curx 0
    set cury 0
    label .arcchm$id.show.can.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .arcchm$id.show.can create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .arcchm$id.show.can.xy
    bind .arcchm$id.show.can <Motion> {
	global l m yscale zscale yrange zrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$yscale+$yrange(0)]
	set cury [expr ($cury-$m)/$zscale+$zrange(0)]
        if { $curx<=$yrange(1) & $curx>=$yrange(0) & $cury<=$zrange(1) & \
                $cury>=$zrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }
    coverup $l $m $width .arcchm$id.show.can

    add_axes yz $cm_ticks($id,y) $cm_ticks($id,z) $l $m $width $yrange(0)\
	    $yrange(1) $zrange(0) $zrange(1)\
	    $yscale $zscale .arcchm$id.show.can

    pack .arcchm$id.show.can -side top -anchor n
}

proc add_ARCCHM_yz {id yscale zscale ymin zmin l m parent_w} {
    global cmval medium nmed colorlist helvfont meds_used values colornum

    # assign numbers to the media
    set nregion [expr 2*$cmval($id,4)+2]
    for {set i 1} {$i<=$nregion} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,14,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    foreach i {back sur xwall} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,14,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
       if [string compare $values(2) $medium($j)]==0 {
	   set med(air) [min_nrc $j $colornum]
	   set meds_used($j) 1
	   break
       }
   }

    set pi [expr double(3.1415926535897932384626433832795028841971)]

    # First calculate all the necessary values
    # min z depends on zsrc and zrad1
    set zrad1 $cmval($id,3)
    set zrad2 [expr $zrad1+$cmval($id,8)]
    set zrad3 [expr $zrad2+$cmval($id,7)]
    set zrad4 [expr $zrad3+$cmval($id,9)]
    set s [expr double($cmval($id,4)*$cmval($id,5)+\
	    ($cmval($id,4)-1)*$cmval($id,6))]
    set phi [expr double($s)/double($zrad2)]
    set abszmin [expr $cmval($id,2)+$cmval($id,8)-$zrad2*(1.0-cos($phi/2.0))]
    set ymin1sq [expr -$zrad2*$zrad2*cos($phi/2.0)*cos($phi/2.0)+$zrad1*$zrad1]
    if $ymin1sq<0 {
	tk_dialog .arcchmerror "Bad ymin1" "The arc chamber\
		geometry is badly defined.\
		The distance ymin1 (see diagram in User's manual) has been\
		set to 0.01cm."  warning 0 OK
	set ymin1 0.01
    } else {
	set ymin1 [expr sqrt($ymin1sq)]
    }
    set ymin2 [expr $zrad2*sin($phi/2.0)]
    set ymax1 [expr sqrt($zrad3*$zrad3-\
	    $zrad2*$zrad2*cos($phi/2.0)*cos($phi/2.0))]
    set ymax2 [expr sqrt($zrad4*$zrad4-\
	    $zrad2*$zrad2*cos($phi/2.0)*cos($phi/2.0))]

    # First object is the background rectangle for air
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($abszmin-$zmin)*$zscale+$m]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(2) [expr ($cmval($id,12)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(sur)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color -outline {}

    # Now an arc for the back face
    # The square must always be centered on the same point, i.e. the
    # centre of the circle.
    set zcentre [expr $cmval($id,2)-$cmval($id,3)]
    set y(1) [expr (-$zrad4-$ymin)*$yscale+$l]
    set y(2) [expr ($zrad4-$ymin)*$yscale+$l]
    set z(1) [expr ($zcentre-$zrad4-$zmin)*$zscale+$m]
    set z(2) [expr ($zcentre+$zrad4-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(back)]
    $parent_w create arc $y(1) $z(1) $y(2) $z(2) -start 180 -extent 180\
	    -fill $color -outline {}

    # Now an arc to fill in the ends
    set y(1) [expr (-$zrad3-$ymin)*$yscale+$l]
    set y(2) [expr ($zrad3-$ymin)*$yscale+$l]
    set z(1) [expr ($zcentre-$zrad3-$zmin)*$zscale+$m]
    set z(2) [expr ($zcentre+$zrad3-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(3)]
    $parent_w create arc $y(1) $z(1) $y(2) $z(2) -start 180 -extent 180\
	    -fill $color -outline {} -style pieslice

    # Now a series of arcs for the chambers/septa
    set startangle [expr double(270.0-90.0*$phi/$pi)]
    set chmextent [expr double(180.0*$cmval($id,5)/($zrad2*$pi))]
    set sepextent [expr double(180.0*$cmval($id,6)/($pi*$zrad2))]
    for {set i 1} {$i<$cmval($id,4)} {incr i} {
	# chamber
	set color [lindex $colorlist $med([expr 2*$i+2])]
	$parent_w create arc $y(1) $z(1) $y(2) $z(2) -start $startangle\
		-extent $chmextent -fill $color -outline {} -style pieslice
	# septum
	set startangle [expr double($startangle+$chmextent)]
	set color [lindex $colorlist $med([expr 2*$i+3])]
	$parent_w create arc $y(1) $z(1) $y(2) $z(2) -start $startangle\
		-extent $sepextent -fill $color -outline {} -style pieslice
	set startangle [expr double($startangle+$sepextent)]
    }
    # last chamber
    set color [lindex $colorlist $med($cmval($id,4))]
    $parent_w create arc $y(1) $z(1) $y(2) $z(2) -start $startangle\
	    -extent $chmextent -fill $color -outline {}

    # An arc for the front layer
    set y(1) [expr (-$zrad2-$ymin)*$yscale+$l]
    set y(2) [expr ($zrad2-$ymin)*$yscale+$l]
    set z(1) [expr ($zcentre-$zrad2-$zmin)*$zscale+$m]
    set z(2) [expr ($zcentre+$zrad2-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(2)]
    $parent_w create arc $y(1) $z(1) $y(2) $z(2) -start 180 -extent 180\
	    -fill $color -outline {}

    # An arc to finish it off, region before chamber
    set y(1) [expr (-$zrad1-$ymin)*$yscale+$l]
    set y(2) [expr ($zrad1-$ymin)*$yscale+$l]
    set z(1) [expr ($zcentre-$zrad1-$zmin)*$zscale+$m]
    set z(2) [expr ($zcentre+$zrad1-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(1)]
    $parent_w create arc $y(1) $z(1) $y(2) $z(2) -start 180 -extent 180\
	    -fill $color -outline {}

    # last a rectangle to cover up excess arcs (air gap at top)
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($abszmin-$zmin)*$zscale+$m]
    set z(2) [expr (-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rect $y(1) $z(1) $y(2) $z(2) -fill $color -outline {}
#    $parent_w create line $y(1) $z(1) $y(2) $z(1)

}

proc add_ARCCHM {id xscale zscale xmin zmin l m parent_w} {
    global cmval medium nmed colorlist helvfont meds_used values colornum

    # assign numbers to the media
    set nregion [expr 2*$cmval($id,4)+2]
    for {set i 1} {$i<=$nregion} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,14,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    foreach i {back sur xwall} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,14,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
       if [string compare $values(2) $medium($j)]==0 {
	   set med(air) [min_nrc $j $colornum]
	   set meds_used($j) 1
	   break
       }
   }

   # Three rectangles required only: One for central, 2 for x-walls,
   # 2 for front and back layers
   # Central medium is the septum at y=0
   set x(1) [expr ($cmval($id,11,0)-$xmin)*$xscale+$l]
   set x(2) [expr ($cmval($id,11,1)-$xmin)*$xscale+$l]
   set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
   set z(2) [expr ($cmval($id,2)+$cmval($id,8)+$cmval($id,7)\
	   +$cmval($id,9)-$zmin)*$zscale+$m]
   set color [lindex $colorlist $med([expr $cmval($id,4)+3])]
   $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

   # front layer
   set x(1) [expr ($cmval($id,11,0)-$xmin)*$xscale+$l]
   set x(2) [expr ($cmval($id,11,1)-$xmin)*$xscale+$l]
   set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
   set z(2) [expr ($cmval($id,2)+$cmval($id,8)-$zmin)*$zscale+$m]
   set color [lindex $colorlist $med(2)]
   $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

   # back layer
   set x(1) [expr ($cmval($id,11,0)-$xmin)*$xscale+$l]
   set x(2) [expr ($cmval($id,11,1)-$xmin)*$xscale+$l]
   set z(1) [expr ($cmval($id,2)+$cmval($id,8)+$cmval($id,7)-$zmin)*$zscale+$m]
   set z(2) [expr ($cmval($id,2)+$cmval($id,8)+$cmval($id,7)\
	   +$cmval($id,9)-$zmin)*$zscale+$m]
   set color [lindex $colorlist $med(back)]
   $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

   # x-walls
   set x(1) [expr ($cmval($id,11,0)-$xmin)*$xscale+$l]
   set x(2) [expr ($cmval($id,11,0)+$cmval($id,10)-$xmin)*$xscale+$l]
   set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
   set z(2) [expr ($cmval($id,2)+$cmval($id,8)+$cmval($id,7)\
	   +$cmval($id,9)-$zmin)*$zscale+$m]
   set color [lindex $colorlist $med(xwall)]
   $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

   set x(1) [expr ($cmval($id,11,1)-$cmval($id,10)-$xmin)*$xscale+$l]
   set x(2) [expr ($cmval($id,11,1)-$xmin)*$xscale+$l]
   set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
   set z(2) [expr ($cmval($id,2)+$cmval($id,8)+$cmval($id,7)\
	   +$cmval($id,9)-$zmin)*$zscale+$m]
   set color [lindex $colorlist $med(xwall)]
   $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

}


