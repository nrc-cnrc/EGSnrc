
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: MESH
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


proc init_MESH { id } {
    global cmval

    set cmval($id,0) {}
    set cmval($id,1) {}
    set cmval($id,2) {}
    set cmval($id,3,0) {}
    set cmval($id,3,1) {}
    set cmval($id,3,2) {}
    set cmval($id,3,3) {}
    set cmval($id,4,0) {}
    set cmval($id,4,1) {}
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,5,$k) {}
	set cmval($id,6,$k) {}
    }
    set cmval($id,7) {}
}

proc read_MESH { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read MESH $cm_ident($id).  The inputs don't begin where\
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

    # read x_air_width etc
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,3,$j]
    }

    # read xtotal, ytotl
    gets $fileid data
    for {set j 0} {$j<=1} {incr j} {
	set data [get_val $data cmval $id,4,$j]
    }

    # read ecut etc for air inside and surrounding mesh
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,5,$j]
    }

    # read ecut etc for wire region
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,6,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,7]
}

proc edit_MESH { id zmax } {
    global cmval GUI_DIR helvfont values help_mesh_text

    catch { destroy .mesh$id }
    toplevel .mesh$id
    wm title .mesh$id "Edit MESH, CM \#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF
    frame .mesh$id.frm -bd 4
    set top .mesh$id.frm

    label $top.mainlabel -text "Mesh" -font $helvfont
    pack $top.mainlabel -pady 10

    # MESH_macros.mortran:REPLACE {$MAX_N_$MESH} WITH  {5};
    # "maximum number of local regions"
    get_1_default MESH $top "maximum number of local regions"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Half-width of outer X dimension of mesh (cm)}
    entry $top.inp3.inp -textvariable cmval($id,4,0) -width 10
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Half-width of outer Y dimension of mesh (cm)}
    entry $top.inp4.inp -textvariable cmval($id,4,1) -width 10
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.bottom
    set top $top.bottom
    frame $top.left
    set w $top.left

    label $w.label -text "Air region" -font $helvfont
    pack $w.label -side top

    frame $w.f1 -bd 4
    label $w.f1.label -text {X width of each air region in mesh (cm)}
    entry $w.f1.inp -textvariable cmval($id,3,0) -width 10
    pack $w.f1.label -anchor w -side left
    pack $w.f1.inp -anchor e -side right -fill x -expand true
    pack $w.f1 -side top -fill x

    frame $w.f2 -bd 4
    label $w.f2.label -text {Y width of each air region in mesh (cm)}
    entry $w.f2.inp -textvariable cmval($id,3,1) -width 10
    pack $w.f2.label -anchor w -side left
    pack $w.f2.inp -anchor e -side right -fill x -expand true
    pack $w.f2 -side top -fill x

    add_ecut $w.f3 $id,5,0
    add_pcut $w.f4 $id,5,1
    add_dose $w.f5 $id,5,2
    add_latch $w.f6 $id,5,3

    pack $w -side left -anchor n
    frame $top.right
    set w $top.right

    label $w.label -text "Wire region" -font $helvfont
    pack $w.label -side top

    frame $w.f1 -bd 4
    label $w.f1.label -text {X-Y width of wire in the mesh (cm)}
    entry $w.f1.inp -textvariable cmval($id,3,2) -width 10
    pack $w.f1.label  -anchor w -side left
    pack $w.f1.inp -anchor e -side right -fill x -expand true
    pack $w.f1 -side top -fill x

    frame $w.f2 -bd 4
    label $w.f2.label -text {Z thickness of wire in the mesh (cm)}
    entry $w.f2.inp -textvariable cmval($id,3,3) -width 10
    pack $w.f2.label -anchor w -side left
    pack $w.f2.inp -anchor e -side right -fill x -expand true
    pack $w.f2 -side top -fill x

    add_ecut $w.f3 $id,6,0
    add_pcut $w.f4 $id,6,1
    add_dose $w.f5 $id,6,2
    add_latch $w.f6 $id,6,3
    add_material $w.f7 $id,7

    pack $w -side right -padx 5 -anchor n

    pack $top -side top -pady 5

    set top .mesh$id.frm

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .mesh$id" -relief groove\
	    -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .mesh$id.help {$help_mesh_text} help_mesh"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_MESH $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts -pady 5
    pack $top
}

