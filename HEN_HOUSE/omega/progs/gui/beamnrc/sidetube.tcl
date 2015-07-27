
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: SIDETUBE
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
#  Author:          Joanne Treurniet, 1999
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


proc init_SIDETUBE { id } {
    global cmval

    for {set i 0} {$i<5} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,5,0) {}
    set cmval($id,5,1) {}
    set cmval($id,6) {}

    for {set j 0} {$j<=20} {incr j} {
	set cmval($id,7,$j) {}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,8,$k,$j) {}
	}
	set cmval($id,9,$j) {}
    }
}

proc read_SIDETUBE { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read SIDETUBE $cm_ident($id).  The inputs don't begin where\
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

    # read zmin, 2
    gets $fileid data
    set data [get_val $data cmval $id,2]

    # read zthick 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read zcyl 4
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # read xmin, xmax 5
    gets $fileid data
    set data [get_val $data cmval $id,5,0]
    set data [get_val $data cmval $id,5,1]

    # read nsidetube 6
    gets $fileid data
    set data [get_val $data cmval $id,6]

    # read radii 7
    gets $fileid data
    for {set i 1} {$i<=$cmval($id,6)} {incr i} {
	set data [get_val $data cmval $id,7,$i]
    }

    # read ecut etc. 8,9
    for {set i 1} {$i<=[expr $cmval($id,6)+1]} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,8,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,9,$i]
    }
}

proc edit_SIDETUBE { id zmax } {
    global cmval GUI_DIR helvfont values help_sidetube_text

    catch { destroy .sidet$id }
    toplevel .sidet$id
    wm title .sidet$id "Edit SIDETUBE, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NSIDETUBE.
    frame .sidet$id.frm -bd 4
    set top .sidet$id.frm

    label $top.mainlabel -text "Sidetube" -font $helvfont
    pack $top.mainlabel -pady 10

    # SIDETUBE_macros.mortran:REPLACE {$MAX_N_$SIDETUBE} WITH {80}
    # SIDETUBE_macros.mortran:REPLACE {$MAX_N_SC_$SIDETUBE} WITH {80}
    get_1_default SIDETUBE $top "maximum number of coaxial cylinders"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Thickness of CM (cm)}
    entry $top.inp3.inp -textvariable cmval($id,3) -width 8
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Z position of axis of coaxial cylinders (cm)}
    entry $top.inp4.inp -textvariable cmval($id,4) -width 8
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.inp5 -bd 4
    label $top.inp5.label -text {Lower X edge of cylinders (cm)}
    entry $top.inp5.inp -textvariable cmval($id,5,0) -width 8
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -anchor e -side right -fill x -expand true
    pack $top.inp5 -side top -fill x

    frame $top.inp6 -bd 4
    label $top.inp6.label -text {Upper X edge of cylinders (cm)}
    entry $top.inp6.inp -textvariable cmval($id,5,1) -width 8
    pack $top.inp6.label -anchor w -side left
    pack $top.inp6.inp -anchor e -side right -fill x -expand true
    pack $top.inp6 -side top -fill x

    frame $top.inp7 -bd 4
    label $top.inp7.label -text {Number of coaxial cylinders}
    entry $top.inp7.inp -textvariable cmval($id,6) -width 8
    button $top.inp7.nextproc -text " >> "\
	    -command "define_sidetube $id"
    pack $top.inp7.label -anchor w -side left
    pack $top.inp7.inp -side left -padx 10
    pack $top.inp7.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp7 -side top -fill x

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .sidet$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .sidet$id.help {$help_sidetube_text} help_sidetube"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_SIDETUBE $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts
    pack $top
}

