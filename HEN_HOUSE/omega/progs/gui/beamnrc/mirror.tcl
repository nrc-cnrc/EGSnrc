
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: MIRROR
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


proc init_MIRROR { id } {
    global cmval

    set cmval($id,0) {}
    set cmval($id,1) {}
    set cmval($id,2,0) {}
    set cmval($id,2,1) {}
    set cmval($id,3,0) {}
    set cmval($id,3,1) {}
    set cmval($id,4) {}
    for {set j 0} {$j<=20} {incr j} {
	set cmval($id,5,$j) {}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,6,$k,$j) {}
	}
	set cmval($id,7,$j) {}
    }
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,8,$k) {}
    }
    set cmval($id,9) {}
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,10,$k) {}
    }
    set cmval($id,11) {}

}

proc read_MIRROR { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read MIRROR $cm_ident($id).  The inputs don't begin where\
		they should.  There is either an error in the CM preceding\
		this, or the accelerator module file is not the correct one\
		for use with this input file." error 0 OK
	return -code break
	# breaks out of the loops this procedure was called from.
    }

    # read rmax 0
    gets $fileid data
    set data [get_val $data cmval $id,0]

    # read title
    gets $fileid data
    set cmval($id,1) $data

    # read zmin, zthick
    gets $fileid data
    for {set j 0} {$j<=1} {incr j} {
	set data [get_val $data cmval $id,2,$j]
    }

    # read xfmin, xbmin
    gets $fileid data
    for {set j 0} {$j<=1} {incr j} {
	set data [get_val $data cmval $id,3,$j]
    }

    # read nmirror
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # read layer thicknesses
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	gets $fileid data
	set data [get_val $data cmval $id,5,$i]
    }

    # For ecut etc, for each layer
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,6,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,7,$i]
    }

    # read ecut etc. for region behind mirror
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,8,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,9]

    # read ecut etc. for region in front of mirror
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,10,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,11]
}

proc edit_MIRROR { id zmax } {
    global cmval GUI_DIR helvfont values omega help_mirror_text

    catch { destroy .mirror$id }
    toplevel .mirror$id
    wm title .mirror$id "Edit MIRROR, CM \#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NMIRROR.
    frame .mirror$id.frm -bd 4
    set top .mirror$id.frm

    label $top.mainlabel -text "Mirror" -font $helvfont
    pack $top.mainlabel -pady 10

    #MIRROR_macros.mortran:REPLACE {$MAX_N_$MIRROR} WITH {10}
    get_1_default MIRROR $top "maximum number of layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2,0

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Z-direction span}
    entry $top.inp3.inp -textvariable cmval($id,2,1) -width 10
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {X-coordinate of mirror at top, \
	    from Z-axis to first layer (cm)}
    entry $top.inp4.inp -textvariable cmval($id,3,0) -width 10
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.inp5 -bd 4
    label $top.inp5.label -text {X-coordinate of mirror at bottom, \
	    from Z-axis to first layer (cm)}
    entry $top.inp5.inp -textvariable cmval($id,3,1) -width 10
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -anchor e -side right -fill x -expand true
    pack $top.inp5 -side top -fill x

    set mirror_angle "0.0000"
    frame $top.calc -bd 4
    button $top.calc.angleb -text "Re-calculate angle between axis and mirror"\
	    -command "calc_mirror_angle $id"
    label $top.calc.anglet -text "$mirror_angle degrees" -font $helvfont
    pack $top.calc.angleb $top.calc.anglet -side left -padx 10
    pack $top.calc -side top

    calc_mirror_angle $id

    frame $top.inp6 -bd 4
    label $top.inp6.label -text {Number of layers in mirror}
    entry $top.inp6.inp -textvariable cmval($id,4) -width 10
    button $top.inp6.nextproc -text " Define layers >> "\
	    -command "define_mirrors $id"
    pack $top.inp6.label -anchor w -side left
    pack $top.inp6.inp -side left -padx 10
    pack $top.inp6.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp6 -side top -fill x

    # Now put on the back and front of mirror properties entry boxes.

    frame $top.inp7 -bd 4
    frame $top.inp7.back

    label $top.inp7.back.lab -text "Region behind mirror:" -font $helvfont
    pack $top.inp7.back.lab -pady 10

    add_ecut $top.inp7.back.f1 $id,8,0
    add_pcut $top.inp7.back.f2 $id,8,1
    add_dose $top.inp7.back.f3 $id,8,2
    add_latch $top.inp7.back.f4 $id,8,3
    add_material $top.inp7.back.f6 $id,9

    pack $top.inp7.back -side top

    frame $top.inp7.front

    label $top.inp7.front.lab -text "Region in front of mirror:" -font $helvfont
    pack $top.inp7.front.lab -pady 10

    add_ecut $top.inp7.front.f1 $id,10,0
    add_pcut $top.inp7.front.f2 $id,10,1
    add_dose $top.inp7.front.f3 $id,10,2
    add_latch $top.inp7.front.f4 $id,10,3
    add_material $top.inp7.front.f6 $id,11

    pack $top.inp7.back $top.inp7.front -side left -padx 5

    pack $top.inp7 -side top -pady 5

    frame $top.buts -bd 4
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .mirror$id.help {$help_mirror_text} help_mirror"\
	    -relief groove -bd 8
    button $top.buts.okb -text "OK" -command "destroy .mirror$id"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_MIRROR $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts -side top -pady 10
    pack $top
}