proc write_MESH {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), MESH"
    set str {}
    for {set i 0} {$i<=3} {incr i} {
	set str "$str$cmval($id,3,$i), "
    }
    puts $fileid "$str, AIR AND WIRE WIDTHS"

    puts $fileid "$cmval($id,4,0), $cmval($id,4,1), XTOTAL, YTOTAL"

    set str {}
    for {set i 0} {$i<=3} {incr i} {
	set str "$str$cmval($id,5,$i), "
    }
    puts $fileid $str
    set str {}
    for {set i 0} {$i<=3} {incr i} {
	set str "$str$cmval($id,6,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,7)
}


proc show_MESH { id } {
    global cmval helvfont medium nmed xrange yrange med cm_ticks

    catch { destroy .mesh$id.show }
    toplevel .mesh$id.show
    wm title .mesh$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6

    # have to make an initial xrange, yrange:
    set xrange(0) -$cmval($id,0)
    set xrange(1) $cmval($id,0)
    set yrange(0) -$cmval($id,0)
    set yrange(1) $cmval($id,0)

    if [catch { draw_MESH $id }]==1 {
	destroy .mesh$id.show
	tk_dialog .mesh$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .mesh$id.show.buts
    button .mesh$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .mesh$id.show xy" -relief groove -bd 8
    button .mesh$id.show.buts.print -text "Print..." -command\
	    "print_canvas .mesh$id.show.can 600 650" -relief groove -bd 8
    button .mesh$id.show.buts.done -text "Done" -command\
	    "destroy .mesh$id.show" -relief groove -bd 8
    pack .mesh$id.show.buts.range .mesh$id.show.buts.print\
	    .mesh$id.show.buts.done -side left -padx 10
    pack .mesh$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_MESH { id } {
    global helvfont yrange xrange l m xscale yscale cm_ticks

    catch { destroy .mesh$id.show.can }

    # put the canvas up
    set ncan 1
    set width 450
    canvas .mesh$id.show.can -width [expr $ncan*($width+150)]\
	    -height [expr $ncan*($width+200)]

    set xscale [expr $width/abs($xrange(1)-$xrange(0))]
    set yscale [expr $width/abs($yrange(1)-$yrange(0))]

    set l 100.0
    set m 100.0

    add_MESH_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .mesh$id.show.can

    coverup $l $m $width .mesh$id.show.can

    set curx 0
    set cury 0
    label .mesh$id.show.can.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .mesh$id.show.can create window [expr $l/2+$width] [expr 3*$m/2+$width]\
	    -window .mesh$id.show.can.xy
    bind .mesh$id.show.can <Motion> {
	global l m xscale yscale xrange yrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$xscale+$xrange(0)]
	set cury [expr ($cury-$m)/$yscale+$yrange(0)]
        if { $curx<=$xrange(1) & $curx>=$xrange(0) & $cury<=$yrange(1) & \
                $cury>=$yrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .mesh$id.show.can

    pack .mesh$id.show.can -side top -anchor n
}

proc add_MESH_xy {id xscale yscale xmin ymin l m parent_w} {
    global cmval colorlist medium nmed helvfont values colornum

    # assign numbers to the media, wire and air
    set med(wire) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,7) $medium($j)]==0 {
	    set med(wire) [min_nrc $j $colornum]
	    break
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    break
	}
    }

    # rectangle for air
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color

    # get number of x,y holes
    set wire $cmval($id,3,2)
    set xwidth $cmval($id,3,0)
    set ywidth $cmval($id,3,1)
    set nx [expr 2*($cmval($id,4,0)-0.5*$xwidth-$wire)/($xwidth+$wire)]
    if [expr $nx-int($nx)]>0 {
	# not an integer, have to change $cmval($id,4,0)
	set nx [expr int($nx)+1]
	# if not an even number, put it back
	if [expr $nx%2]!=0 { set nx [expr $nx-1] }
	set cmval($id,4,0) [expr 0.5*$xwidth+$wire+$nx*0.5*($xwidth+$wire)]
	$parent_w create text 325 20 -text "The total x-width of\
		the mesh has been changed to $cmval($id,4,0) cm."\
		-font $helvfont
    }
    set ny [expr 2*($cmval($id,4,1)-0.5*$ywidth-$wire)/($ywidth+$wire)]
    if [expr $ny-int($ny)]>0 {
	# not an integer, have to change $cmval($id,4,1)
	set ny [expr int($ny)+1]
	# if not an even number, put it back
	if [expr $ny%2]!=0 { set ny [expr $ny-1] }
	set cmval($id,4,1) [expr 0.5*$ywidth+$wire+$ny*0.5*($ywidth+$wire)]
	$parent_w create text 325 40 -text "The total y-width of\
		the mesh has been changed to $cmval($id,4,1) cm."\
		-font $helvfont
    }

    # rectangle for wire
    set x(1) [expr (-$cmval($id,4,0)-$xmin)*$xscale+$l]
    set y(1) [expr (-$cmval($id,4,1)-$ymin)*$yscale+$m]
    set x(2) [expr ($cmval($id,4,0)-$xmin)*$xscale+$l]
    set y(2) [expr ($cmval($id,4,1)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(wire)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color

    # draw on the air holes
    set color [lindex $colorlist $med(air)]
    set yg [expr -$cmval($id,4,1)+$wire]
    for {set i 0} {$i<=$ny} {incr i} {
	set xg [expr -$cmval($id,4,0)+$wire]
	for {set j 0} {$j<=$nx} {incr j} {
	    set x(1) [expr ($xg-$xmin)*$xscale+$l]
	    set y(1) [expr ($yg-$ymin)*$yscale+$m]
	    set x(2) [expr ($xg+$xwidth-$xmin)*$xscale+$l]
	    set y(2) [expr ($yg+$ywidth-$ymin)*$yscale+$m]
	    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color
	    set xg [expr $xg+$wire+$xwidth]
	}
	set yg [expr $yg+$wire+$ywidth]
    }
}

proc add_MESH {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist medium nmed meds_used values colornum

    # assign numbers to the media, wire and air
    set med(wire) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,7) $medium($j)]==0 {
	    set med(wire) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
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
    # rectangle of air in background
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,2)+$cmval($id,3,3)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    # This is the one we're showing in the accelerator preview only.
    # Get number of x,y holes.  If dimensions change, change the value
    # temporarily only.
    set wire $cmval($id,3,2)
    set xwidth $cmval($id,3,0)
    set xtotal $cmval($id,4,0)
    set nx [expr 2*($cmval($id,4,0)-0.5*$xwidth-$wire)/($xwidth+$wire)]
    if [expr $nx-int($nx)]>0 {
	# not an integer, have to change $cmval($id,4,0)
	set nx [expr int($nx)+1]
	# if not an even number, put it back
	if [expr $nx%2]!=0 { set nx [expr $nx-1] }
	set xtotal [expr 0.5*$xwidth+$wire+$nx*0.5*($xwidth+$wire)]
    }
    # rectangles of wire
    set x1 -$xtotal
    for {set i 0} {$i<=[expr $nx+1]} {incr i} {
	set x2 [expr $x1+$cmval($id,3,2)]
	set xc1 [expr ($x1-$xmin)*$xscale+$l]
	set xc2 [expr ($x2-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(wire)]
	$parent_w create rectangle $xc1 $z(1) $xc2 $z(2) -fill $color -outline {}
	set x1 [expr $x2+$cmval($id,3,0)]
    }
}

proc add_MESH_yz {id yscale zscale ymin zmin l m parent_w} {
    global cmval colorlist medium nmed meds_used values colornum

    # assign numbers to the media, wire and air
    set med(wire) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,7) $medium($j)]==0 {
	    set med(wire) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
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
    # rectangle of air in background
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(2) [expr ($cmval($id,2)+$cmval($id,3,3)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color -outline {}

    # This is the one we're showing in the accelerator yz preview only.
    # Get number of x,y holes.  If dimensions change, change the value
    # temporarily only.
    set wire $cmval($id,3,2)
    set ywidth $cmval($id,3,1)
    set ytotal $cmval($id,4,1)
    set ny [expr 2*($cmval($id,4,1)-0.5*$ywidth-$wire)/($ywidth+$wire)]
    if [expr $ny-int($ny)]>0 {
	# not an integer, have to change $cmval($id,4,1)
	set ny [expr int($ny)+1]
	# if not an even number, put it back
	if [expr $ny%2]!=0 { set ny [expr $ny-1] }
	set ytotal [expr 0.5*$ywidth+$wire+$ny*0.5*($ywidth+$wire)]
    }
    # rectangles of wire
    set y1 -$ytotal
    for {set i 0} {$i<=[expr $ny+1]} {incr i} {
	set y2 [expr $y1+$cmval($id,3,2)]
	set yc1 [expr ($y1-$ymin)*$yscale+$l]
	set yc2 [expr ($y2-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(wire)]
	$parent_w create rectangle $yc1 $z(1) $yc2 $z(2) -fill $color -outline {}
	set y1 [expr $y2+$cmval($id,3,1)]
    }
}