proc define_sidetube { id } {
    global cmval helvfont values

    # allow a maximum of 3/window, the proceed to the next window.
    set ntube $cmval($id,6)
    set nwindow [expr $ntube/3]
    if [expr $ntube%3]>0 {
	incr nwindow
    }
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .sidet$id.baby$i }
	toplevel .sidet$id.baby$i
	set top .sidet$id.baby$i
	wm title $top "Define sidetube, window $i"
	for {set ii 1} {$ii<=3} {incr ii} {
	    frame $top.f$i-$ii
	    set index [expr 3*($i-1)+$ii]
	    if $index>$ntube {break}

	    # define the 6 values that go with each tube

	    label $top.f$i-$ii.lab -text "Cylinder $index" -font $helvfont
	    pack $top.f$i-$ii.lab -side top -pady 5

	    frame $top.f$i-$ii.f0
	    set w $top.f$i-$ii.f0
	    label $w.lab -text "Outer radius of cylinder $index (cm)"
	    pack $w.lab -anchor w -side left
	    entry $w.ent -textvariable cmval($id,7,$index) -width 10
	    pack $w.ent -anchor e -side right -fill x -expand true
	    pack $top.f$i-$ii.f0 -side top -fill x

	    add_ecut $top.f$i-$ii.f1 $id,8,0,$index
	    add_pcut $top.f$i-$ii.f2 $id,8,1,$index
	    add_dose $top.f$i-$ii.f3 $id,8,2,$index
	    add_latch $top.f$i-$ii.f4 $id,8,3,$index
	    add_material $top.f$i-$ii.f5 $id,9,$index

	    frame $top.f$i-$ii.sep -width 100 -bd 4 -height 2 -relief groove
	    pack $top.f$i-$ii.sep -pady 10 -fill x

	    pack $top.f$i-$ii -side top
	}
	if $index<$ntube {
	    button $top.okb -text "OK" -command "destroy $top"\
		    -relief groove -bd 8
	    pack $top.okb -pady 10
	}
	if $index>$ntube {break}
    }
    frame $top.flast

    label $top.flast.lab -text "Region containing the cylinders" \
	    -font $helvfont
    pack $top.flast.lab -side top -pady 5

    set index [expr $cmval($id,6)+1]

    add_ecut $top.flast.f1 $id,8,0,$index
    add_pcut $top.flast.f2 $id,8,1,$index
    add_dose $top.flast.f3 $id,8,2,$index
    add_latch $top.flast.f4 $id,8,3,$index
    add_material $top.flast.f5 $id,9,$index

    pack $top.flast -side top
    button $top.okb -text "OK" -command "destroy $top"\
	    -relief groove -bd 8
    pack $top.okb -pady 10
}

proc write_SIDETUBE {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZMIN"
    puts $fileid "$cmval($id,3), ZTHICK"
    puts $fileid "$cmval($id,4), ZCYL"
    puts $fileid "$cmval($id,5,0), $cmval($id,5,1), XMIN, XMAX"
    puts $fileid "$cmval($id,6), # CYLINDERS"

    set str {}
    for {set i 1} {$i<=$cmval($id,6)} {incr i} {
	set str "$str$cmval($id,7,$i), "
    }
    puts $fileid $str

    for {set i 1} {$i<=[expr $cmval($id,6)+1]} {incr i} {
	set str {}
	set str "$str$cmval($id,8,0,$i), "
	set str "$str$cmval($id,8,1,$i), "
	set str "$str$cmval($id,8,2,$i), "
	set str "$str$cmval($id,8,3,$i), "
	puts $fileid $str
	puts $fileid $cmval($id,9,$i)
    }
}