proc calc_mirror_angle { id } {
    global cmval helvfont
    if [catch {set x1 [expr $cmval($id,3,0)]}]==1 { return}
    if [catch {set x2 [expr $cmval($id,3,1)]}]==1 { return}
    if [catch {set zT [expr $cmval($id,2,1)]}]==1 { return}
    set mirror_angle [expr atan2(abs($x1 - $x2), $zT)]
    set mirror_angle [expr $mirror_angle * 180 / 3.14159]
    .mirror$id.frm.calc.anglet configure -text "$mirror_angle degrees"
}

proc define_mirrors { id } {
    global cmval helvfont values

    # allow a maximum of 4/window, the proceed to the next window.
    set nmirror $cmval($id,4)
    set nwindow [expr $nmirror/4]
    if [expr $nmirror%4]>0 {
	incr nwindow
    }
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .mirror$id.baby$i }
	toplevel .mirror$id.baby$i
	set top .mirror$id.baby$i
	wm title $top "Define mirror, window $i"
	for {set ii 1} {$ii<=4} {incr ii} {
	    frame $top.f$i-$ii
	    set index [expr 4*($i-1)+$ii]
	    if $index>$nmirror {break}

	    # define the 6 values that go with each slab

	    label $top.f$i-$ii.lab -text "Mirror layer $index" -font $helvfont
	    pack $top.f$i-$ii.lab -side top

	    frame $top.f$i-$ii.f0
	    set w $top.f$i-$ii.f0
	    label $w.lab -text "Mirror layer thickness (cm)"
	    pack $w.lab -anchor w -side left
	    entry $w.ent -textvariable cmval($id,5,$index) -width 10
	    pack $w.ent -anchor e -side right -fill x -expand true
	    pack $top.f$i-$ii.f0 -side top -fill x

	    add_ecut $top.f$i-$ii.f1 $id,6,0,$index
	    add_pcut $top.f$i-$ii.f2 $id,6,1,$index
	    add_dose $top.f$i-$ii.f3 $id,6,2,$index
	    add_latch $top.f$i-$ii.f4 $id,6,3,$index
	    add_material $top.f$i-$ii.f5 $id,7,$index

	    pack $top.f$i-$ii -side top

	    frame $top.f$i-$ii.sep -bd 4 -width 100 -height 2 -relief groove
	    pack $top.f$i-$ii.sep -pady 10 -fill x
	}
	button $top.okb -text "OK" -command "destroy .mirror$id.baby$i"\
		-relief groove -bd 8
	pack $top.okb -pady 10
	if [expr 4*($i-1)+$ii]>$nmirror {break}
    }
}

proc write_MIRROR {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), ZMIN, ZTHICK"
    puts $fileid "$cmval($id,3,0), $cmval($id,3,1), XFMIN, XBMIN"
    puts $fileid "$cmval($id,4), # LAYERS"
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	puts $fileid "$cmval($id,5,$i),  thickness of layer $i"
    }
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set str {}
	set str "$str$cmval($id,6,0,$i), "
	set str "$str$cmval($id,6,1,$i), "
	set str "$str$cmval($id,6,2,$i), "
	set str "$str$cmval($id,6,3,$i), "
	puts $fileid $str
	puts $fileid $cmval($id,7,$i)
    }
    # front
    set str {}
    set str "$str$cmval($id,8,0), "
    set str "$str$cmval($id,8,1), "
    set str "$str$cmval($id,8,2), "
    set str "$str$cmval($id,8,3), "
    puts $fileid $str
    puts $fileid $cmval($id,9)
    # back
    set str {}
    set str "$str$cmval($id,10,0), "
    set str "$str$cmval($id,10,1), "
    set str "$str$cmval($id,10,2), "
    set str "$str$cmval($id,10,3), "
    puts $fileid $str
    puts $fileid $cmval($id,11)
}