proc show_SIDETUBE { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .sidet$id.show }
    toplevel .sidet$id.show
    wm title .sidet$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange, zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set yrange(0) -$cmval($id,0)
	set yrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_SIDETUBE $id }]==1 {
	destroy .sidet$id.show
	tk_dialog .sidet$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .sidet$id.show.buts
    button .sidet$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .sidet$id.show xyz" -relief groove -bd 8
    button .sidet$id.show.buts.print1 -text "Print xz..." -command\
	    "print_canvas .sidet$id.show.can.one 500 500" -relief groove -bd 8
    button .sidet$id.show.buts.print2 -text "Print yz..." -command\
	    "print_canvas .sidet$id.show.can.two 500 500" -relief groove -bd 8
    button .sidet$id.show.buts.done -text "Done" -command\
	    "destroy .sidet$id.show" -relief groove -bd 8
    pack .sidet$id.show.buts.range .sidet$id.show.buts.print1\
	    .sidet$id.show.buts.print2 .sidet$id.show.buts.done\
	    -side left -padx 10
    pack .sidet$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_SIDETUBE { id } {
    global zrange yrange xrange xscale yscale zscale l m helvfont
    global cm_ticks

    catch { destroy .sidet$id.show.can }

    # put the canvas up
    set ncan 2
    set width 350.0
    set canwidth 500.0
    canvas .sidet$id.show.can -width [expr $ncan*($width+150.0)]\
	    -height [expr ($width+150.0)]

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    # left canvas, x/z cross-section
    set l 100.0
    set m 50.0

    canvas .sidet$id.show.can.one -width $canwidth -height $canwidth

    add_air $id .sidet$id.show.can.one $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_SIDETUBE $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .sidet$id.show.can.one

    coverup $l $m $width .sidet$id.show.can.one

    set curx 0
    set cury 0
    label .sidet$id.show.can.one.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .sidet$id.show.can.one create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .sidet$id.show.can.one.xy
    bind .sidet$id.show.can.one <Motion> {
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

    add_axes xz $cm_ticks($id,x) $cm_ticks($id,z) $l $m $width $xrange(0)\
	    $xrange(1) $zrange(0) $zrange(1)\
	    $xscale $zscale .sidet$id.show.can.one

    .sidet$id.show.can create window [expr $canwidth/2.0]\
	    [expr $canwidth/2.0] -window .sidet$id.show.can.one

    # right canvas, YZ cross-section

    canvas .sidet$id.show.can.two -width $canwidth -height $canwidth

    add_air $id .sidet$id.show.can.two $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    add_SIDETUBE_yz $id $yscale $zscale $yrange(0) $zrange(0)\
	    $l $m .sidet$id.show.can.two

    coverup $l $m $width .sidet$id.show.can.two

    label .sidet$id.show.can.two.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .sidet$id.show.can.two create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .sidet$id.show.can.two.xy
    bind .sidet$id.show.can.two <Motion> {
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

    add_axes yz $cm_ticks($id,y) $cm_ticks($id,z) $l $m $width $yrange(0)\
	    $yrange(1) $zrange(0) $zrange(1)\
	    $yscale $zscale .sidet$id.show.can.two

    .sidet$id.show.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .sidet$id.show.can.two

    pack .sidet$id.show.can -side top -anchor n
}

proc add_SIDETUBE {id xscale zscale xmin zmin l m parent_w} {
    global colorlist medium nmed cmval meds_used colornum

    # assign numbers to the media, and air
    for {set i 1} {$i<=[expr $cmval($id,6)+1]} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,9,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }

    # left canvas, xz cross-section
    # rectangle for outside the tube
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,2)+$cmval($id,3)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med([expr $cmval($id,6)+1])]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    set zcyl $cmval($id,4)
    # for each layer, put on rectangle of medium, starting with outer so
    # that the smaller ones go on top.

    set x(1) [expr ($cmval($id,5,0)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,5,1)-$xmin)*$xscale+$l]
    for {set i $cmval($id,6)} {$i>=1} {incr i -1} {
	set z(1) [expr ($zcyl+$cmval($id,7,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($zcyl-$cmval($id,7,$i)-$zmin)*$zscale+$m]
	set color [lindex $colorlist $med($i)]
	$parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color\
		-outline {}
    }
}

proc add_SIDETUBE_yz {id yscale zscale ymin zmin l m parent_w} {
    global colorlist medium nmed cmval colornum

    # assign numbers to the media, and air
    for {set i 1} {$i<=[expr $cmval($id,6)+1]} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,9,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		break
	    }
	}
    }

    # right canvas, YZ cross-section
    # rectangle for outside medium
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(2) [expr ($cmval($id,2)+$cmval($id,3)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med([expr $cmval($id,6)+1])]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color -outline {}

    # for each layer, put on 2 rectangles to show the apps at x=0

    set zcyl $cmval($id,4)
    for {set i $cmval($id,6)} {$i>=1} {incr i -1} {
	set z(1) [expr ($zcyl+$cmval($id,7,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($zcyl-$cmval($id,7,$i)-$zmin)*$zscale+$m]
	set y(1) [expr (-$cmval($id,7,$i)-$ymin)*$yscale+$l]
	set y(2) [expr ($cmval($id,7,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med($i)]
	$parent_w create oval $y(1) $z(1) $y(2) $z(2)\
		-fill $color -outline {}
    }
}