proc show_MIRROR { id } {
    global cmval xrange zrange cm_ticks

    catch { destroy .mirror$id.show }
    toplevel .mirror$id.show
    wm title .mirror$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange, zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_MIRROR $id }]==1 {
	destroy .mirror$id.show
	tk_dialog .mirror$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .mirror$id.show.buts
    button .mirror$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .mirror$id.show xz" -relief groove -bd 8
    button .mirror$id.show.buts.print -text "Print..." -command\
	    "print_canvas .mirror$id.show.can 600 650" -relief groove -bd 8
    button .mirror$id.show.buts.done -text "Done" -command\
	    "destroy .mirror$id.show" -relief groove -bd 8
    pack .mirror$id.show.buts.range .mirror$id.show.buts.print\
	    .mirror$id.show.buts.done -side left -padx 10
    pack .mirror$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_MIRROR { id } {
    global helvfont zrange xrange zscale xscale l m cm_ticks

    catch { destroy .mirror$id.show.can }

    # put the canvas up
    set ncan 1
    set width 450.0
    canvas .mirror$id.show.can -width [expr $ncan*($width+150)]\
	    -height [expr $ncan*($width+200)]

    set xscale [expr $width/abs($xrange(1)-$xrange(0))]
    set zscale [expr $width/abs($zrange(1)-$zrange(0))]
    set l 100.0
    set m 50.0

    add_air $id .mirror$id.show.can $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_MIRROR $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .mirror$id.show.can

    coverup $l $m $width .mirror$id.show.can

    set curx 0
    set cury 0
    label .mirror$id.show.can.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .mirror$id.show.can create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .mirror$id.show.can.xy
    bind .mirror$id.show.can <Motion> {
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
	    $xscale $zscale .mirror$id.show.can

    pack .mirror$id.show.can -side top -anchor n
}


proc add_MIRROR {id xscale zscale xmin zmin l m parent_w } {
    global colorlist medium nmed cmval meds_used colornum

    # assign numbers to the media, wire and air
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(behind) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,9) $medium($j)]==0 {
	    set med(behind) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(front) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if { [string compare $cmval($id,11) $medium($j)]==0 } {
	    set med(front) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # rectangle for region behind
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(behind)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color\
	    -outline {}

    # get angle between axis and mirror
    set x1 [expr $cmval($id,3,0)]
    set x2 [expr $cmval($id,3,1)]
    set zT [expr $cmval($id,2,1)]
    set angle [expr atan2(abs($x1 - $x2), $zT)]
    set angle [expr cos($angle)]

    set xtop1 $cmval($id,3,0)
    set xbot1 $cmval($id,3,1)
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]

    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set thick [expr $cmval($id,5,$i)/$angle]
	set xtop2 [expr $xtop1+$thick]
	set xbot2 [expr $xbot1+$thick]
	set x(1) [expr ($xtop1-$xmin)*$xscale+$l]
	set x(2) [expr ($xtop2-$xmin)*$xscale+$l]
	set x(3) [expr ($xbot1-$xmin)*$xscale+$l]
	set x(4) [expr ($xbot2-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med($i)]
	$parent_w create poly $x(1) $z(1) $x(2) $z(1) $x(4) $z(2)\
		$x(3) $z(2) -fill $color
	set xtop1 $xtop2
	set xbot1 $xbot2
    }

    # polygon for front: xtop2, xbot2 are the starting points, z(1), z(2) are ok
    set x(1) [expr ($xtop2-$xmin)*$xscale+$l]
    set x(2) [expr ($xbot2-$xmin)*$xscale+$l]
    set x(3) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(front)]
    $parent_w create poly $x(1) $z(1) $x(3) $z(1) $x(3) $z(2)\
	    $x(2) $z(2) -fill $color -outline {}

}

proc add_MIRROR_yz {id yscale zscale ymin zmin l m parent_w } {
    global colorlist medium nmed cmval meds_used colornum

    # assign numbers to the media, wire and air
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(behind) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,9) $medium($j)]==0 {
	    set med(behind) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(front) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if { [string compare $cmval($id,11) $medium($j)]==0 } {
	    set med(front) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # get angle between axis and mirror
    set x1 [expr $cmval($id,3,0)]
    set x2 [expr $cmval($id,3,1)]
    set zT [expr $cmval($id,2,1)]
    set angle [expr atan2(abs($x1 - $x2), $zT)]
    set tanangle [expr abs($x1-$x2)/$zT]

    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]

    # rectangle for region behind
    set zbehind [expr $x1/$tanangle]
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$zbehind-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(behind)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
	    -outline {}

    set z1 $zbehind
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	# for each layer calculate the z-thickness, dz
	set dz [expr $cmval($id,5,$i)/sin($angle)]
	set z2 [expr $z1+$dz]
	set z(1) [expr ($cmval($id,2,0)+$z1-$zmin)*$zscale+$m]
	set z(2) [expr ($cmval($id,2,0)+$z2-$zmin)*$zscale+$m]
	set color [lindex $colorlist $med($i)]
	$parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
		-outline {}
	set z1 $z2
    }

    # now the material in front of the mirror:
    set z(1) [expr ($cmval($id,2,0)+$z1-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(behind)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
	    -outline {}

}








